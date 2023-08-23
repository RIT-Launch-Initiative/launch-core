/**
 * Platform Independent RFM (RFM95W, RFM96W, RFM98W) Driver
 *
 * @author Aaron Chan
 * @link https://www.hoperf.com/data/upload/portal/20190801/RFM95W-V2.0.pdf
 *
 */

#ifndef RFM9XW_H
#define RFM9XW_H

#include "net/network_layer/NetworkLayer.h"
#include "sched/macros.h"
#include "device/GPIODevice.h"
#include "device/SPIDevice.h"
#include "sched/macros.h"


using RFM9WX_PA_CONFIG_T = struct {
        uint8_t output_power : 4;
        uint8_t max_power : 3;
        uint8_t pa_select : 1;
	};

static constexpr uint8_t RFM9XW_VERSION = 0x12;
static constexpr uint8_t RFM9XW_NUM_IRQS = 3;
static constexpr uint8_t DAC_LOW_POWER_MODE = 0x84;
static constexpr uint8_t DAC_HIGH_POWER_MODE = 0x87;
static constexpr uint8_t DIO_MAPPING_1_IRQ_TX_DONE = 0x40;
static constexpr uint8_t DIO_MAPPING_1_IRQ_RX_DONE = 0x00;
static constexpr uint32_t RX_TIMEOUT = 1000;



class RFM9XW : public NetworkLayer {
    typedef enum {
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
        RFM9XW_REG_FIFO_RX_CURRENT_ADDR = 0x10,
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
    } RFM9XW_REGISTER_T;

    typedef enum {
        RFM9XW_MODE_SLEEP = 0x00,
        RFM9XW_MODE_LORA_SLEEP = 0x80,
        RFM9XW_MODE_STANDBY = 0x81,
        RFM9XW_MODE_TX = 0x83,
        RFM9XW_MODE_RX_SINGLE = 0x86
    } RFM9XW_MODE_T;

    typedef enum {
        RFM9XW_REG_OP_MODE_SLEEP = 0b000,
        RFM9XW_REG_OP_MODE_STANDBY = 0b001,
        RFM9XW_REG_OP_MODE_FSTx = 0b010,
        RFM9XW_REG_OP_MODE_Tx = 0b011,
        RFM9XW_REG_OP_MODE_FSRx = 0b100,
        RFM9XW_REG_OP_MODE_Rx = 0b101,
    } RFM9XW_REG_OP_MODE_T;

    typedef enum {
        RFM9XW_RX_MODE_NONE = 0,
        RFM9XW_RX_MODE_1 = 1,
        RFM9XW_RX_MODE_1_2 = 2
    } RFM9XW_RX_MODE_T;

    typedef enum {
        RFM9XW_INT_DIO0,
        RFM9XW_INT_DIO1,
        RFM9XW_INT_DIO5,
    } RFM9XW_INT_T;

public:
    explicit RFM9XW(SPIDevice &spi, GPIODevice &cs, GPIODevice &rst,
                    GPIODevice &dio_zero, GPIODevice &dio_one, GPIODevice &dio_two,
                    GPIODevice &dio_three, GPIODevice &dio_four, GPIODevice &dio_five
                    ) : m_spi(spi), m_cs(cs), m_rst(rst), m_dio_zero(dio_zero), m_dio_one(dio_one),
                    m_dio_two(dio_two), m_dio_three(dio_three), m_dio_four(dio_four), m_dio_five(dio_five)  {}

    RetType init() {
        RESUME();

        constexpr uint32_t frequency = 920;
        rx_mode = RFM9XW_RX_MODE_1_2;

        static uint8_t tmp;
        RetType ret = CALL(reset());
        ERROR_CHECK(ret);

        ret = CALL(read_reg(RFM9XW_REG_VERSION, &tmp, 1));
        FAIL_IF(RET_SUCCESS != ret && tmp != RFM9XW_VERSION);

        ret = CALL(set_mode(true, true, RFM9XW_REG_OP_MODE_STANDBY));
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
        ret = CALL(set_mode(RFM9XW_MODE_LORA_SLEEP));
        ERROR_CHECK(ret);

        ret = CALL(set_frequency(920));

        RESET();
        return ret;
    }

    RetType send_data(uint8_t *buff, size_t len) {
        RESUME();

        // Guarantee that we are in standby mode
        RetType ret = CALL(set_mode(RFM9XW_MODE_STANDBY));
        ERROR_CHECK(ret);

        // Set to Tx mode
        ret = CALL(set_mode(RFM9XW_MODE_TX));
        ERROR_CHECK(ret);

        // Set FifoAddrPtr to FifoTxBaseAddr
        ret = CALL(write_reg(RFM9XW_REG_FIFO_ADDR_PTR, 0x80));
        ERROR_CHECK(ret);

        // Set PayloadLength
        ret = CALL(write_reg(RFM9XW_REG_PAYLOAD_LENGTH, len));
        ERROR_CHECK(ret);

        // Write data to FIFO
        ret = CALL(write_reg(RFM9XW_REG_FIFO_ACCESS, buff, len));
        ERROR_CHECK(ret);

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

        ret = CALL(set_mode(RFM9XW_MODE_STANDBY));

        RESET();
        return ret;
    }

