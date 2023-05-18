//
// Created by aaron on 5/18/23.
//

#ifndef RADIO_MODULE_RFM9XW_H
#define RADIO_MODULE_RFM9XW_H

#include "net/network_layer/NetworkLayer.h"
#include "device/GPIODevice.h"
#include "device/SPIDevice.h"

using RFM9XW_EEPROM_CONFIG_T = struct {
    uint16_t magic;
    uint16_t rx_frame_count;
    uint16_t tx_frame_count;
    uint8_t rx_one_delay;
    uint32_t channel_frequencies[16];
    uint16_t channel_mask;
};

using RFM9XW_REGISTER_T = enum {
    RFM9X_FIFO_ACCESS = 0x00,
    RFM9X_OP_MODE = 0x01,

    RFM9X_FR_MSB = 0x06,
    RFM9X_FR_MID = 0x07,
    RFM9X_FR_LSB = 0x08,

    RFM9X_VERSION = 0x42,
};

class RFM9XW : public NetworkLayer {
public:
    RFM9XW(SPIDevice &spi, GPIODevice &cs, GPIODevice &rst) : m_spi(spi), m_cs(cs), m_rst(rst)  {}

    RetType init() {
        RESUME();

        static uint8_t tmp;
        RetType ret = CALL(read_reg(RFM9X_VERSION, &tmp, 1));
        if (ret != RET_SUCCESS) goto init_end;


        init_end:
        RESET();
        return ret;
    }

    RetType receive(Packet& packet, netinfo_t& info, NetworkLayer* caller) override {
        return RET_ERROR; // TODO:
    }

    RetType transmit(Packet& packet, netinfo_t& info, NetworkLayer* caller) override {
        return RET_SUCCESS;
    }

    RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) override {
        return RET_ERROR; // TODO
    }

private:
    SPIDevice &m_spi;
    GPIODevice &m_cs;
    GPIODevice &m_rst;

    RetType read_reg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();

        RetType ret = CALL(m_cs.set(0));

        memset(buff, 0, len);
        buff[0] = reg & 0x7FU;

        // TODO: Write one byte and then proceed to read or is this ok?
        ret = CALL(m_spi.write_read(buff, buff, len));
        if (ret != RET_SUCCESS) goto read_reg_end;

        read_reg_end:
        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
    }

    RetType write_reg(uint8_t reg, uint8_t val) {
        RESUME();

        static uint8_t buff[2];
        buff[0] = reg | 0x80U;
        buff[1] = reg;

        RetType ret = CALL(m_cs.set(0));

        ret = CALL(m_spi.write(buff, 2));
        if (ret != RET_SUCCESS) goto write_reg_end;

        write_reg_end:
        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
    }

    RetType reset() {
        RESUME();
        CALL(m_rst.set(0));
        SLEEP(50);
        CALL(m_rst.set(1));
        RESET();
        return RET_SUCCESS;
    }
};

#endif //RADIO_MODULE_RFM9XW_H
