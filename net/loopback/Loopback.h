#ifndef LOOPBACK_H
#define LOOPBACK_H

#include "return.h"
#include "net/network_layer/NetworkLayer.h"
#include "sched/macros.h"

/// @brief simple network layer that loops packets back to the caller
class Loopback : public NetworkLayer {
public:
    /// @brief constructor
    Loopback() {};

    RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        // don't do anything
        // but this has to be the bottom of the stack

        return RET_SUCCESS;
    }

    /// @brief transmit (second pass)
    /// bounce the packet back to the caller
    RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        RESUME();

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
