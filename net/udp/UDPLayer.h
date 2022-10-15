#ifndef LAUNCH_CORE_UDPLAYER_H
#define LAUNCH_CORE_UDPLAYER_H

#include "net/socket/Socket.h"
#include "net/packet/Packet.h"
#include "net/network_layer/NetworkLayer.h"
#include "vector/vector.h"
#include "hashmap/hashmap.h"
#include "udp.h"
#include <stdint.h>

namespace udp {
    static const size_t SIZE = 25;

    class UDPLayer : public NetworkLayer {
    public:
        UDPLayer() {
            src_port = 0;
        }

        UDPLayer(uint16_t port_num) {
            src_port = port_num;
        }

        RetType subscribePort(NetworkLayer *subscriber, uint16_t port_num) {
            NetworkLayer** ret_loc = port_map.add(port_num);

            if (ret_loc != NULL) {
                return RET_ERROR;
            }

            ret_loc = &subscriber;

            return RET_SUCCESS;
        }

        RetType unsubscribePort(uint16_t port_num) {
            bool success = port_map.remove(port_num);

            return success ? RET_SUCCESS : RET_ERROR;
        }

        RetType receive(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {
            RESUME();

            UDP_HEADER_T *header = packet.read_ptr<UDP_HEADER_T>();

            if (header != NULL) {
                return RET_ERROR;
            }

            NetworkLayer** next_ptr = port_map[info.port];
            if (!next_ptr) {
                return RET_ERROR;
            }

            NetworkLayer *next = *next_ptr;

            RetType ret = CALL(next->receive(packet, info, this));

            RESET();

            return ret;
        }

        RetType transmit(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {
            RESUME();

            UDP_HEADER_T *header = packet.allocate_header<UDP_HEADER_T>();

            if (header != NULL) {
                return RET_ERROR;
            }

            header->src = src_port; // TODO: Could do a second pass to get src
            header->dst = info.port.udp;
            header->checksum = 0;
            header->data_octets = packet.read_ptr<uint32_t>();
            header->length = info.payload_len;

            RetType ret = CALL(route->next->transmit(packet, info, this));

            RESET();
            return ret;
        }

    private:
        uint16_t src_port;
        alloc::Hashmap<uint16_t, NetworkLayer*, SIZE, SIZE> port_map;

    };
}

#endif //LAUNCH_CORE_UDPLAYER_H
