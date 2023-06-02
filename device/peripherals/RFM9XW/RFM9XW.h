/**
 * Platform Independent RFM (RFM95W, RFM96W, RFM98W) Driver
 *
 * @author Aaron Chan
 * @link https://www.hoperf.com/data/upload/portal/20190801/RFM95W-V2.0.pdf
 *
 */

#ifndef RADIO_MODULE_RFM9XW_H
#define RADIO_MODULE_RFM9XW_H

#include "net/network_layer/NetworkLayer.h"
#include "device/GPIODevice.h"
#include "device/SPIDevice.h"

#define RFM9XW_VERSION 0x12

using RFM9WX_PA_CONFIG_T = struct {
        uint8_t output_power : 4;
        uint8_t max_power : 3;
        uint8_t pa_select : 1;
	};

static constexpr uint8_t RFM9XW_NUM_IRQS = 3;
static constexpr uint8_t DAC_LOW_POWER_MODE = 0x84;
static constexpr uint8_t DAC_HIGH_POWER_MODE = 0x87;
static constexpr uint8_t DIO_MAPPING_1_IRQ_TX_DONE = 0x40;
static constexpr uint8_t DIO_MAPPING_1_IRQ_RX_DONE = 0x00;

using RFM9XW_REGISTER_T = enum {
    RFM9XW_REG_FIFO_ACCESS = 0x00,
    RFM9XW_REG_OP_MODE = 0x01,
    RFM9XW_REG_FR_MSB = 0x06,
    RFM9XW_REG_FR_MID = 0x07,
    RFM9XW_REG_FR_LSB = 0x08,
    RFM9XW_REG_PA_CONFIG = 0x09,
    RFM9XW_REG_LNA = 0x0C,
    RFM9XW_REG_FIFO_ADDR_PTR = 0x0D,
    RFM9XW_REG_FIFO_TX_BASE_ADDR = 0x0E,
    RFM9XW_REG_FIFO_RX_BASE_ADDR = 0x0F,
    RFM9XW_REG_IRQ_FLAGS = 0x12,
    RFM9XW_REG_FIFO_RX_BYTES_NB = 0x13,
    RFM9XW_REG_PACKET_SNR = 0x19,
    RFM9XW_REG_MODEM_CONFIG_1 = 0x1D,
    RFM9XW_REG_MODEM_CONFIG_2 = 0x1E,
    RFM9XW_REG_SYMB_TIMEOUT_LSB = 0x1F,
    RFM9XW_REG_PREAMBLE_MSB = 0x20,
    RFM9XW_REG_PREAMBLE_LSB = 0x21,
    RFM9XW_REG_PAYLOAD_LENGTH = 0x22,
    RFM9XW_REG_MAX_PAYLOAD_LENGTH = 0x23,
    RFM9XW_REG_MODEM_CONFIG_3 = 0x26,
    RFM9XW_REG_DETECTION_OPTIMIZE = 0x31,
    RFM9XW_REG_INVERT_IQ_1 = 0x33,
    RFM9XW_REG_DETECTION_THRESHOLD = 0x37,
    RFM9XW_REG_SYNC_WORD = 0x39,
    RFM9XW_REG_INVERT_IQ_2 = 0x3B,
    RFM9XW_REG_DIO_MAPPING_1 = 0x40,
    RFM9XW_REG_VERSION = 0x42,
    RFM9XW_REG_PA_DAC = 0x4D
};

using RFM9XW_MODE_T = enum {
    RFM9XW_SLEEP_MODE_SLEEP = 0x00,
    RFM9XW_SLEEP_MODE_LORA_SLEEP = 0x80,
    RFM9XW_SLEEP_MODE_STANDBY = 0x81,
    RFM9XW_SLEEP_MODE_TX = 0x83,
    RFM9XW_SLEEP_MODE_RX_SINGLE = 0x86
};

using RFM9XW_RX_MODE_T = enum {
    RFM9XW_RX_MODE_NONE = 0,
    RFM9XW_RX_MODE_1 = 1,
    RFM9XW_RX_MODE_1_2 = 2
};

using RFM9XW_INT_T = enum {
    RFM9XW_INT_DIO0,
    RFM9XW_INT_DIO1,
    RFM9XW_INT_DIO5,
};

class RFM9XW : public NetworkLayer {
public:
    RFM9XW(SPIDevice &spi, GPIODevice &cs, GPIODevice &rst) : m_spi(spi), m_cs(cs), m_rst(rst) {}

