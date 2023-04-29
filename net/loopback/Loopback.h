#ifndef LOOPBACK_H
#define LOOPBACK_H

#include "return.h"
#include "net/network_layer/NetworkLayer.h"

#include "sched/macros/macros.h"
#include "net/ipv4/ipv4.h"
#include "net/common.h"

/// @brief simple network layer that loops packets back to IPv4
/// NOTE: IPv4Router must always be the caller, as loopback assumes the previous
///       header is an IPv4 header so it can modify the src address to 127.0.0.1
class Loopback : public NetworkLayer {
public:
    /// @brief constructor
    Loopback() {};

    /// @brief transmit (first pass)
    RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        // don't do anything
        // but this has to be the bottom of the stack

        return RET_SUCCESS;
    }

    /// @brief transmit (second pass)
    /// bounce the packet back to the caller
    /// NOTE: assumes the caller is the IPv4 layer!
    ///       it will poke where the IPv4 header should be to change the dst
    ///       address to 127.0.0.1!
    RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        RESUME();

        ipv4::IPv4Header_t* hdr = packet.header_ptr<ipv4::IPv4Header_t>();
        if(NULL == hdr) {
            // no room
            return RET_ERROR;
        }

        // set the src address to 127.0.0.1
        ipv4::IPv4Addr_t new_dst;
        ipv4::IPv4Address(127, 0, 0, 1, &new_dst);
        hdr->dst = hton32(new_dst);

        // hint upper layers to ignore checksums as they will be incorrect now
        info.ignore_checksums = true;

        packet.seek_read(true);
        return CALL(caller->receive(packet, info, this));
    }

    /// @brief receive
    /// @return always error, loopback cannot receive
    RetType receive(Packet&, netinfo_t&, NetworkLayer*) {
        return RET_ERROR;
    }
};

#endif
