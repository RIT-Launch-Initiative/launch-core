/**
 * Platform Independent RFM9xW (RFM95W, RFM96W, RFM98W) Driver using LoRa mode
 *
 * @author Aaron Chan
 * @link https://www.hoperf.com/data/upload/portal/20190801/RFM95W-V2.0.pdf
 *
 */

#ifndef RFM9XW_H
#define RFM9XW_H

#include "RFM9XW_defs.h"
#include "net/network_layer/NetworkLayer.h"
#include "sched/macros.h"
#include "device/GPIODevice.h"
#include "device/SPIDevice.h"
#include "sched/macros.h"

using namespace RFM9XW_DEFS;

using RFM9WX_PA_CONFIG_T = struct {
    uint8_t output_power: 4;
    uint8_t max_power: 3;
    uint8_t pa_select: 1;
};

static constexpr uint8_t RFM9XW_VERSION = 0x12;
static constexpr uint8_t RFM9XW_NUM_IRQS = 3;
static constexpr uint8_t DAC_LOW_POWER_MODE = 0x84;
static constexpr uint8_t DAC_HIGH_POWER_MODE = 0x87;
static constexpr uint8_t DIO_MAPPING_1_IRQ_TX_DONE = 0x40;
static constexpr uint8_t DIO_MAPPING_1_IRQ_RX_DONE = 0x00;
static constexpr uint32_t RX_TIMEOUT = 1000;
static constexpr uint8_t WRITE_MASK = 0x80;


class RFM9XW : public NetworkLayer, public Device {
public:
    explicit RFM9XW(SPIDevice &spi, GPIODevice &cs, GPIODevice &rst, GPIODevice &dio_zero,
                    GPIODevice &dio_one, GPIODevice &dio_two, GPIODevice &dio_three, GPIODevice &dio_four,
                    GPIODevice &dio_five, const bool isTransmitter = true, const char *name = "RFM9XW") : Device(name),
                                                                         m_spi(spi), m_cs(cs), m_rst(rst),
                                                                         m_dio_zero(dio_zero), m_dio_one(dio_one),
                                                                         m_dio_two(dio_two), m_dio_three(dio_three),
                                                                         m_dio_four(dio_four), m_dio_five(dio_five),
                                                                         mIsTransmitter(isTransmitter) {}

