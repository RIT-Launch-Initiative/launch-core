#ifndef LAUNCH_CORE_UDP_H
#define LAUNCH_CORE_UDP_H


namespace udp {
    typedef struct {
        uint8_t src,
                uint8_t dst,
        unsigned short checksum,
        unsigned short length,
                uint8_t *data_octets
    } UDP_HEADER_T;

    unsigned short calc_checksum(uint8_t *payload) {
        int checksum = 0;
        for (int i = 0; i < sizeof(payload); i++) {
            checksum += payload[i];
        }

        return checksum;
    }

    bool verify_checksum(uint8_t *payload, unsigned short checksum) {
        int result;

        for (int i = 0; i < sizeof(payload); i++) {
            result += payload[i];
        }

        return checksum == result;
    }
}

#endif //LAUNCH_CORE_UDP_H
