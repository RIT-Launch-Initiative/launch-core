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
            this->transmitLayer = nullptr;
        }

        UDPRouter(NetworkLayer &networkLayer) {
            this->transmitLayer = &networkLayer;
        }

        void setTransmitLayer(NetworkLayer &layer) {
            this->transmitLayer = &layer;
        }

        RetType subscribe_port(NetworkLayer &layer, uint16_t port_num) {
            NetworkLayer **layer_loc = protocol_map.add(port_num);
            if (!layer_loc) {
                return RET_ERROR;
            }

            uint16_t *port_num_loc = device_map.add(&layer);
            if (!port_num_loc) {
                return RET_ERROR;
            }

            *layer_loc = &layer;
            port_num_loc = &port_num;

            return RET_SUCCESS;
        }

        RetType unsubscribePort(uint16_t port_num) {
            NetworkLayer **layer = protocol_map[port_num];
            bool device_success = device_map.remove(*layer);
            bool protocol_success = protocol_map.remove(port_num);

            return device_success && protocol_success ? RET_SUCCESS : RET_ERROR;
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
            printf("Receiving from UDP\n");
            RetType ret = CALL(next->receive(packet, info, this));

            RESET();

            return ret;
        }

        RetType transmit(Packet &packet, sockmsg_t &info, NetworkLayer *caller) {
            RESUME();

            if (this->transmitLayer == NULL) {
                return RET_ERROR;
            }

            UDP_HEADER_T *header = packet.allocate_header<UDP_HEADER_T>();

            if (header == NULL) {
                return RET_ERROR;
            }

            header->src = *device_map[caller];
            header->dst = info.port.udp; // UDP is big endian
            header->checksum = 0;
            header->length = info.payload_len;

            uint16_t *src_port_m = device_map[caller];

            if (!src_port_m) {
                return RET_ERROR;
            }

            uint16_t src_port = *src_port_m;
            printf("Transmitting from UDP\n");

            RetType ret = CALL(transmitLayer->transmit(packet, info, this));

            RESET();
            return ret;
        }

    private:
        alloc::Hashmap<uint16_t, NetworkLayer *, SIZE, SIZE> protocol_map; // TODO: Needs a name change
        alloc::Hashmap<NetworkLayer *, uint16_t, SIZE, SIZE> device_map;
        NetworkLayer *transmitLayer;
    };
}

#endif //LAUNCH_CORE_UDPROUTER_H