    RetType init() {
        RESUME();

        static constexpr uint32_t frequency = 920;
        rx_mode = RFM9XW_RX_MODE_1_2;

        static uint8_t tmp;
        RetType ret = CALL(reset());
        ERROR_CHECK(ret);

        ret = CALL(read_reg(COMMON_REG_VERSION, &tmp, 1));
        FAIL_IF(RET_SUCCESS != ret && tmp != RFM9XW_VERSION);
        if (tmp != RFM9XW_VERSION) {
            RESET();
            return RET_ERROR;
        }

        // Put to LoRa mode. Set to sleep before it can go into standby
        ret = CALL(set_mode(true, false, REG_OP_MODE_SLEEP));
        ERROR_CHECK(ret);

        ret = CALL(set_mode(true, false, REG_OP_MODE_STANDBY));
        ERROR_CHECK(ret);

        // TODO: Don't know enough about RF to config this yet
        // Preamble, LNA, Sync Word set to default

        ret = CALL(set_power(20));
        ERROR_CHECK(ret);

        ret = CALL(set_payload_len(128));
        ERROR_CHECK(ret);

        ret = CALL(set_preamble(0x0, 0x8));
        ERROR_CHECK(ret);

        ret = CALL(set_lna(0b01000000));
        ERROR_CHECK(ret);

        ret = CALL(set_frequency(920));
        ERROR_CHECK(ret);

        ret = CALL(configure_dio_mapping_one(DIO_ZERO_PAY_RDY_PKT_SNT, DIO_ONE_FIFO_FULL, DIO_TWO_RX_RDY));
        ERROR_CHECK(ret);

        ret = CALL(configure_dio_mapping_two(DIO_THREE_TX_READY, DIO_FOUR_RX_TIME_OUT, DIO_FIVE_DATA));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_IRQ_FLAGS, 0xFF)); // Clear all interrupt flags

        if (mIsTransmitter) {
            ret = CALL(setup_transmitter());
        } else {
            ret = CALL(setup_receiver());
        }

        RESET();
        return ret;
    }

    RetType setup_receiver() {
        RESUME();

        m_buff[0] = 0;
        m_buff[0] |= (0b0000 << 4); // Signal bandwidth
        m_buff[0] |= (0b000 << 1); // Coding rate
        m_buff[0] |= (0b0 << 0); // Explicit header mode

        // Config Reg Modem 1
        RetType ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_1, m_buff[0]));
        ERROR_CHECK(ret);

        m_buff[0] = 0;
        m_buff[0] |= (0x07 << 4); // Spreading factor
        m_buff[0] |= (0b0 << 3); // Tx Continuous Mode
        m_buff[0] |= (0b0 << 2); // Rx Payload CrC
        m_buff[0] |= (0x00 << 1); // Symbol Timeout MSB
        ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_2, m_buff[0]));

        RESET();
        return RET_SUCCESS;
    }

    RetType setup_transmitter() {
        RESUME();

        m_buff[0] = 0;
        m_buff[0] |= (0b0000 << 4); // Signal bandwidth
        m_buff[0] |= (0b000 << 1); // Coding rate
        m_buff[0] |= (0b0 << 0); // Explicit header mode

        // Config Reg Modem 1
        RetType ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_1, m_buff[0]));
        ERROR_CHECK(ret);

        m_buff[0] = 0;
        m_buff[0] |= (0x07 << 4); // Spreading factor
        m_buff[0] |= (0b1 << 3); // Tx Continuous Mode
        m_buff[0] |= (0b0 << 2); // Rx Payload CrC
        m_buff[0] |= (0x00 << 1); // Symbol Timeout MSB
        ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_2, m_buff[0]));

        RESET();
        return RET_SUCCESS;
    }

    RetType tx_init(size_t len) {
        RESUME();

        // Set FifoAddrPtr to FifoTxBaseAddr
        RetType ret = CALL(read_reg(LORA_REG_FIFO_TX_BASE_ADDR, m_buff, 1));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_FIFO_ADDR_PTR, m_buff[0]));
        ERROR_CHECK(ret);

        // Set PayloadLength
        ret = CALL(write_reg(RFM9XW_REG_PAYLOAD_LENGTH, len));
        ERROR_CHECK(ret);

        RESET();
        return ret;
    }

    RetType send_data(uint8_t *buff, size_t len) {
        RESUME();

        // Guarantee that we are in standby mode
        RetType ret = CALL(set_mode(true, false, REG_OP_MODE_STANDBY));
        ERROR_CHECK(ret);

        ret = CALL(tx_init(len));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_FIFO_ACCESS, buff, len));

        // Transmit
        ret = CALL(set_mode(RFM9XW_MODE_TX));
        ERROR_CHECK(ret);

        // Wait for TxDone
        while (true) { // TODO: At some point this should just block and be woken by interrupt
            ret = CALL(read_reg(RFM9XW_REG_IRQ_FLAGS, &m_buff[0], 1));
            ERROR_CHECK(ret);

            if (m_buff[0] & 0b00001000) {
                break;
            }

            YIELD();
        }

        RESET();
        return ret;
    }

    RetType receive_data(uint8_t *buff, uint8_t buff_len, uint8_t *rx_len) {
        RESUME();

        RetType ret = CALL(setup_data_receive(rx_len));
        if (RET_SUCCESS == ret) {
            if (*rx_len > buff_len) {
                *rx_len = buff_len; // Prevent an overflow. Up to the user to give a large enough buffer
            }
            ret = CALL(read_reg(RFM9XW_REG_FIFO_RX_CURRENT_ADDR, buff, *rx_len));
        }

        RESET();
        return ret;
    }

    RetType enable_continuous_rx() {
        RESUME();

        RetType ret = CALL(set_mode(RFM9XW_MODE_STANDBY));
        if (RET_SUCCESS == ret) {
            ret = CALL(set_mode(true, true, REG_OP_MODE_CONT_Rx));
        }

        RESET();
        return ret;
    }

    RetType continuous_rx(uint8_t *buff, size_t buff_len, uint8_t *rx_len) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_IRQ_FLAGS, 0xFF)); // Clear all flags
        ERROR_CHECK(ret);

        while (true) {
            // Check for DIO1 interrupt
            ret = CALL(read_reg(RFM9XW_REG_DIO_MAPPING_1, &m_buff[0], 1)); // TODO: Use GPIO interrupts
            if (RET_SUCCESS == ret && (m_buff[0] & 0b00000001) != 0) {
                // Make sure ValidHeader, PayloadCrcError, RxDone and RxTimeout are not asserted
                ret = CALL(read_reg(RFM9XW_REG_IRQ_FLAGS, &m_buff[0], 1));

                if (RET_SUCCESS == ret && ((0b11110000 & m_buff[0]) == 0)) {
                    break;
                }
            }

            YIELD();
        }
        ret = CALL(receive_data(buff, buff_len, rx_len));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_IRQ_FLAGS, 0xFF)); // Clear IRQ flags
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_DIO_MAPPING_1, 0x00)); // Clear interrupt flags


        RESET();
        return ret;
    }

    RetType transmit(Packet &, netinfo_t &, NetworkLayer *) override {
        return RET_SUCCESS;
    }

    RetType transmit2(Packet &packet, netinfo_t &, NetworkLayer *) override {
        RESUME();

        // Configure modem
        RetType ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_1, 0x72));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_2, 0x74));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_3, 0x04));
        ERROR_CHECK(ret);

        // Payload Length
        ret = CALL(set_payload_len(packet.size()));
        ERROR_CHECK(ret);

        // IQ Registers
        ret = CALL(write_reg(RFM9XW_REG_INVERT_IQ_1, 0x27));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_INVERT_IQ_2, 0x1D));
        ERROR_CHECK(ret);

        // LORA Standby
        ret = CALL(set_mode(RFM9XW_MODE_STANDBY));
        ERROR_CHECK(ret);

        // TODO: Interrupt
        SLEEP(timeout_time);

        // FIFO Write
        ret = CALL(write_reg(RFM9XW_REG_FIFO_ADDR_PTR, 0x80));
        ERROR_CHECK(ret);

        packet.seek_read(false);
        current_tx_buff = packet.read_ptr<uint8_t>();
        current_tx_buff_end = current_tx_buff + packet.available();
        for (; current_tx_buff < current_tx_buff_end; current_tx_buff++) {
            ret = CALL(write_reg(RFM9XW_REG_FIFO_ACCESS, *current_tx_buff));
            ERROR_CHECK(ret);
        }

        // Transmit
        ret = CALL(set_mode(RFM9XW_MODE_TX));
        ERROR_CHECK(ret);

        // Wait
        ret = CALL(wait_for_irq(RFM9XW_INT_DIO0, timeout_time));
        SLEEP(5000);

        ret = CALL(set_mode(RFM9XW_MODE_STANDBY));
        if (RET_SUCCESS == ret) {
            tx_frame_count++;
        }

        RESET();
        return ret;
    }

    RetType receive(Packet &, netinfo_t &, NetworkLayer *) override {
        return RET_ERROR;
    }

    RetType recv_data(uint8_t *buff, size_t *len, int8_t *snr, uint32_t tx_ticks) {
        RESUME();
        *len = 0;

        static uint32_t rx1_target;
        static uint32_t rx1_window_symbols;
        static uint8_t irq_flags;
        static int8_t packet_snr;
        static uint8_t calc_len;

//        calculate_rx_timings(125000, 7, tx_ticks, &rx1_target, &rx1_window_symbols);

        if (rx1_window_symbols > 0x3ff) {
            RESET();
            return RET_ERROR;
        }

        // Configure modem (125kHz, 4/6 error coding rate, SF7, single packet, CRC enable, AGC auto on)
        RetType ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_1, 0x72));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_2, 0x74 | ((rx1_window_symbols >> 8) & 0x3)));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_MODEM_CONFIG_3, rx1_window_symbols & 0xff));
        ERROR_CHECK(ret);

        // Set maximum symbol timeout.
        ret = CALL(write_reg(RFM9XW_REG_SYMB_TIMEOUT_LSB, rx1_window_symbols));
        ERROR_CHECK(ret);

        // Set IQ registers according to AN1200.24.
        ret = CALL(write_reg(RFM9XW_REG_INVERT_IQ_1, RFM9XW_REG_INVERT_IQ_1));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_INVERT_IQ_2, RFM9XW_REG_INVERT_IQ_2));
        ERROR_CHECK(ret);

        // TODO: Interrupts
        SLEEP(timeout_time);

        ret = CALL(read_reg(RFM9XW_REG_IRQ_FLAGS, &irq_flags, 1));
        ERROR_CHECK(ret);

        // CRC Check
        if (irq_flags & 0x20) {
            RESET();
            return RET_ERROR;
        }

        ret = CALL(read_reg(RFM9XW_REG_PACKET_SNR, reinterpret_cast<uint8_t *>(&packet_snr), 1));

        *snr = packet_snr / 4;

        // Read packet len and then data
        ret = CALL(read_reg(RFM9XW_REG_FIFO_RX_BYTES_NB, &calc_len, 1));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_FIFO_ADDR_PTR, 0));
        ERROR_CHECK(ret);

        ret = CALL(read_reg(RFM9XW_REG_FIFO_ACCESS, buff, calc_len));
        ERROR_CHECK(ret);

        // Return modem to sleep.
        ret = CALL(set_mode(RFM9XW_MODE_LORA_SLEEP));
        ERROR_CHECK(ret);

        // Successful payload receive, set payload length to tell caller.
        *len = calc_len;

        RESET();
        return ret;

    }

    RetType poll() {
        return RET_SUCCESS;
    }

