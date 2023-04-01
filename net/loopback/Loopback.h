#ifndef LOOPBACK_H
#define LOOPBACK_H

#include "return.h"
#include "net/network_layer/NetworkLayer.h"

/// @brief simple network layer that loops packets back to the caller
class Loopback : public NetworkLayer {
public:
    /// @brief constructor
    Loopback() : rx_packet(nullptr), rx_info(nullptr), rx_caller(nullptr) {};

    RetType transmit(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        // don't do anything
        // but this has to be the bottom of the stack

        return RET_SUCCESS;
    }

    /// @brief transmit (second pass)
    /// bounce the packet back to the caller
    RetType transmit2(Packet& packet, sockinfo_t& info, NetworkLayer* caller) {
        packet.seek_read(true);
        flag = 1;
        rx_packet = &packet;
        rx_info = &info;
        rx_caller = caller;
        
        // Flag when transmit2 is called, return success
        // Copy packet 
        return RET_SUCCESS;
    }   

    /// @brief receive
    /// @return always error, loopback cannot receive
    RetType receive(Packet&, sockinfo_t&, NetworkLayer*) {
        if (flag == 1) {
            flag = 0;
            return rx_caller->receive(*rx_packet, *rx_info, rx_caller);
        }
        return RET_ERROR;   
    }

private:
    int flag = 0;   // Flag to indicate transmit2 has been called, 0=false, 1=true

    Packet* rx_packet;
    sockinfo_t* rx_info;
    NetworkLayer* rx_caller;
};

#endif
