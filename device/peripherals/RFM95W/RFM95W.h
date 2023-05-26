/**
 * RFM95W LoRa Driver
 *
 * @author Aaron Chan, Jonathan Russo
 */

#ifndef LAUNCH_CORE_RFM95W_H
#define LAUNCH_CORE_RFM95W_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "return.h"
#include "sched/macros/resume.h"
#include "sched/macros/call.h"
#include "sched/macros/reset.h"
#include "device/SPIDevice.h"
#include "device/StreamDevice.h"
#include "device/GPIODevice.h"

#include "main.h"
#include "device/platforms/stm32/HAL_GPIODevice.h"
#include "device/platforms/stm32/HAL_UARTDevice.h"
#include "device/platforms/stm32/HAL_SPIDevice.h"

#define RFM9x_VER 0x12

#define FREQ_915 15169815u

#define RFM_FIFO_SIZE 255

#define RFM_MAX_PAYLOAD_LENGTH 255

#define RFM_HEADER_LEN 4

#define RFM_MAX_MESSAGE_LEN (RFM_MAX_PAYLOAD_LENGTH - RFM_HEADER_LEN)


enum RFM_MODEM_CONFIG_T {
    RFM_Bw3 = (1 << 7),
    RFM_Bw2 = (1 << 6),
    RFM_Bw1 = (1 << 5),
    RFM_Bw0 = (1 << 4),
    RFM_CodingRate2 = (1 << 3),
    RFM_CodingRate1 = (1 << 2),
    RFM_CodingRate0 = (1 << 1),
    RFM_ImplicitHeaderModeOn = (1 << 0)
};

enum RFM_REG_OP_MODE_T {
    RFM_LongRangeMode = (1 << 7),
    RFM_AccessSharedReg = (1 << 6),
    RFM_Mode2 = (1 << 2),
    RFM_Mode1 = (1 << 1),
    RFM_Mode0 = (1 << 0)
};


enum RFM_MODE_T {
    RFM_MODE_SLEEP = 0x00,
    RFM_MODE_STANDBY = 0x01,
    RFM_MODE_TX = 0x03,
    RFM_MODE_RXCONTINUOUS = 0x05,
    RFM_MODE_RXSINGLE = 0x06,
    RFM_MODE_CAD = 0x07
};

/* 0x09 RegPaConfig */

enum REG_PA_CONFIG_T {
    RFM_PaSelect = (1 << 7),
    RFM_MaxPower2 = (1 << 6),
    RFM_MaxPower1 = (1 << 5),
    RFM_MaxPower0 = (1 << 4),
    RFM_OutputPower3 = (1 << 3),
    RFM_OutputPower2 = (1 << 2),
    RFM_OutputPower1 = (1 << 1),
    RFM_OutputPower0 = (1 << 0)
};

/* 0x12 RegIrqFlags */
enum RFM_IRQ_FLAGS_T {
    RFM_RxTimeout = (1 << 7),
    RFM_RxDone = (1 << 6),
    RFM_PayloadCrcError = (1 << 5),
    RFM_ValidHeader = (1 << 4),
    RFM_TxDone = (1 << 3),
    RFM_CadDone = (1 << 2),
    RFM_FhssChangeChannel = (1 << 1),
    RFM_CadDetected = (1 << 0)
};

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
    RFM95_REGISTER_DETECTION_OPTIMIZE = 0x31,
    RFM95_REGISTER_INVERT_IQ_1 = 0x33,
    RFM95_REGISTER_DETECTION_THRESHOLD = 0x37,
    RFM95_REGISTER_SYNC_WORD = 0x39,
    RFM95_REGISTER_INVERT_IQ_2 = 0x3B,
    RFM95_REGISTER_DIO_MAPPING_1 = 0x40,
    RFM95_REGISTER_VERSION = 0x42,
    RFM95_REGISTER_PA_DAC = 0x4D
};


class RFM95W {
public:
    RFM95W(SPIDevice *spiDevice, GPIODevice *csPin, GPIODevice *nrstPin) : mSpi(spiDevice),
                                                                           csPin(csPin),
                                                                           nrstPin(nrstPin) {}