private:
    const bool mIsTransmitter;
    uint8_t rx_buff[256] = {0};
    uint32_t m_gpio_tmp = 0;
    uint8_t *current_tx_buff = nullptr;
    uint8_t *current_tx_buff_end = nullptr;

    SPIDevice &m_spi;
    GPIODevice &m_cs;
    GPIODevice &m_rst;
    GPIODevice &m_dio_zero;
    GPIODevice &m_dio_one;
    GPIODevice &m_dio_two;
    GPIODevice &m_dio_three;
    GPIODevice &m_dio_four;
    GPIODevice &m_dio_five;

    uint8_t m_buff[16]{};

    uint16_t magic{};
    uint16_t rx_frame_count{};
    uint16_t tx_frame_count{};
    uint8_t rx_one_delay{};
    uint32_t channel_frequencies[16]{};
    uint16_t channel_mask{};
    RFM9XW_RX_MODE_T rx_mode;
    uint32_t irq_times[RFM9XW_NUM_IRQS] = {};
    uint32_t precision_tick_freq{};
    uint32_t timeout_time = 10;

    RetType check_rx_termination(void) {
        RESUME();

        RetType ret = CALL(read_reg(RFM9XW_REG_IRQ_FLAGS, m_buff, 1));
//        if (RET_SUCCESS == ret) {
//            // ValidHeader, PayloadCrcError, RxDone and RxTimeout
//            constexpr uint8_t mask = 0b11110000;
//            if (0 != (m_buff[0] & mask)) {
//                ret = RET_ERROR;
//            }
//        }

        RESET();
        return ret;
    }

    RetType setup_data_receive(uint8_t *rx_len) {
        RESUME();

        // Set FifoAddrPtr to FifoRxBaseAddr
        RetType ret = CALL(read_reg(LORA_REG_FIFO_RX_BYTE_ADDR, m_buff, 1));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_FIFO_ADDR_PTR, m_buff[0]));

        // Read number of bytes received
        ret = CALL(read_reg(RFM9XW_REG_FIFO_RX_BYTES_NB, rx_len, 1));

        RESET();
        return ret;
    }

    //TODO
    RetType wait_for_irq(const RFM9XW_INT_T irq, const uint32_t timeout) {
        RESUME();
        static uint32_t timeout_time;
        timeout_time = sched_time() + timeout * precision_tick_freq / 1000;

        while (irq_times[irq] == 0) {
            FAIL_IF(sched_time() >= timeout_time);
            YIELD();
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType wait_for_rx_irq(void) {
        RESUME();
        static uint32_t timeout_time;
        timeout_time = sched_time() + RX_TIMEOUT * precision_tick_freq / 1000;

        while (irq_times[RFM9XW_INT_DIO0] == 0 && irq_times[RFM9XW_INT_DIO1] == 0) {
            if (sched_time() >= timeout_time) {
                RESET();
                return RET_ERROR;
            }

            YIELD();
        }

        RESET();
        return RET_SUCCESS;
    }

    RetType read_reg(const uint8_t reg, uint8_t *buff, const size_t len, const uint32_t timeout = 0) {
        RESUME();

        RetType ret = CALL(m_cs.set(0));

        memset(buff, 0, len);
        buff[0] = reg & 0x7FU;

        // TODO: Write one byte and then proceed to read or is this ok?

        ret = CALL(m_spi.write(buff, len));
        ERROR_CHECK(ret);

        ret = CALL(m_spi.read(buff, len));
        ERROR_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
    }

    RetType write_reg(const uint8_t reg, const uint8_t val) {
        RESUME();

        m_buff[0] = reg | WRITE_MASK;
        m_buff[1] = val;

        RetType ret = CALL(m_cs.set(0));

        ret = CALL(m_spi.write(m_buff, 2));
        ERROR_CHECK(ret);

//        ret = CALL(m_spi.write(&m_buff[0], 1));
//        ERROR_CHECK(ret);
//
//        ret = CALL(m_spi.write(&m_buff[1], 1));

        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
    }

    RetType write_reg(const uint8_t reg, const uint8_t *buff, const size_t len) {
        RESUME();

        m_buff[0] = reg | WRITE_MASK;

        RetType ret = CALL(m_cs.set(0));

        ret = CALL(m_spi.write(m_buff, 1));
        if (RET_SUCCESS == ret) {
            ret = CALL(m_spi.write(const_cast<uint8_t *>(buff), len));
        } else {
            ret = RET_ERROR;
        }

        CALL(m_cs.set(1));
        RESET();
        return ret;
    }

    RetType set_frequency(const uint32_t freq) {
        RESUME();

        static uint64_t new_freq;
        new_freq = (static_cast<uint64_t>(freq * 1000000) << 19) / 32000000;
        RetType ret = CALL(write_reg(RFM9XW_REG_FR_MSB, static_cast<uint8_t>(new_freq >> 16)));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_FR_MID, static_cast<uint8_t>(new_freq >> 8)));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_FR_LSB, static_cast<uint8_t>(new_freq)));

        RESET();
        return ret;
    }

    RetType set_channel(const uint8_t channel) {
        RESUME();

        if (!(channel_mask & (1 << channel))) {
            RESET();
            return RET_ERROR;
        };

        RetType ret = CALL(set_frequency(channel_frequencies[channel]));

        RESET();
        return ret;
    }

    RetType set_power(const int8_t power) {
        RESUME();

        RetType ret;
        static uint8_t dac_config;
        static RFM9WX_PA_CONFIG_T config{
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

        ret = CALL(write_reg(RFM9XW_REG_PA_CONFIG,
                             reinterpret_cast<uint8_t *>(&config)[0])); // TODO: Check if this is correct
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_PA_DAC, dac_config));

        RESET();
        return ret;
    }

    RetType set_mode(const RFM9XW_MODE_T sleep_mode) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_OP_MODE, sleep_mode));

        RESET();
        return ret;
    }

    RetType set_mode(const bool lora_mode, const bool low_freq_mode, const RFM9XW_REG_OP_MODE_T mode,
                     const bool ook_modulation = false) {
        RESUME();

        m_buff[0] = 0;
        if (!lora_mode && ook_modulation) {
            m_buff[0] |= 0b01 << 5;
        } else {
            m_buff[0] |= 0b1 << 7;
        }

        if (low_freq_mode) {
            m_buff[0] |= 0b1 << 3;
        }

        m_buff[0] |= mode;
        RetType ret = CALL(write_reg(RFM9XW_REG_OP_MODE, m_buff[0]));

        RESET();
        return ret;
    }


    RetType set_payload_len(const size_t len) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_PAYLOAD_LENGTH, len));

        RESET();
        return ret;
    }

    RetType set_preamble(const uint8_t msb, const uint8_t lsb) {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_PREAMBLE_MSB, msb));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_PREAMBLE_LSB, lsb));

        RESET();
        return ret;
    }

    RetType set_lna(const uint8_t val) {
        RESUME();

        RetType ret = CALL(write_reg(COMMON_REG_LNA, val));

        RESET();
        return ret;
    }

    RetType get_valid_rx_headers(uint16_t *val) {
        RESUME();

        RetType ret = CALL(read_reg(LORA_REG_RX_HEADER_CNT_VALUE_MSB, &m_buff[0], 2));
        ERROR_CHECK(ret);

//        ret = CALL(read_reg(LORA_REG_RX_HEADER_CNT_VALUE_LSB, &m_buff[1]));
//        ERROR_CHECK(ret);

        *val = (m_buff[0] << 8) | m_buff[1];

        RESET();
        return RET_SUCCESS;
    }

    RetType configure_fsk_packet(const bool variable_len = true, const uint8_t dc_free_encoding = 0b00,
                                 const bool crc_enabled = false,
                                 const bool crc_auto_clear_off = false, const uint8_t addr_filter = 0b00,
                                 const bool ibm_whitening = false) {
        RESUME();

        m_buff[0] = 0;
        if (variable_len) m_buff[0] |= 0b1 << 7;
        m_buff[0] |= (dc_free_encoding & 0b11) << 5;
        if (crc_enabled) m_buff[0] |= 0b1 << 4;
        if (crc_auto_clear_off) m_buff[0] |= 0b1 << 3;
        m_buff[0] |= (addr_filter & 0b11) << 1;
        if (ibm_whitening) m_buff[0] |= 0b1;

        RetType ret = CALL(write_reg(RFM9XW_REG_PACKET_CONFIG_1, m_buff[0]));

        RESET();
        return RET_SUCCESS;
    }

    RetType
    configure_dio_mapping_one(const uint8_t dio_zero_val, const uint8_t dio_one_val, const uint8_t dio_two_val) {
        RESUME();

        m_buff[0] = (dio_two_val << 6) | (dio_one_val << 4) | dio_zero_val;
        RetType ret = CALL(write_reg(COMMON_REG_DIO_MAPPING_1, m_buff[0]));

        RESET();
        return ret;
    }

    RetType
    configure_dio_mapping_two(const uint8_t dio_three_val, const uint8_t dio_four_val, const uint8_t dio_five_val) {
        RESUME();

        m_buff[0] = (dio_three_val << 6) | (dio_four_val << 4) | dio_five_val;
        RetType ret = CALL(write_reg(COMMON_REG_DIO_MAPPING_2, m_buff[0]));

        RESET();
        return ret;
    }

    RetType clear_irq_flags() {
        RESUME();

        RetType ret = CALL(write_reg(RFM9XW_REG_IRQ_FLAGS, 0xFF));

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

#endif //RFM9XW_H
