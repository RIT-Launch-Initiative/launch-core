#ifndef LAUNCH_CORE_UDPROUTER_H
#define LAUNCH_CORE_UDPROUTER_H

#include "net/common.h"
#include "net/socket/Socket.h"
#include "net/packet/Packet.h"
#include "net/network_layer/NetworkLayer.h"
#include "vector/vector.h"
#include "hashmap/hashmap.h"
#include "udp.h"
#include <stdint.h>

namespace udp {
    static const size_t SIZE = 25;

    class UDPRouter : public NetworkLayer {
    public:
        UDPRouter() {
            src_port = 0;
        }

        UDPRouter(uint16_t port_num) {
            src_port = port_num;
        }

        RetType bind(NetworkLayer layer, uint16_t &port_num) {
            uint16_t **ret_loc = device_map.add(layer);

            if (ret_loc == NULL) {
                return RET_ERROR;
            }

            *ret_loc = &port_num;

            return RET_SUCCESS;
        }

        RetType unbind() {
            bool success = protocol_map.remove(port_num);

            return success ? RET_SUCCESS : RET_ERROR;
        }


        RetType subscribePort(NetworkLayer &subscriber, uint16_t port_num) {
            NetworkLayer **ret_loc = protocol_map.add(port_num);

            if (ret_loc == NULL) {
                return RET_ERROR;
            }

            *ret_loc = &subscriber;

            return RET_SUCCESS;
        }

        RetType unsubscribePort(uint16_t port_num) {
            bool success = protocol_map.remove(port_num);

            return success ? RET_SUCCESS : RET_ERROR;
        }

        RetType receive(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {
            RESUME();

            UDP_HEADER_T *header = packet.read_ptr<UDP_HEADER_T>();
            info.port.udp = header->src;

            if (header == NULL) {
                return RET_ERROR;
            }

            packet.skip_read(header->length);

            NetworkLayer **next_ptr = protocol_map[header->dst];
            if (next_ptr == NULL) {
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

            if (header == NULL) {
                return RET_ERROR;
            }

            header->src = 0; // TODO: Could do a second pass to get src Info
            header->dst = info.port.udp; // UDP is big endian
            header->checksum = 0;
            header->length = info.payload_len;

            NetworkLayer **next_ptr = protocol_map[header->dst];

            if (!next_ptr) {
                return RET_ERROR;
            }

            NetworkLayer *next = *next_ptr;
            printf("Transmitting from UDP\n");

            RetType ret = CALL(next->transmit(packet, info, this));

            RESET();
            return ret;
        }

    private:
        uint16_t src_port;
        alloc::Hashmap<uint16_t, NetworkLayer *, SIZE, SIZE> protocol_map;
        alloc::Hashmap<NetworkLayer *, uint8_t, SIZE, SIZE> device_map;



    };
}

#endif //LAUNCH_CORE_UDPROUTER_H
