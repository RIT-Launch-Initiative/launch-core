/**
* Trivial File Transfer Protocol (Layer 5)
 *
 * @author Aaron Chan
*/

#include "net/network_layer/NetworkLayer.h"

namespace tftp {
class TFTP : public NetworkLayer {
public:
    TFTP() : NetworkLayer() {}

    RetType receive(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_ERROR;
    }

    RetType transmit(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_ERROR;
    }

    RetType transmit2(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_ERROR;
    }

private:


};
}