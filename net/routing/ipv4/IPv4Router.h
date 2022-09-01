#ifndef IPV4_ROUTER_H
#define IPV4_ROUTER_H

#include <stdint.h>

#include "net/common.h"
#include "net/routing/NetworkLayer.h"
#include "net/ipv4/ipv4.h"
#include "net/Socket.h"
#include "hashmap/hashmap.h"
#include "queue/allocated_queue.h"

using Socket::msg_t;

namespace ipv4 {

/// @brief IPv4 router
/// @tparam SIZE    the number of routes that can be stored
template <const size_t SIZE>
class IPv4Router : public NetworkLayer {
public:
    /// @brief constructor
    IPv4Router() : m_routingTable(route_sort), m_devMap(), m_protMap() {};

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
    RetType add_route(addr_t addr, addr_t subnet, NetworkLayer& layer) {
        // check that it's a valid subnet
        if(!valid_subnet(subnet)) {
            return RET_ERROR;
        }

        // add to the device map
        NetworLayer** dev_ptr = m_devMap[addr];
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
            m_devMap.rm(addr);

            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    /// @brief remove a device route
    /// @param addr     the address of the device to remove
    /// @return returns success if the route no longer exists (including if it never did)
    RetType remove_route(addr_t addr) {
        // remove the device from the hashmap
        // we don't check the return because if the address doesn't exist it's already removed
        m_devMap.remove(addr));

        // search for the address in the routing table
        QueueIterator<Route> it = m_routingTable.iterator();

        Route* curr;
        while(curr = *it) {
            if(curr->addr == addr) {
                // this is our guy
                m_routingTable.remove(curr);
                return RET_SUCCESS;
            }

            it++;
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
        m_protMap.rm(protocol);
    }

    /// @brief receive a packet
    /// @return
    RetType receive(Packet& packet, msg_t& info, NetworkLayer* caller) {
        header_t* hdr = (header_t*)(packet.raw() + (uint8_t*)packet.size());

        uint8_t version = (hdr->version_ihl & (0b11110000)) >> 4;
        if(version != 4) {
            // not IPv4
            return RET_ERROR;
        }

        if(hdr->flags & 0b00111111) {
            // MF is set or there's a fragment offset
            // TODO we discard fragmented packets for now
            return RET_ERROR;
        }

        uint8_t header_len = (hdr->version_ihl & (0b00001111));
        // uint16_t total_len = ntoh16(hdr.total_len);
        // TODO anything with the len? pass it up to transport probably, in case we get extra data

        // check the address
        addr_t addr = ntoh32(hdr->dst);

        NetworkLayer** dev_ptr = m_devMap[addr];
        if(dev_ptr == NULL) {
            // no device with this IP
            return RET_ERROR;
        }

        if(*dev_ptr != caller) {
            // the address doesn't match the device it should have come in on
            return RET_ERROR;
        }

        // find what layer to route it to based on protocol
        NetworkLayer** next_ptr = m_protMap[hdr->protocol];
        if(next_ptr == NULL) {
            // nowhere to send it to even if it is valid
            return RET_ERROR;
        }

        NetworkLayer* next = *next_ptr;

        // zero the checksum in order to calculate, cache first
        uint16_t checksum = ntoh16(hdr->checksum);
        hdr->checksum = 0;

        // check the checkcum
        if(checksum != checksum(packet->raw(), header_len)) {
            // invalid checksum
            return RET_ERROR;
        }

        // record the source address
        msg.addr.addr = ntoh32(hdr->src);

        // read forward in the payload so it's at the payload
        // we don't handle options for now
        if(RET_SUCCESS != packet.skip_read(header_len)) {
            return RET_ERROR;
        }

        return next->receive(packet, info, this);
    }

    /// @brief transmit a packet
    /// @return
    RetType transmit(Packet& packet, msg_t& info, NetworkLayer*) {
        // first find the route to send this packet over
        QueueIterator<Route> it = m_routingTable.iterator();
        Route* route;
        bool found = false;
        while(route = *it) {
            if(route->addr == ((info.addr.addr) & route->subnet)) {
                // this is a good match!
                // since the queue is presorted by biggest subnet mask, we know
                // this is the longest prefix match!
                found = true;
                break;
            }

            it++;
        }

        if(!found) {
            // we couldn't find a route
            // this shouldn't happen a lot b/c the user should generally add
            // some default route at 0.0.0.0/0
            return RET_ERROR;
        }

        uint8_t* curr = packet.raw() + (uint8_t*)packet.size();
        IPHeader_t* hdr = (IPHeader_t*)curr;

        hdr->version_ihl = DEFAULT_VERSION_IHL;
        hdr->dscp_ecn = 0;
        hdr->total_len = msg.payload_len;
        hdr->identification = 0;
        hdr->flags_frag = 0;
        hdr->ttl = DEFAULT_TTL;
        hdr->protocol = 0; // TODO need to get passed a protocol somehows, or look it up based on the layer that called us
        hdr->checksum = 0;
        hdr->dst = hton32(msg.addr.addr);
        hdr->src = hton32(route->addr);

        // calculate checksum
        hdr->checksum = checksum(curr, sizeof(IPHeader_t));

        // TODO how to preallocate space for headers in the packet
        // for now we just move it along
        if(RET_SUCCESS != packet.skip_read(sizeof(IPHeader_t))) {
            return RET_ERROR;
        }

        return route->next->transmit(packet, info, this);
    }

private:
    // IPv4 route to be used for longest prefix matching
    struct Route {
        addr_t addr;            // device or network address
        addr_t subnet;          // subnet
        NetworkLayer* next;     // layer to forward to
    };

    /// @brief helper function to validate a subnet address
    /// @param subnet   the subnet mask to validate
    /// @return 'true' if valid, 'false' otherwise
    bool valid_subnet(addr_t subnet) {
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
    inline size_t subnet_len(addr_t subnet) {
        for(size_t i = 0; i < (sizeof(subnet) * 8); i++) {
            if(subnet & (1 << i)) {
                return (sizeof(subnet) * 8) - i;
            }
        }

        // the all zero subnet
        return 0;
    }

    /// @brief sorting function to store routes by largest subnet first
    bool route_sort(Route& fst, Route& snd) {
        return subnet_len(fst.subnet) > subnet_len(snd.subnet);
    }

    /// @brief calculates IPv4 checksum
    /// header checksum field must be zero before calling!
    uint16_t checksum(const uint16_t* data, uint16_t len) {
        uint16_t sum = 0;
        for(int i = 0; i < (len / sizeof(uint16_t)); i++) {
            sum += data[i];
        }

        // retrun one's complement
        return ~sum;
    }

    // stores all outgoing routes
    // stores with
    alloc::SortedQueue<Route, SIZE> m_routingTable;

    // maps device addresses to layers those addresses should be received from
    alloc::Hashmap<addr_t, NetworkLayer*, SIZE, SIZE> m_devMap;

    // maps device protocol to a next layer
    alloc::Hashmap<uint8_t, NetworkLayer*, SIZE, SIZE> m_protMap;
};

} // namespace ipv4

#endif
