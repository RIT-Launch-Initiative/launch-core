#ifndef IPV4_ROUTER_H
#define IPV4_ROUTER_H

#include <stdint.h>

#include "net/common.h"
#include "net/routing/Router.h"
#include "hashmap/hashmap.h"
#include "bloom_filter/bloom_filter.h"
#include "net/ipv4/ipv4.h"

namespace ipv4 {


class IPv4Router : public Router {
public:
    /// @brief constructor
    IPv4Router() {};

    /// @brief provide the router with a received packet
    ///        pushes the packet up the stack
    /// @return
    // TODO track parsed address somewheres
    RetType receive(Packet& packet) {
        IPv4Header_t* hdr = (IPv4Header_t*)(packet.raw() + (uint8_t*)packet.size());

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
        // uint16_t payload_len = total_len - header_len;
        // TODO anything with the len? pass it up to transport probably, in case we get extra data



        // zero the checksum in order to calculate, cache first
        uint16_t checksum = ntoh16(hdr->checksum);
        hdr->checksum = 0;

        // check the checkcum
        if(checksum != checksum(packet->raw(), header_len)) {
            // invalid checksum
            return RET_ERROR;
        }

        // TODO record dst address
        // TODO check the address is ours OR a multicast address
        // TODO to know our address, we should have the calling router passed in

        // read forward in the payload so it's at the payload
        // we don't handle options for now
        if(RET_SUCCESS != packet.skip_read(header_len)) {
            return RET_ERROR;
        }

        if(!m_protFilter.present(protocol)) {
            // we for sure don't have this route
            return RET_ERROR;
        }

        Router&* router = m_protMap[protocol];
        if(router == NULL) {
            // filter had a false positive
            return RET_ERROR;
        }

        // keep routing the packet
        router->route(packet);

        return RET_SUCCESS;
    }

    /// @brief send a packet through the router
    ///        pushes the packet down the stack
    /// @return
    // TODO include address somewheres
    RetType transmit(Packet& packet) {
        uint8_t* curr = packet.raw() + (uint8_t*)packet.size();
        IPHeader_t* hdr = (IPHeader_t*)curr;

        hdr->version_ihl = 0x45; // version = 4, length = 5 words (20 bytes)
        hdr->dscp_ecn = 0;
        hdr->total_len = 20; // TODO need to get passed a length
        hdr->identification = 0;
        hdr->flags_frag = 0;
        hdr->ttl = 255; // TODO something smaller?
        hdr->protocol = 0; // TODO need to get passed a protocol
        hdr->checksum = 0;
        hdr->dst = 0; // TODO get passed in
        hdr->src = 0; // TODO depends on route below

        // TODO calculate checksum

        // TODO
        // do a longest prefix match with the routes we have and send to that router
        // we assume there's never a default gateway and we're always routing to the local network

        return RET_SUCCESS;
    }

    /// @brief add a route for outgoing packets
    /// @param router   the router to push packets to
    /// @param addr     the address of to subscribe the router to
    /// @return
    RetType add_route(IPv4Addr_t addr, Router& router) {
        if(m_filter.present(addr)) {
            if(m_map[addr] != NULL) {
                // we already have an entry for this address
                return RET_ERROR;
            }
        } else {
            // this address is not in the map currently
            if(NULL != m_map.add(addr)) {
                return RET_ERROR;
            }

            // encode into the bloom filter
            m_filter.encode(addr);
        }

        return RET_SUCCESS;
    }

    /// @brief remove a route
    /// @param addr     the address to remove
    /// @return
    RetType remove_route(ADDR addr) {
        // for now, we leave the address encoded into the filter
        // we should re-encode every address when we remove one
        if(!m_map.remove(addr)) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    /// @brief add a transport layer router to forward packets to
    /// @param protocol     the transport protocol number in system endianness
    /// @param router       the router to forward packets with this protocol to
    RetType add_transport(uint8_t protocol, Router& router) {
        if(m_protFilter.present(protocol)) {
            if(m_protMapmap[protocol] != NULL) {
                // we already have an entry for this address
                return RET_ERROR;
            }
        } else {
            // this address is not in the map currently
            if(NULL != m_protMap.add(protocol)) {
                return RET_ERROR;
            }

            // encode into the bloom filter
            m_protFilter.encode(protocol);
        }

        return RET_SUCCESS;
    }

    /// @brief remove a transport protocol router
    /// @param protocol     the protocol number
    /// @return
    RetType remove_transport(uint8_t protocol) {
        // for now, we leave the address encoded into the filter
        // we should re-encode every address when we remove one
        if(!m_protMap.remove(protocol)) {
            return RET_ERROR;
        }

        return RET_SUCCESS;
    }

private:
    // maps addresses to routers
    // basically an IPv4 routing table
    // uses a bloom filter for fast rejection
    // TODO define sizes somewhere probably (or template it)
    alloc::Hashmap<IPv4Addr_t, Router&, 20, 20> m_map;
    BloomFilter<IPv4Addr_t> m_filter;

    // maps protocol numbers to routers
    // TODO define sizes somewhere probably (or template it)
    alloc::Hashmap<uint8_t, Router&, 20, 20> m_protMap;
    BloomFilter<uint8_t> m_protFilter;

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
};

} // namespace ipv4

#endif
