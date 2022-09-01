#ifndef LOOPBACK_H
#define LOOPBACK_H

#include "return.h"
#include "net/routing/NetworkLayer.h"

/// @brief simple network layer that loops packets back to the caller
class Loopback : public NetworkLayer {
public:
    /// @brief constructor
    Loopback() {};

    /// @brief transmit
    RetType transmit(Packet& packet, msg_t& info, NetworkLayer* caller) {
        return caller->receive(packet, info, this);
    }

    /// @brief receive
    /// @return always error, loopback cannot receive
    RetType receive(Packet&, msg_t&, NetworkLayer*) {
        return RET_ERROR;
    }
};

#endif