    RetType init() {
        RESUME();

        static uint8_t tmp;
        RetType ret = CALL(read_reg(RFM9XW_REG_VERSION, &tmp, 1));
        if (ret != RET_SUCCESS) goto init_end;
        if (tmp != RFM9XW_VERSION) {
            ret = RET_ERROR;
            goto init_end;
        }

        ret = CALL(set_mode(RFM9XW_SLEEP_MODE_SLEEP));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_mode(RFM9XW_SLEEP_MODE_LORA_SLEEP));
        if (ret != RET_SUCCESS) goto init_end;

        // TODO: Don't know enough about RF to config this yet
        // Preamble, LNA, Sync Word set to default
        ret = CALL(set_power(20));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_payload_len(128));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_preamble(0x0, 0x8));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_lna(0b01000000));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_mode(RFM9XW_SLEEP_MODE_LORA_SLEEP));
        if (ret != RET_SUCCESS) goto init_end;

        init_end:
        RESET();
        return ret;
    }

    RetType receive(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_ERROR;
    }

    RetType transmit(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_SUCCESS;
    }

    RetType transmit2(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_ERROR; // TODO
    }





    RetType poll() {
        return RET_SUCCESS;
    }

private:
    SPIDevice &m_spi;
    GPIODevice &m_cs;
    GPIODevice &m_rst;

    uint16_t magic;
    uint16_t rx_frame_count;
    uint16_t tx_frame_count;
    uint8_t rx_one_delay;
    uint32_t channel_frequencies[16];
    uint16_t channel_mask;
    RFM9XW_RX_MODE_T rx_mode;
    uint32_t irq_times[RFM9XW_NUM_IRQS] = {};


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

    RetType set_frequency(uint32_t freq) {
        RESUME();

        static uint64_t new_freq;
        new_freq = (static_cast<uint64_t>(freq) << 19) / 32000000;
        RetType ret = CALL(write_reg(RFM9XW_REG_FR_MSB, static_cast<uint8_t>(new_freq >> 16)));
        if (ret != RET_SUCCESS) goto set_frequency_end;

        ret = CALL(write_reg(RFM9XW_REG_FR_MID, static_cast<uint8_t>(new_freq >> 8)));
        if (ret != RET_SUCCESS) goto set_frequency_end;

        ret = CALL(write_reg(RFM9XW_REG_FR_LSB, static_cast<uint8_t>(new_freq)));

        set_frequency_end:
        RESET();
        return ret;
    }

    RetType set_channel(uint8_t channel) {
        RESUME();

        if (!(channel_mask & (1 << channel))) {
            RESET();
            return RET_ERROR;
        };

        RetType ret = CALL(set_frequency(channel_frequencies[channel]));

        RESET();
        return ret;
    }

    RetType set_power(int8_t power) {
        RESUME();

        RetType ret;
        static uint8_t dac_config;
        static RFM9WX_PA_CONFIG_T config {
                .output_power = 0,
                .max_power = 7,
                .pa_select = 1,
        };


        if (power >= 2 && power <= 17) {
            config.output_power = power - 2;
            dac_config = DAC_LOW_POWER_MODE;
        } else if (power == 20) {
            config.output_power = 15;
            dac_config = DAC_HIGH_POWER_MODE;
        } else {
            RESET();
            return RET_ERROR;
        }

        ret = CALL(write_reg(RFM9XW_REG_PA_CONFIG, reinterpret_cast<uint8_t *>(&config)[0])); // TODO: Check if this is correct
        if (ret != RET_SUCCESS) goto set_power_end;

        ret = CALL(write_reg(RFM9XW_REG_PA_DAC, dac_config));

        set_power_end:
        RESET();
        return ret;
    }

    RetType set_mode(RFM9XW_MODE_T sleep_mode) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_OP_MODE, sleep_mode));

        RESET();
        return ret;
    }

    RetType set_payload_len(size_t len) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_PAYLOAD_LENGTH, len));

        RESET();
        return ret;
    }

    RetType set_preamble(uint8_t msb, uint8_t lsb) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_PREAMBLE_MSB, msb));
        if (ret != RET_SUCCESS) goto set_preamble_end;

        ret = CALL(write_reg(RFM9XW_REG_PREAMBLE_LSB, lsb));

        set_preamble_end:
        RESET();
        return ret;
    }

    RetType set_lna(uint8_t val) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_LNA, val));

        RESET();
        return ret;
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