    RetType init() {
        RESUME();
	
        // add potential asserts here
        static uint8_t RegOpMode;
        static uint8_t RegModemConfig1;
        static uint8_t RegModemConfig2;
        static uint8_t version = 0;

        RetType ret = CALL(reset());
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
		
		/*
        ret = CALL(readReg(RFM95_REGISTER_VERSION, &version, 1));
        if (version != RFM9x_VER) {
            RESET();
            return RET_ERROR;

        }
*/
        //place module in sleep mode
        ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, 0x00));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, 0x80)); // turn on LoRa
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
		
		//set to default interrupt config
        ret = CALL(writeReg(RFM95_REGISTER_DIO_MAPPING_1, 0x00));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
		
		//set power
        ret = CALL(setPower(17));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
		
		//Set to maximum gain
        ret = CALL(writeReg(RFM95_REGISTER_LNA, 0x23));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }		
		
		// Set BW to 500K
        RegModemConfig1 = RFM_Bw3 | RFM_Bw0;
        /* Set coding rate to 4/8 -> 100 */
        RegModemConfig1 |= RFM_CodingRate1;
        /* Implicit header mode */
        RegModemConfig1 &= ~RFM_ImplicitHeaderModeOn;

        ret = CALL(writeReg(RFM95_REGISTER_MODEM_CONFIG_1, RegModemConfig1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
	

        /* Set SF9 = 256 chips/symbol */
//	    RegModemConfig2 = RFM_SpreadingFactor3; // TODO: Uncomment when these are defined
        /* Enable CRCs: */
//	    RegModemConfig2 |= RFM_RxPayloadCrcOn;

		/*
        ret = CALL(writeReg(RFM95_REGISTER_MODEM_CONFIG_2, RegModemConfig2));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
		
		*/

        //config interrupts

        //set frequency to 915 MHz
        ret = CALL(configFrequency(FREQ_915));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        //set preamble to 8 + 4.25 = 12.25 symbols.
        ret = CALL(writeReg(RFM95_REGISTER_PREAMBLE_MSB, 0x00));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
        ret = CALL(writeReg(RFM95_REGISTER_PREAMBLE_LSB, 0x08));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        //set ttn sync word 0x34
        ret = CALL(writeReg(RFM95_REGISTER_SYNC_WORD, 0x34));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        //set up base addresses for rx and tx
        ret = CALL(writeReg(RFM95_REGISTER_FIFO_TX_BASE_ADDR, 0x80));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
        ret = CALL(writeReg(RFM95_REGISTER_FIFO_RX_BASE_ADDR, 0x00));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Set Maximum payload length to 64
        ret = CALL(writeReg(RFM95_REGISTER_MAX_PAYLOAD_LENGTH, 64));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        // Let module sleep after init
        ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, 0x80));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }


        RESET();
        return RET_SUCCESS;
    }

    RetType configFrequency(uint32_t frequency) {
        RESUME();
        static uint64_t freqRespFunc = (static_cast<uint64_t>(frequency) << 19) / 32000000;

        RetType ret = CALL(writeReg(RFM95_REGISTER_FR_MSB, static_cast<uint8_t>(freqRespFunc >> 16)));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_FR_MID, static_cast<uint8_t>(freqRespFunc >> 8)));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_FR_LSB, static_cast<uint8_t>(freqRespFunc >> 0)));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;
    }


    RetType reset() {
        RESUME();

        RetType ret = this->nrstPin->set(0);
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        SLEEP(100);
        ret = this->nrstPin->set(1);
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        SLEEP(100);
        RESET();
        return RET_SUCCESS;
    }


    RetType setSpreadingFactor(int sf) {
        RESUME();

        if (sf < 6) {
            sf = 6;
        } else if (sf > 12) {
            sf = 12;
        }

        RetType ret;
        if (sf == 6) {
            ret = CALL(writeReg(RFM95_REGISTER_DETECTION_OPTIMIZE, 0xc5));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
            ret = CALL(writeReg(RFM95_REGISTER_DETECTION_THRESHOLD, 0x0c));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
        } else {
            ret = CALL(writeReg(RFM95_REGISTER_DETECTION_OPTIMIZE, 0xc3));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
            ret = CALL(writeReg(RFM95_REGISTER_DETECTION_THRESHOLD, 0x0a));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
        }

        // writeRegister(REG_MODEM_CONFIG_2, (readRegister(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));


        RESET();
        return RET_SUCCESS;
    }


    RetType setPower(uint8_t power) {
        RESUME();

        static uint8_t RegPaConfig = 0x00;

        if ((power < 2) || (power > 17)) { // no magic numbers
            power = 2;

        }

        RegPaConfig |= RFM_PaSelect;

        RegPaConfig |= (power - 2);

        RetType ret = CALL(writeReg(RFM95_REGISTER_PA_CONFIG, RegPaConfig));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;

    }

    RetType setMode(uint8_t mode) {
        RESUME();

        static uint8_t RegOpMode;

        if (mode > 0b00000111) {
            mode = RFM_MODE_STANDBY; //standby mode
        }

        static uint8_t curr_mode;
        RetType ret = CALL(readReg(RFM95_REGISTER_OP_MODE, &curr_mode, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        curr_mode &= 0b11111000; // clear old mode;
        curr_mode |= mode; //set mode;

        ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, curr_mode));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        while (1) {
            uint8_t mode_check;
            RetType ret = CALL(readReg(RFM95_REGISTER_OP_MODE, &mode_check, 1));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
            if (mode_check == mode) {
                break;
            }
        }

        RESET();
        return RET_SUCCESS;


    }

    RetType receive(uint8_t *buf, uint8_t len) {
        RESUME();
        static uint8_t irq_flags; // make all variables are static
        static bool valid = false;

        RetType ret = CALL(setMode(RFM_MODE_STANDBY));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_PAYLOAD_LENGTH, len));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        do {
            static uint8_t rx_addr;
            ret = CALL(readReg(RFM95_REGISTER_FIFO_RX_BASE_ADDR, &rx_addr, 1));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, rx_addr));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            //set to rx mode
            ret = CALL(setMode(RFM_MODE_RXSINGLE));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            do {
                ret = CALL(readReg(RFM95_REGISTER_IRQ_FLAGS, &irq_flags, 1));
                if (ret != RET_SUCCESS) {
                    RESET();
                    return ret;
                }
            } while (!(irq_flags & (RFM_RxTimeout | RFM_RxDone)));

            static uint8_t rec_len;
            ret = CALL(readReg(RFM95_REGISTER_FIFO_RX_BYTES_NB, &rec_len, 1));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            valid = (rec_len == len) && (irq_flags & RFM_RxDone) && !(irq_flags & RFM_PayloadCrcError);

            ret = CALL(writeReg(RFM95_REGISTER_IRQ_FLAGS, RFM_RxDone | RFM_PayloadCrcError | RFM_RxTimeout));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
        } while (!valid);

        static uint8_t curr_addr;
        ret = CALL(readReg(0x10, &curr_addr, 1)); // 0x10 = curr address register
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, curr_addr));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        //read packet into buffer from FIFO
        ret = CALL(readReg(RFM95_REGISTER_FIFO_RX_BASE_ADDR, buf, len));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;


    }

    RetType transmit(uint8_t *buffer, uint8_t len) {
        RESUME();

        RetType ret = CALL(setMode(RFM_MODE_STANDBY));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_PAYLOAD_LENGTH, len));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        static uint8_t tx_addr;
        ret = CALL(readReg(RFM95_REGISTER_FIFO_TX_BASE_ADDR, &tx_addr, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, tx_addr));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }


        //fill fifo
        ret = CALL(readReg(RFM95_REGISTER_FIFO_ACCESS, buffer, len));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(setMode(RFM_MODE_TX));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        while (1) {
            uint8_t mode_check;
            RetType ret = CALL(readReg(RFM95_REGISTER_OP_MODE, &mode_check, 1));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }
            if ((mode_check & 0b00000111) == RFM_MODE_TX) {
                break;
            }
        }

        ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, RFM_TxDone));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();

        return RET_SUCCESS;

    }

    RetType receiveAsync(uint8_t *buf, uint8_t len) {
        RESUME();

        RetType ret = CALL(setMode(RFM_MODE_STANDBY));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_PAYLOAD_LENGTH, len));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_IRQ_FLAGS, RFM_RxDone | RFM_PayloadCrcError | RFM_RxTimeout));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        static uint8_t curr_addr;
        ret = CALL(readReg(0x10, &curr_addr, 1)); // 0x10 = curr address register
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, curr_addr));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
		
		ret = CALL(setMode(RFM_MODE_RXCONTINUOUS));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        RESET();
        return RET_SUCCESS;

    }

    RetType packetWaiting(int *status) {
        RESUME();

        static uint8_t irq_flags;

        RetType ret = CALL(readReg(RFM95_REGISTER_IRQ_FLAGS, &irq_flags, 1)); // 0x10 = curr address register
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        if (irq_flags & (1 << 6)) {
            *status = 1;
        } else {
            *status = 0;
        }


        RESET();
        return RET_SUCCESS;
    }

    RetType receive_packet_async(uint8_t *buffer, uint8_t len) {
        RESUME();

        static uint8_t rx_len;
        static uint8_t irq_flags;

        RetType ret = CALL(readReg(RFM95_REGISTER_IRQ_FLAGS, &irq_flags, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        ret = CALL(readReg(RFM95_REGISTER_FIFO_RX_BYTES_NB, &rx_len, 1));
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }

        if ((irq_flags & (1 << 6)) && !(irq_flags & (1 << 5)) && (rx_len == len)) { //good receive
            uint8_t curr_addr;
            RetType ret;
//            = CALL(readReg(0x10, &curr_addr, 1)); // 0x10 = curr address register
//            if (ret != RET_SUCCESS) {
//                RESET();
//                return ret;
//            }

            ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, curr_addr));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            ret = CALL(readReg(RFM95_REGISTER_FIFO_RX_BASE_ADDR, buffer, len));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            ret = CALL(writeReg(RFM95_REGISTER_IRQ_FLAGS, (1 << 6) | (1 << 5) | (1 << 7)));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            RESET();
            return RET_SUCCESS;

        } else {
            ret = CALL(writeReg(RFM95_REGISTER_IRQ_FLAGS, (1 << 6) | (1 << 5) | (1 << 7)));
            if (ret != RET_SUCCESS) {
                RESET();
                return ret;
            }

            RESET();
            return RET_SUCCESS; //change??
        }

    }

    RetType getRSSIVal(uint8_t *val) {
        RESUME();

        static uint8_t read;
        RetType ret = CALL(readReg(0x1a, &read, 1)); // 0x10 = curr address register
        if (ret != RET_SUCCESS) {
            RESET();
            return ret;
        }
        static uint16_t rssi = (uint16_t) read * 99 / 155;

        if (rssi > 99) {
            rssi = 99;
        }

        *val = rssi;
        RESET();
        return RET_SUCCESS;


    }

//macro out the register checks
private:
    SPIDevice *mSpi;
    GPIODevice *csPin;
    GPIODevice *nrstPin;

    uint16_t txCount;
    uint16_t rxCount;


    RetType writeReg(RFM95_REGISTER_T reg, uint8_t val) {
        RESUME();

        RetType ret = CALL(this->csPin->set(0));
        if (ret != RET_SUCCESS) return ret;

        uint8_t txBuff[2] = {static_cast<uint8_t>((static_cast<uint8_t>(reg)) | 0x80u), val};
        ret = CALL(this->mSpi->write(txBuff, 2));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(this->csPin->set(1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;


    }

    RetType readReg(uint8_t reg, uint8_t *buff, size_t len) {
        RESUME();

        RetType ret = CALL(this->csPin->set(0));
        if (ret != RET_SUCCESS) return ret;

        static uint8_t txBuff = static_cast<uint8_t>(reg) & 0x7fu;
        ret = CALL(this->mSpi->write(&txBuff, 1));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(this->mSpi->read(buff, len));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(this->csPin->set(1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }


};

#endif //LAUNCH_CORE_RFM95W_H

