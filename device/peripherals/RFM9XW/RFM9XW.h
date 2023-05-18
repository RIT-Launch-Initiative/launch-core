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

class RFM95WX : public NetworkLayer {
public:
    RFM95WX(SPIDevice &spi, GPIODevice &cs, GPIODevice &rst) : m_spi(spi), m_cs(cs), m_rst(rst)  {}


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
        buff[0] = (reg | 0x80U);
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
