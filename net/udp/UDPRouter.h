#ifndef LAUNCH_CORE_UDPROUTER_H
#define LAUNCH_CORE_UDPROUTER_H

#include "net/common.h"
#include "net/socket/Socket.h"
#include "net/packet/Packet.h"
#include "net/network_layer/NetworkLayer.h"
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
            NetworkLayer **layer_loc = port_bindings.add(port_num);
            if (!layer_loc) {
                return RET_ERROR;
            }

            uint16_t *port_num_loc = layer_bindings.add(&layer);
            if (!port_num_loc) {
                return RET_ERROR;
            }

            *layer_loc = &layer;
            port_num_loc = &port_num;

            return RET_SUCCESS;
        }

        RetType unsubscribePort(uint16_t port_num) {
            NetworkLayer **layer = port_bindings[port_num];
            bool device_success = layer_bindings.remove(*layer);
            bool protocol_success = port_bindings.remove(port_num);

            return device_success && protocol_success ? RET_SUCCESS : RET_ERROR;
        }

        RetType receive(Packet &packet, sockinfo_t &info, NetworkLayer *caller) {
            RESUME();

            UDP_HEADER_T *header = packet.read_ptr<UDP_HEADER_T>();
            info.src.udp_port = ntoh16(header->src);

            if (header == NULL) {
                return RET_ERROR;
            }

            packet.skip_read(sizeof(UDP_HEADER_T));

            PSEUDO_HEADER_T pseudo = {
                    ntoh32(info.src.ipv4_addr),
                    ntoh32(info.dst.ipv4_addr),
                    0,
                    17,
                    header->length
            };

            if (header->checksum != checksum(&pseudo)) {
                printf("Invalid checksum");

                return RET_ERROR;
            }

            NetworkLayer **next_ptr = port_bindings[ntoh16(header->dst)];
            if (next_ptr == NULL) {
                return RET_ERROR;
            }

            NetworkLayer *next = *next_ptr;
            if (next == NULL) {
                return RET_ERROR;
            }
            RetType ret = CALL(next->receive(packet, info, this));

            RESET();

            return ret;
        }

        RetType transmit(Packet &packet, sockinfo_t &info, NetworkLayer *caller) {
            RESUME();

            if (this->transmitLayer == NULL) {
                return RET_ERROR;
            }

            UDP_HEADER_T *header = packet.allocate_header<UDP_HEADER_T>();

            if (header == NULL) {
                return RET_ERROR;
            }

            uint16_t *src_port = layer_bindings[caller];
            if (src_port == NULL) {
                return RET_ERROR;
            }

            header->src = hton16(*src_port);
            header->dst = hton16(info.dst.udp_port); // UDP is big endian
            header->checksum = 0;
            header->length = sizeof(info)  + packet.headerSize() - sizeof(UDP_HEADER_T);

            RetType ret = CALL(transmitLayer->transmit(packet, info, this));

            RESET();
            return ret;
        }

        RetType transmit2(Packet &packet, sockinfo_t &info, NetworkLayer *caller) {
            RESUME();

            if (this->transmitLayer == NULL) {
                return RET_ERROR;
            }

            UDP_HEADER_T *header = packet.allocate_header<UDP_HEADER_T>();

            if (header == NULL) {
                return RET_ERROR;
            }

            uint16_t *src_port = layer_bindings[caller];
            if (src_port == NULL) {
                return RET_ERROR;
            }

            header->src = hton16(*src_port);
            header->dst = hton16(info.dst.udp_port); // UDP is big endian
            PSEUDO_HEADER_T pseudo = {
                    info.src.ipv4_addr,
                    info.dst.ipv4_addr,
                    0,
                    17,
                    header->length
            };

            header->checksum = checksum(&pseudo);
            header->length = sizeof(info) + packet.headerSize() - sizeof(UDP_HEADER_T);

            printf("UDP transmit 2\n");

            RetType ret = CALL(transmitLayer->transmit2(packet, info, this));

            RESET();
            return ret;
        }

    private:
        alloc::Hashmap<uint16_t, NetworkLayer *, SIZE, SIZE> port_bindings;
        alloc::Hashmap<NetworkLayer *, uint16_t, SIZE, SIZE> layer_bindings;
        NetworkLayer *transmitLayer;
    };
}

#endif //LAUNCH_CORE_UDPROUTER_H
