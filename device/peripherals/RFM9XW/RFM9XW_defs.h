/**
 * Defines for RFM9XW
 *
 * @author Aaron Chan
 */
#ifndef RFM9XW_DEFS_H
#define RFM9XW_DEFS_H

namespace RFM9XW_DEFS {
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


    // Refer to table 28 and 29 in datasheet
    typedef enum {
        DIO_ZERO_PAY_RDY_PKT_SNT = 0b00,
        DIO_ZERO_CRC_OK = 0b01,
        DIO_ZERO_NO_OP = 0b10,
        DIO_ZERO_TEMP_CHG_LOW_BATT = 0b11,
    } PACKET_DIO_ZERO_MAPPING_T;

    typedef enum {
        DIO_ONE_FIFO_LEVEL = 0b00,
        DIO_ONE_FIFO_EMPTY = 0b01,
        DIO_ONE_FIFO_FULL = 0b10,
        DIO_ONE_NO_OP = 0b11,
    } PACKET_DIO_ONE_MAPPING_T;

    typedef enum {
        DIO_TWO_FIFO_FULL = 0b00,
        DIO_TWO_RX_RDY = 0b01,
        DIO_TWO_FIFO_FULL_TIME_OUT = 0b10,
        DIO_TWO_FIFO_FULL_SYNC_ADDR = 0b11,
    } PACKET_DIO_TWO_MAPPING_T;

    typedef enum {
        DIO_THREE_FIFO_EMPTY = 0b00, // Can also be 0b10 and 0b11
        DIO_THREE_TX_READY = 0b01,
    } PACKET_DIO_THREE_MAPPING_T;

    typedef enum {
        DIO_FOUR_TEMP_CHG_LOW_BATT = 0b00,
        DIO_FOUR_PLL_LOCK = 0b01,
        DIO_FOUR_RX_TIME_OUT = 0b10,
        DIO_FOUR_PREAMBLE_DETECT = 0b11,
    } PACKET_DIO_FOUR_MAPPING_T;

    typedef enum {
        DIO_FIVE_CLK_OUT = 0b00,
        DIO_FIVE_PLL_LOCK = 0b01,
        DIO_FIVE_DATA = 0b10,
        DIO_FIVE_MODE_READY = 0b11,
    } PACKET_DIO_FIVE_MAPPING_T;
}

#endif // RFM9XW_DEFS_H
