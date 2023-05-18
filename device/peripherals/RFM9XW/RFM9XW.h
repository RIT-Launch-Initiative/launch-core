//
// Created by aaron on 5/18/23.
//

#ifndef RADIO_MODULE_RFM9XW_H
#define RADIO_MODULE_RFM9XW_H

#include "net/network_layer/NetworkLayer.h"
#include "device/GPIODevice.h"
#include "device/SPIDevice.h"

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
};

#endif //RADIO_MODULE_RFM9XW_H
