/**
 * RFM95W LoRa Driver
 *
 * @author Aaron Chan
 */

#ifndef LAUNCH_CORE_RFM95W_H
#define LAUNCH_CORE_RFM95W_H

#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "device/SPIDevice.h"
#include "device/StreamDevice.h"
#include "device/GPIODevice.h"


enum RFM95_REGISTER_T {
    RFM95_REGISTER_FIFO_ACCESS = 0x00,
    RFM95_REGISTER_OP_MODE = 0x01,
    RFM95_REGISTER_FR_MSB = 0x06,
    RFM95_REGISTER_FR_MID = 0x07,
    RFM95_REGISTER_FR_LSB = 0x08,
    RFM95_REGISTER_PA_CONFIG = 0x09,
    RFM95_REGISTER_LNA = 0x0C,
    RFM95_REGISTER_FIFO_ADDR_PTR = 0x0D,
    RFM95_REGISTER_FIFO_TX_BASE_ADDR = 0x0E,
    RFM95_REGISTER_FIFO_RX_BASE_ADDR = 0x0F,
    RFM95_REGISTER_IRQ_FLAGS = 0x12,
    RFM95_REGISTER_FIFO_RX_BYTES_NB = 0x13,
    RFM95_REGISTER_PACKET_SNR = 0x19,
    RFM95_REGISTER_MODEM_CONFIG_1 = 0x1D,
    RFM95_REGISTER_MODEM_CONFIG_2 = 0x1E,
    RFM95_REGISTER_SYMB_TIMEOUT_LSB = 0x1F,
    RFM95_REGISTER_PREAMBLE_MSB = 0x20,
    RFM95_REGISTER_PREAMBLE_LSB = 0x21,
    RFM95_REGISTER_PAYLOAD_LENGTH = 0x22,
    RFM95_REGISTER_MAX_PAYLOAD_LENGTH = 0x23,
    RFM95_REGISTER_MODEM_CONFIG_3 = 0x26,
    RFM95_REGISTER_INVERT_IQ_1 = 0x33,
    RFM95_REGISTER_SYNC_WORD = 0x39,
    RFM95_REGISTER_INVERT_IQ_2 = 0x3B,
    RFM95_REGISTER_DIO_MAPPING_1 = 0x40,
    RFM95_REGISTER_VERSION = 0x42,
    RFM95_REGISTER_PA_DAC = 0x4D
};


class RFM95W : public StreamDevice {
public:
    RFM95W(SPIDevice *spiDevice, GPIODevice *csPin) : StreamDevice("RFM95W"),
                                                      mSpi(spiDevice),
                                                      csPin(csPin) {}

    RetType init() {
        RESUME();

//        RetType ret = ;
//        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }

    RetType write(uint8_t *buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType read(uint8_t *buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    size_t available() {
        return 0;
    }

    RetType wait(size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType setFrequency(uint32_t frequency) {
        uint64_t freqRespFunc = (static_cast<uint64_t>(frequency) << 19) / 32000000;

        RetType ret = CALL(writeReg(RFM95_REGISTER_FR_MSB, static_cast<uint8_t>(freqRespFunc >> 16)));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(writeReg(RFM95_REGISTER_FR_MID, static_cast<uint8_t>(freqRespFunc >> 8)));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(writeReg(RFM95_REGISTER_FR_LSB, static_cast<uint8_t>(freqRespFunc >> 0)));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }




    RetType reset() {
        RESUME();

        RetType ret = this->nrstPin->set(1);
        if (ret != RET_SUCCESS) return ret;

        // Might need short delay here
        ret = this->nrstPin->set(0);
        if (ret != RET_SUCCESS) return ret;

        // Here as well
        RESET();
        return RET_SUCCESS;
    }

    RetType togglePower() {

    }

private:
    SPIDevice *mSpi;
    GPIODevice *csPin;
    GPIODevice *nrstPin;
    uint16_t txCount;
    uint16_t rxCount;

    RetType writeReg(RFM95_REGISTER_T reg, uint8_t val) {


    }

    RetType readReg(RFM95_REGISTER_T reg, uint8_t *val) {
        RESUME();

        RetType ret =;
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }


};

#endif //LAUNCH_CORE_RFM95W_H