    RetType receive_data(uint8_t *buff, size_t buff_len, uint8_t *rx_len) {
        RESUME();

        RetType ret = CALL(check_rx_termination());
        ERROR_CHECK(ret);

        ret = CALL(setup_data_receive(rx_len));
        if (RET_SUCCESS == ret) {
            if (*rx_len > buff_len) { // Prevent an overflow. Up to the user to give a large enough buffer
                ret = CALL(read_reg(RFM9XW_REG_FIFO_RX_CURRENT_ADDR, buff, buff_len));
            } else {
                ret = CALL(read_reg(RFM9XW_REG_FIFO_RX_CURRENT_ADDR, buff, *rx_len));
            }
        }

        RESET();
        return ret;
    }

    RetType transmit(Packet&, netinfo_t&, NetworkLayer*) override {
        return RET_SUCCESS;
    }

    RetType transmit2(Packet &packet, netinfo_t&, NetworkLayer*) override {
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

    RetType receive(Packet&, netinfo_t&, NetworkLayer*) override {
        return RET_ERROR;
    }

    RetType recv_data(uint8_t *buff, size_t *len, int8_t* snr, uint32_t tx_ticks) {
        RESUME();
        *len = 0;

        static uint32_t rx1_target;
        static uint32_t rx1_window_symbols;
        static uint8_t irq_flags;
        static int8_t packet_snr;
        static uint8_t calc_len;

        calculate_rx_timings(125000, 7, tx_ticks, &rx1_target, &rx1_window_symbols);

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

    uint8_t m_buff[16];

    uint16_t magic;
    uint16_t rx_frame_count;
    uint16_t tx_frame_count;
    uint8_t rx_one_delay;
    uint32_t channel_frequencies[16];
    uint16_t channel_mask;
    RFM9XW_RX_MODE_T rx_mode;
    uint32_t irq_times[RFM9XW_NUM_IRQS] = {};
    uint32_t precision_tick_freq;
    uint32_t timeout_time = 10;

    RetType check_rx_termination() {
        RESUME();

        RetType ret = CALL(read_reg(RFM9XW_REG_IRQ_FLAGS, m_buff, 1));
        if (RET_SUCCESS == ret) {
            // ValidHeader, PayloadCrcError, RxDone and RxTimeout
            constexpr uint8_t mask = 0b11110000;
            if (0 != (m_buff[0] & mask)) {
                ret = RET_ERROR;
            }
        }



        RESET();
        return ret;
    }

    RetType setup_data_receive(uint8_t *rx_len) {
        RESUME();

        // Set FifoAddrPtr to FifoRxBaseAddr
        RetType ret = CALL(read_reg(RFM9XW_REG_FIFO_ADDR_PTR, m_buff, 1));
        ERROR_CHECK(ret);

        ret = CALL(write_reg(RFM9XW_REG_FIFO_ADDR_PTR, m_buff[0]));

        // Read number of bytes received
        ret = CALL(read_reg(RFM9XW_REG_FIFO_RX_BYTES_NB, rx_len, 1));

        RESET();
        return RET_SUCCESS;
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

    RetType wait_for_rx_irq() {
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

    RetType read_reg(const uint8_t reg, uint8_t* buff, const size_t len) {
        RESUME();

        RetType ret = CALL(m_cs.set(0));

        memset(buff, 0, len);
        buff[0] = reg & 0x7FU;

        // TODO: Write one byte and then proceed to read or is this ok?
        ret = CALL(m_spi.write_read(buff, buff, len));
        ERROR_CHECK(ret);

//        ret = CALL(m_spi.write(buff, len));
//        ERROR_CHECK(ret);
//
//        ret = CALL(m_spi.read(buff, len));
//        ERROR_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
    }

    RetType write_reg(const uint8_t reg, const uint8_t val) {
        RESUME();

        m_buff[0] = reg | 0x80U;
        m_buff[1] = val;

        RetType ret = CALL(m_cs.set(0));

        ret = CALL(m_spi.write(m_buff, 2));
        ERROR_CHECK(ret);

        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
    }

    RetType write_reg(const uint8_t reg, const uint8_t *buff, const size_t len) {
        RESUME();

        m_buff[0] = reg | 0x80U;

        RetType ret = CALL(m_cs.set(0));

        ret = CALL(m_spi.write(m_buff, 1));
        if (RET_SUCCESS == ret) {
            ret = CALL(m_spi.write(const_cast<uint8_t *>(buff), len));
        }

        ret = CALL(m_cs.set(1));
        RESET();
        return RET_SUCCESS;
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

    RetType set_mode(bool lora_mode, bool low_freq_mode, RFM9XW_REG_OP_MODE_T mode, bool ook_modulation = false) {
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

    void calculate_rx_timings(const uint32_t bw, const uint8_t sf, const uint32_t tx_ticks, uint32_t* const rx_target, uint32_t* const rx_window_symbols) {
    }



};

#endif //RFM9XW_H
