/**
* Trivial File Transfer Protocol (Layer 5)
 *
 * @author Aaron Chan
*/
#include <stdint.h>
#include <string.h>

#include "sched/macros.h"
#include "net/common.h"
#include "net/network_layer/NetworkLayer.h"

namespace tftp {
    static constexpr uint16_t TFTP_PORT = 69;

    typedef enum {
        READ_REQUEST_OP = 1,
        WRITE_REQUEST_OP = 2,
        DATA_OP = 3,
        ACK_OP = 4,
        ERROR_OP = 5
    } TFTP_OPCODE_T;

    typedef enum {
        NETASCII_MODE = 0,
        OCTET_MODE,
        MAIL_MODE
    } TFTP_MODE_T;

    typedef struct {
        uint16_t opcode;
        uint8_t *filename;
        uint8_t *mode; // "netascii" "octet" or "mail"
    } TFTP_HEADER_T;



class TFTP : public NetworkLayer {
public:
    TFTP(NetworkLayer &layer) : NetworkLayer(), m_lowerLayer(&layer) {}

    RetType sendFile(Packet packet, netinfo_t &info, const TFTP_MODE_T mode,
                     const uint8_t *filename, const size_t filenameLen,
                     uint8_t *data, size_t *len) {
        RESUME();

        if (lock) {
            RESET();
            return RET_ERROR;
        }

        setOpcode(WRITE_REQUEST_OP);
        setFilename(filename, filenameLen);
        setMode(mode);

        lock = true;

        packet.clear();

        while (0 != *len) {
            if (512 > *len) {
                packet.push(data, *len);
            } else {
                packet.push(data, 512);
                *len -= 512;
            }

            RetType ret = CALL(m_lowerLayer->transmit(packet, info, this));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            ret = CALL(m_lowerLayer->transmit2(packet, info, this));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            m_retransmitPacket = packet;

            packet.clear();

            ret = CALL(m_lowerLayer->receive(packet, info, this));
            if (ret != RET_SUCCESS || ACK_OP != packet.read_ptr<TFTP_HEADER_T>()->opcode) {
                RESET();
                return ret;
            }
        }

        lock = false;
        RESET();
        return RET_SUCCESS;
    }

    RetType receive(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_ERROR;
    }

    RetType transmit(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        RESUME();

        if (lock) {
            RESET();
            return RET_ERROR;
        }

        lock = true;

        TFTP_HEADER_T *header = packet.allocate_header<TFTP_HEADER_T>();
        if (header == nullptr) {
            RESET();
            return RET_ERROR;
        }

        if (!setHeader(header)) {
            RESET();
            return RET_ERROR;
        }
        
        RetType ret = CALL(m_lowerLayer->transmit(packet, info, this));

        RESET();
        return ret;
    }

    RetType transmit2(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        RESUME();

        if (lock) {
            RESET();
            return RET_ERROR;
        }

        lock = true;

        TFTP_HEADER_T *header = packet.allocate_header<TFTP_HEADER_T>();
        if (header == nullptr) {
            RESET();
            return RET_ERROR;
        }

        if (!setHeader(header)) {
            RESET();
            return RET_ERROR;
        }
        
        RetType ret = CALL(m_lowerLayer->transmit2(packet, info, this));
        

        lock = false;
        RESET();
        return ret;
    }

    void setOpcode(TFTP_OPCODE_T opcode) {
        this->m_currentOpcode = opcode;
    }

    void setFilename(const uint8_t* filename, size_t len) {
        memcpy(m_currentFilename, filename, len);

        if (0 != *(m_currentFilename + len) && len + 1 < 256) {
            *(m_currentFilename + len + 1) = 0;
            len += 1;
        } else { // Just guarantee null termination if we can't fit the null
            *(m_currentFilename + len) = 0;
        }
    }

    void setMode(TFTP_MODE_T mode) {
        switch (mode) {
            case NETASCII_MODE:
                memcpy(m_currentMode, "netascii", 8);
                *(m_currentMode + 8) = 0;
                m_currentModeLen = 9;
                break;
            case OCTET_MODE:
                memcpy(m_currentMode, "octet", 5);
                *(m_currentMode + 5) = 0;
                m_currentModeLen = 6;
                break;
            case MAIL_MODE:
                memcpy(m_currentMode, "mail", 4);
                *(m_currentMode + 4) = 0;
                m_currentModeLen = 5;
                break;
        }
    }
    
private:
    NetworkLayer *m_lowerLayer;
    Packet m_retransmitPacket = alloc::Packet<0, 0>();
    
    TFTP_OPCODE_T m_currentOpcode = READ_REQUEST_OP;
    
    uint8_t m_currentFilename[256] = "";
    uint8_t m_currentFilenameLen = 0;

    uint8_t m_currentMode[9] = "";
    uint8_t m_currentModeLen = 0;

    bool lock = false; // Avoid setting headers while transmitting


    bool setHeader(TFTP_HEADER_T *header) {
        if (0 == m_currentFilenameLen || 0 == m_currentModeLen) {
            return false;
        }

        header->opcode = hton16(m_currentOpcode);
        header->filename = m_currentFilename;
        header->mode = m_currentMode;

        return true;
    }

};
}