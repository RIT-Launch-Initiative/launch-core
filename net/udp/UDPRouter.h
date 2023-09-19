#ifndef LAUNCH_CORE_UDPROUTER_H
#define LAUNCH_CORE_UDPROUTER_H

#include "net/common.h"
#include "net/socket/Socket.h"
#include "net/packet/Packet.h"
#include "net/network_layer/NetworkLayer.h"
#include "hashmap/hashmap.h"
#include "udp.h"
#include "sched/macros.h"
#include <stdint.h>

namespace udp {
    static const size_t SIZE = 25;

    class UDPRouter : public NetworkLayer {
    public:
        explicit UDPRouter(NetworkLayer &networkLayer) : transmitLayer(&networkLayer) {}

        RetType subscribe_port(NetworkLayer *layer, uint16_t port_num) {
            NetworkLayer **layer_loc = port_bindings.add(port_num);
            if (!layer_loc) {
                return RET_ERROR;
            }

            uint16_t *port_num_loc = layer_bindings.add(layer);
            if (!port_num_loc) {
                return RET_ERROR;
            }

            *layer_loc = layer;
            *port_num_loc = port_num;

            return RET_SUCCESS;
        }

        RetType unsubscribePort(uint16_t port_num) {
            NetworkLayer **layer = port_bindings[port_num];
            bool device_success = layer_bindings.remove(*layer);
            bool protocol_success = port_bindings.remove(port_num);

            return device_success && protocol_success ? RET_SUCCESS : RET_ERROR;
        }

        RetType receive(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
            RESUME();

            UDP_HEADER_T *header = packet.read_ptr<UDP_HEADER_T>();
            info.src.udp_port = ntoh16(header->src);

            if (header == nullptr) {
                return RET_ERROR;
            }

            packet.skip_read(sizeof(UDP_HEADER_T));

            if(!info.ignore_checksums) {
                uint32_t src_ip = ntoh32(info.src.ipv4_addr);
                uint32_t dst_ip = ntoh32(info.dst.ipv4_addr);
                uint16_t original_checksum = header->checksum;
                header->checksum = 0;
                uint16_t calc_checksum = checksum(header, reinterpret_cast<uint8_t *>(&src_ip), reinterpret_cast<uint8_t *>(&dst_ip), packet.read_ptr<uint8_t>(), packet.available());
                if(original_checksum != calc_checksum) {
                    RESET();
                    return RET_ERROR;
                }
            }

            NetworkLayer **next_ptr = port_bindings[ntoh16(header->dst)];
            if (next_ptr == nullptr) {
                RESET();
                return RET_ERROR;
            }

            NetworkLayer *next = *next_ptr;
            if (next == nullptr) {
                RESET();
                return RET_ERROR;
            }
            RetType ret = CALL(next->receive(packet, info, this));

            RESET();
            return ret;
        }

        RetType transmit(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
            RESUME();

            UDP_HEADER_T *header = packet.allocate_header<UDP_HEADER_T>();

            if (header == nullptr) {
                RESET();
                return RET_ERROR;
            }

            uint16_t *src_port = layer_bindings[caller];
            if (src_port == nullptr) {
                RESET();
                return RET_ERROR;
            }

            header->src = hton16(*src_port);
            header->dst = hton16(info.dst.udp_port);
            header->checksum = 0;
            header->length = hton16(sizeof(UDP_HEADER_T) + packet.size());

            RetType ret = CALL(transmitLayer->transmit(packet, info, this));

            RESET();
            return ret;
        }

        RetType transmit2(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
            RESUME();

            UDP_HEADER_T *header = packet.allocate_header<UDP_HEADER_T>();

            if(NULL == header) {
                RESET();
                return RET_ERROR;
            }

            header->checksum = checksum(header, (uint8_t*)(&info.src.ipv4_addr),
                                        (uint8_t*)(&info.dst.ipv4_addr),
                                        packet.read_ptr<uint8_t>(), packet.available());

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
