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

// number if routes and devices that can be stored
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

    /// @brief add route to/from a device
    /// @param addr     the IPv4 address of the device
    /// @param subnet   the subnet mask of the network the device is connected to
    /// @param layer    the layer packets are sent to / received from
    /// If a transmitted packet's destination IP has the longest match with this
    //  address/subnet the packet will be forwarded to 'layer'. A received packet
    //  from 'layer' must have the exact destination address of 'addr' to be
    //  forwarded to the next layer.
    //  NOTE: this means for multicasts, each route needs to be added individually!
    //        ex) add a route for the layer with address 234.0.0.1/32
    // for now there can be only device per address (including multicast)
    /// @return
    RetType add_route(IPv4Addr_t addr, IPv4Addr_t subnet, NetworkLayer& layer) {
        // check that it's a valid subnet
        if(!valid_subnet(subnet)) {
            return RET_ERROR;
        }

        // add to the device map
        NetworkLayer** dev_ptr = m_devMap[addr];
        if(dev_ptr) {
            // something else has this address
            return RET_ERROR;
        }

        dev_ptr = m_devMap.add(addr);
        if(dev_ptr == NULL) {
            // failed to add
            return RET_ERROR;
        }

        *dev_ptr = &layer;

        // add to the routing table
        Route route = {addr, subnet, &layer};
        if(!m_routingTable.push(route)) {
            // no room
            // remove device from map
            m_devMap.remove(addr);

            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    /// @brief remove a device route
    /// @param addr     the address of the device to remove
    /// @return returns success if the route no longer exists (including if it never did)
    RetType remove_route(IPv4Addr_t addr) {
        // remove the device from the hashmap
        // we don't check the return because if the address doesn't exist it's already removed
        m_devMap.remove(addr);

        // search for the address in the routing table
        QueueIterator<Route> it = m_routingTable.iterator();

        Route* curr;
        while(curr = *it) {
            if(curr->addr == addr) {
                // this is our guy
                m_routingTable.remove(curr);
                return RET_SUCCESS;
            }

            ++it;
        }

        // couldn't find the route, so it's technically removed
        return RET_SUCCESS;
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

        return RET_SUCCESS;
    }

    /// @brief remove a protocol layer
    /// @param protocol     the protocol layer to remove
    /// @return
    void remove_protocol(uint8_t protocol) {
        m_protMap.remove(protocol);
    }

    /// @brief lookup the next network layer for an IPv4 address
    /// @param addr     the IPv4 address to lookup
    /// @return the device, or NULL on error
    NetworkLayer* device(IPv4Addr_t addr) {
        NetworkLayer** ptr = m_devMap[addr];

        if(ptr == NULL) {
            return NULL;
        }

        return *ptr;
    }

    /// @brief receive a packet
    /// @return
    RetType receive(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
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

        NetworkLayer** dev_ptr = m_devMap[addr];
        if(dev_ptr == NULL) {
            // no device with this IP

            #ifdef NET_STATISTICS
            NetworkStatistics::DroppedIncomingPackets++;
            #endif

            return RET_ERROR;
        }

        if(*dev_ptr != caller) {
            // the address doesn't match the device it should have come in on

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
    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer*) {
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
        hdr->total_len = hton16(packet.size() + packet.headerSize());
        hdr->identification = 0;
        hdr->flags_frag = 0;
        hdr->ttl = DEFAULT_TTL;
        hdr->protocol = IPV4_PROTO[info.type];
        hdr->checksum = 0;
        hdr->dst = hton32(info.dst.ipv4_addr);
        hdr->src = hton32(m_route->addr);

        // calculate checksum
        hdr->checksum = hton16(checksum((uint16_t*)hdr, sizeof(IPv4Header_t) / sizeof(uint16_t)));

        // record information about the packet for lower layers to use
        info.dst.ipv4_addr = hdr->dst;
        info.src.ipv4_addr = hdr->src;

        printf("IP transmit 1");

        RetType ret = CALL(m_route->next->transmit(packet, info, this));

        RESET();
        return ret;
    }

    /// @brief second pass of transmitting a packet
    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
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
        printf("IP transmit 2");

        RetType ret = CALL(m_route->next->transmit2(packet, info, this));

        RESET();
        return ret;
    }

private:
    // IPv4 route to be used for longest prefix matching
    struct Route {
        IPv4Addr_t addr;            // device or network address
        IPv4Addr_t subnet;          // subnet
        NetworkLayer* next;     // layer to forward to
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

    // stores all outgoing routes
    // stores with
    alloc::SortedQueue<Route, SIZE> m_routingTable;

    // maps device addresses to layers those addresses should be received from
    alloc::Hashmap<IPv4Addr_t, NetworkLayer*, SIZE, SIZE> m_devMap;

    // maps device protocol to a next layer
    alloc::Hashmap<uint8_t, NetworkLayer*, SIZE, SIZE> m_protMap;

    // the found route for a packet
    // stores information b/w transmit1 and transmit2
    Route* m_route;
};

} // namespace ipv4

#endif
