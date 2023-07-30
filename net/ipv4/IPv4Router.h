#ifndef IPV4_ROUTER_H
#define IPV4_ROUTER_H

#include <stdint.h>

#include "net/common.h"
#include "net/network_layer/NetworkLayer.h"
#include "net/ipv4/ipv4.h"
#include "net/socket/Socket.h"
#include "hashmap/hashmap.h"
#include "queue/allocated_queue.h"
#include "sched/macros.h"
#include "config.h"

#ifdef NET_STATISTICS
#include "net/statistics/NetworkStatistics.h"
#endif

namespace ipv4 {

// number of routes and layers that can be stored
// TODO don't hardcode this!
// TODO template with alloc::IPv4Router
static const size_t SIZE = 25;

/// @brief IPv4 router
#ifdef NET_STATISTICS
class IPv4Router : public NetworkLayer, public NetworkStatistics {
#else
class IPv4Router : public NetworkLayer {
#endif
public:
    /// @brief constructor
    IPv4Router() : m_routingTable(route_sort) {};

    /// @brief add an outgoing route
    /// @param addr     IPv4 address
    /// @param subnet   subnet mask
    /// @param layer    network layer
    /// If a transmitted packet's destination IP has the longest match with this
    //  'addr' on subnet 'subnet', the packet will be forwarded to 'layer'.
    //  NOTE: currently adding conflicting routes is undefined, whichever
    //        route is matched first will be used
    /// @return
    RetType add_outgoing_route(IPv4Addr_t addr, IPv4Addr_t subnet,
                                                          NetworkLayer& layer) {
        // check that it's a valid subnet
        if(!valid_subnet(subnet)) {
            return RET_ERROR;
        }

        // add to the routing table
        Route route = {addr, subnet, &layer};
        if(!m_routingTable.push(route)) {
            // no room
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    /// @brief remove an outgoing route
    /// @param addr     the address of the route to remove
    /// @param subnet   the subnet of the route to remove
    /// @return success if the route no longer exists (including if it never did)
    /// NOTE: if there are conflicting routes, only one will be removed!
    RetType remove_outgoing_route(IPv4Addr_t addr, IPv4Addr_t subnet) {
        // search for the address in the routing table
        QueueIterator<Route> it = m_routingTable.iterator();

        Route* curr;
        while(curr = *it) {
            if(curr->addr == addr && curr->subnet == subnet) {
                // this is our guy
                m_routingTable.remove(curr);
                return RET_SUCCESS;
            }

            ++it;
        }

        // couldn't find the route, so it's technically removed
        return RET_SUCCESS;
    }

    /// @brief add an incoming route
    /// @param addr   address
    /// @param layer  network layer
    /// Add an incoming route. Packets that are received from 'layer' that have
    /// a destination address of 'addr' will be forwarded to a higher layer.
    /// All other packets, besides broadcasts, will be dropped.
    /// NOTE: only one incoming layer is allowed per address, this is purely
    ///       for implementation simplicity and could be changed
    /// @return
    RetType add_incoming_route(IPv4Addr_t addr, NetworkLayer& layer) {
        NetworkLayer** ptr = m_incoming[addr];
        if(ptr) {
            // this address is already mapped to an incoming layer
            return RET_ERROR;
        }

        ptr = m_incoming.add(addr);
        if(NULL == ptr) {
            // no room
            return RET_ERROR;
        }

        *ptr = &layer;

        return RET_SUCCESS;
    }

    /// @brief remove an incoming route
    /// @param addr     the address of the route to remove
    void remove_incoming_route(IPv4Addr_t addr) {
        m_incoming.remove(addr);
    }

    /// @brief add a protocol layer to forward packets too
    ///        packets with protocol field 'protocol' will be sent to 'layer'
    ///        only one layer per protocol allowed at the moment
    /// @param protocol
    /// @param layer
    /// @return
    RetType add_protocol(uint8_t protocol, NetworkLayer& layer) {
        NetworkLayer** ptr = m_protMap[protocol];
        if(ptr) {
            // something else already handles this protocol
            return RET_ERROR;
        }

        ptr = m_protMap.add(protocol);
        if(ptr == NULL) {
            // unable to add
            return RET_ERROR;
        }

        *ptr = &layer;

        uint8_t* num_ptr = m_protNumMap.add(&layer);
        if(num_ptr == NULL) {
            // unable to add
        }

        *num_ptr = protocol;

        return RET_SUCCESS;
    }

    /// @brief remove a protocol layer
    /// @param protocol     the protocol layer to remove
    /// @return
    void remove_protocol(uint8_t protocol) {
        NetworkLayer** ptr = m_protMap[protocol];

        if(ptr != NULL) {
            m_protNumMap.remove(*ptr);
            m_protMap.remove(protocol);
        }
    }

    /// @brief receive a packet
    /// @return
    RetType receive(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        RESUME();

        IPv4Header_t* hdr = packet.read_ptr<IPv4Header_t>();

        if(hdr == NULL) {
            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        uint8_t version = (hdr->version_ihl & (0b11110000)) >> 4;
        if(version != 4) {
            // not IPv4

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        if(hdr->flags_frag & 0b00111111) {
            // MF is set or there's a fragment offset
            // TODO we discard fragmented packets for now

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        uint8_t header_len = (hdr->version_ihl & (0b00001111)) * 4;
        uint16_t total_len = ntoh16(hdr->total_len);
        uint16_t payload_len = total_len - header_len;

        // read forward in the payload so it's at the payload
        // we don't handle options for now
        if(RET_SUCCESS != packet.skip_read(header_len)) {
            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        // check we have enough data in the packet for what the payload should be
        if(packet.available() < payload_len) {
            // we don't have enough data

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        } else {
            packet.truncate(payload_len);
        }

        // check the address
        IPv4Addr_t addr = ntoh32(hdr->dst);

        NetworkLayer** incoming_ptr = m_incoming[addr];
        if(incoming_ptr == NULL) {
            // no layer with this IP

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        if(*incoming_ptr != caller) {
            // the address doesn't match the layer it should have come in on

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        // find what layer to route it to based on protocol
        NetworkLayer** next_ptr = m_protMap[hdr->protocol];
        if(next_ptr == NULL) {
            // nowhere to send it to even if it is valid

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        NetworkLayer* next = *next_ptr;

        if(!info.ignore_checksums) {
            // zero the checksum in order to calculate, cache first
            uint16_t check = ntoh16(hdr->checksum);
            hdr->checksum = 0;

            // check the checksum
            if(check != checksum((uint16_t*)hdr, header_len / sizeof(uint16_t))) {
                // invalid checksum

                #ifdef NET_STATISTICS
                NetworkStatistics::DroppedIncomingPackets++;
                #endif

                return RET_ERROR;
            }
        }

        // record information about the packet
        info.src.ipv4_addr = ntoh32(hdr->src);
        info.dst.ipv4_addr = addr;


        #ifdef NET_STATISTICS
        NetworkStatistics::IncomingPackets++;
        #endif

        RetType ret = CALL(next->receive(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief transmit a packet
    /// @return
    RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        RESUME();

        // first find the route to send this packet over
        // best route is found by doing longest prefix match of IPv4 CIDR addresses
        QueueIterator<Route> it = m_routingTable.iterator();
        bool found = false;
        while(m_route = *it) {
            if((m_route->addr & m_route->subnet) == (info.dst.ipv4_addr & m_route->subnet)) {
                // this is a good match!
                // since the queue is presorted by biggest subnet mask, we know
                // this is the longest prefix match!
                found = true;
                break;
            }

            ++it;
        }

        if(!found) {
            // we couldn't find a route
            // this shouldn't happen a lot b/c the user should generally add
            // some default route at 0.0.0.0/0

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedOutgoingPackets++;
            #endif

            return RET_ERROR;
        }

        // look up the protocol number
        uint8_t proto;
        uint8_t* ptr = m_protNumMap[caller];
        if(ptr == NULL) {
            // no protocol for this caller
            return RET_ERROR;
        }

        proto = *ptr;

        IPv4Header_t* hdr = packet.allocate_header<IPv4Header_t>();
        if(hdr == NULL) {
            // no room for header

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedOutgoingPackets++;
            #endif

            return RET_ERROR;
        }

        hdr->version_ihl = DEFAULT_VERSION_IHL;
        hdr->dscp_ecn = 0;
        hdr->total_len = hton16(packet.size() + packet.header_size());
        hdr->identification = 0;
        hdr->flags_frag = 0;
        hdr->ttl = DEFAULT_TTL;
        hdr->protocol = proto;
        hdr->checksum = 0;
        hdr->dst = hton32(info.dst.ipv4_addr);
        hdr->src = hton32(m_route->addr);

        // calculate checksum
        hdr->checksum = checksum((uint16_t*)hdr, sizeof(IPv4Header_t));

        // record information about the packet for lower layers to use
        info.dst.ipv4_addr = hdr->dst;
        info.src.ipv4_addr = hdr->src;

        RetType ret = CALL(m_route->next->transmit(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief second pass of transmitting a packet
    RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        RESUME();

        // don't do anything on the second pass, just move the header and pass it along
        IPv4Header_t* hdr = packet.allocate_header<IPv4Header_t>();
        if(hdr == NULL) {
            // no room for header
            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedOutgoingPackets++;
            #endif

            return RET_ERROR;
        }

        #ifdef NET_STATISTICS
        NetworkStatistics::OutgoingPackets++;
        #endif

        // pass it along
        RetType ret = CALL(m_route->next->transmit2(packet, info, this));

        RESET();
        return ret;
    }

private:
    // IPv4 route to be used for longest prefix matching
    struct Route {
        IPv4Addr_t addr;            // network address
        IPv4Addr_t subnet;          // subnet
        NetworkLayer* next;         // lower layer to forward to
    };

    /// @brief helper function to validate a subnet address
    /// @param subnet   the subnet mask to validate
    /// @return 'true' if valid, 'false' otherwise
    bool valid_subnet(IPv4Addr_t subnet) {
        // validate the subnet is of the form 0b111....111000...000
        bool ones = false;
        for(size_t i = 0; i < (sizeof(subnet) * 8); i++) {
            if(ones) {
                if(!(subnet & (1 << i))) {
                    return false;
                }
            } else {
                if(subnet & (1 << i)) {
                    // found the first 1
                    ones = true;
                }
            }
        }

        return true;
    }

    /// @brief get the length of a subnet
    ///        assumes the subnet mask is valid
    /// @param subnet   the subnet mask
    /// @return the length
    static inline size_t subnet_len(IPv4Addr_t subnet) {
        for(size_t i = 0; i < (sizeof(subnet) * 8); i++) {
            if(subnet & (1 << i)) {
                return (sizeof(subnet) * 8) - i;
            }
        }

        // the all zero subnet
        return 0;
    }

    /// @brief sorting function to store routes by largest subnet first
    static bool route_sort(Route& fst, Route& snd) {
        return subnet_len(fst.subnet) > subnet_len(snd.subnet);
    }

    // stores all outgoing routes to lower layers
    alloc::SortedQueue<Route, SIZE> m_routingTable;

    // stores incoming routes
    // maps addresses to a layer packets from that address should come in on
    alloc::Hashmap<IPv4Addr_t, NetworkLayer*, SIZE, SIZE> m_incoming;

    // maps higher level protocols to protocol numbers
    alloc::Hashmap<uint8_t, NetworkLayer*, SIZE, SIZE> m_protMap;

    // maps higher layers to protocol numbers
    alloc::Hashmap<NetworkLayer*, uint8_t, SIZE, SIZE> m_protNumMap;

    // the found route for a packet
    // stores information b/w transmit1 and transmit2
    Route* m_route;
};

} // namespace ipv4

#endif
