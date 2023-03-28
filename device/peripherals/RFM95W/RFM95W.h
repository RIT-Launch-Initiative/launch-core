/**
 * RFM95W LoRa Driver
 *
 * @author Aaron Chan, Jonathan Russo
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

#define RFM9x_VER 0x12



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
	RFM95_REGISTER_DETECTION_OPTIMIZE = 0x31;
    RFM95_REGISTER_INVERT_IQ_1 = 0x33,
	RFM95_REGISTER_DETECTION_THRESHOLD = 0x37;
    RFM95_REGISTER_SYNC_WORD = 0x39,
    RFM95_REGISTER_INVERT_IQ_2 = 0x3B,
    RFM95_REGISTER_DIO_MAPPING_1 = 0x40,
    RFM95_REGISTER_VERSION = 0x42,
    RFM95_REGISTER_PA_DAC = 0x4D
};





class RFM95W {
public:
    RFM95W(SPIDevice *spiDevice, GPIODevice *csPin) : StreamDevice("RFM95W"),
                                                      mSpi(spiDevice),
                                                      csPin(csPin) {}

    RetType init() {
        RESUME();

		// add potential asserts here

        RetType ret = CALL(reset());
        if (ret != RET_SUCCESS) return ret;

		uint8_t version;
		RetType ret = CALL(readReg(RFM95_REGISTER_VERSION, &version, 1));
		if (ret != RET_SUCCESS) return ret;

		if(version != RFM9x_VER) return RET_ERROR;

		//place module in sleep mode
		ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, 0x00));
		if (ret != RET_SUCCESS) return ret;
		ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, 0x80));
		if (ret != RET_SUCCESS) return ret;

		//set to default interrupt config
		ret = CALL(writeReg(RFM95_REGISTER_DIO_MAPPING_1, 0x00));
		if (ret != RET_SUCCESS) return ret;

		//config interrupts

		//set power
		ret = setPower(17);
		if (ret != RET_SUCCESS) return ret;

		//set preamble to 8 + 4.25 = 12.25 symbols.
		ret = CALL(writeReg(RFM95_REGISTER_PREAMBLE_MSB, 0x00));
		if (ret != RET_SUCCESS) return ret;
		ret = CALL(writeReg(RFM95_REGISTER_PREAMBLE_LSB, 0x08));
		if (ret != RET_SUCCESS) return ret;

		//set ttn sync word 0x34
		ret = CALL(writeReg(RFM95_REGISTER_SYNC_WORD, 0x34));
		if (ret != RET_SUCCESS) return ret;

		//set up base addresses for rx and tx
		ret = CALL(writeReg(RFM95_REGISTER_FIFO_TX_BASE_ADDR, 0x80));
		if (ret != RET_SUCCESS) return ret;
		ret = CALL(writeReg(RFM95_REGISTER_FIFO_RX_BASE_ADDR, 0x00));
		if (ret != RET_SUCCESS) return ret;

		// Set Maximum payload length to 64
		ret = CALL(writeReg(RFM95_REGISTER_MAX_PAYLOAD_LENGTH, 64));
		if (ret != RET_SUCCESS) return ret;

		// Let module sleep after init
		ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, 0x80));
		if (ret != RET_SUCCESS) return ret;


        RESET();
        return RET_SUCCESS;
    }

    RetType write(RFM95_REGISTER_T reg, uint8_t *buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType read(uint8_t *buff, size_t len) {
        RESUME();

        RESET();
        return RET_SUCCESS;
    }

    RetType configFrequency(uint32_t frequency) {
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


	RetType isTransmitting(){
		uint8_t transmit;
		if(readReg(RFM95_REGISTER_OP_MODE, &transmit, 1) != RET_SUCCESS
	}

	RetType setSpreadingFactor(int sf){
		RESUME();

		if(sf < 6){
			sf = 6;
		}

		else if(sf > 12){
			sf = 12;
		}

		if(sf == 6){
			RetType ret = CALL(writeReg(RFM95W_REGISTER_DETECTION_OPTIMIZE, 0xc5));
			if (ret != RET_SUCCESS) return ret;
			RetType ret = CALL(writeReg(RFM95W_REGISTER_DETECTION_THRESHOLD, 0x0c));
			if (ret != RET_SUCCESS) return ret;
		}
		else{
			RetType ret = CALL(writeReg(RFM95W_REGISTER_DETECTION_OPTIMIZE, 0xc3));
			if (ret != RET_SUCCESS) return ret;
			RetType ret = CALL(writeReg(RFM95W_REGISTER_DETECTION_THRESHOLD, 0x0a));
			if (ret != RET_SUCCESS) return ret;
		}

		// writeRegister(REG_MODEM_CONFIG_2, (readRegister(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));


		RESET();
		return RET_SUCCESS;
	}


    RetType setPower(int8_t power) {
		RESUME();

		assert((power >= 2 && power <= 17) || power == 20);

		rfm95_register_pa_config_t pa_config = {0};

		if (power >= 2 && power <= 17) {
			pa_config.max_power = 7;
			pa_config.pa_select = 1;
			pa_config.output_power = (power - 2);
			pa_dac_config = RFM95_REGISTER_PA_DAC_LOW_POWER;

		} else if (power == 20) {
			pa_config.max_power = 7;
			pa_config.pa_select = 1;
			pa_config.output_power = 15;
			pa_dac_config = RFM95_REGISTER_PA_DAC_HIGH_POWER;
		}

		ret = CALL(writeReg(RFM95_REGISTER_PA_CONFIG, pa_config.buffer));
		if (ret != RET_SUCCESS) return ret;
		ret = CALL(writeReg(RFM95_REGISTER_PA_DAC, pa_dac_config));
		if (ret != RET_SUCCESS) return ret;

		RESET();
		return RET_SUCCESS;

    }

    RetType handleInterrupt(){

    }

    RetType validateRxBuf(){

    }

    RetType available() {

    }

    RetType clearRxBuf(){

    }

    RetType receive(){

    }

    RetType send(){

    }

    RetType setTx(){

    }

    RetType setRx(){

    }

    RetType setModemReg(){

    }

    RetType setModemCfg(){

    }


private:
    SPIDevice *mSpi;
    GPIODevice *csPin;
    GPIODevice *nrstPin;
    GPIODevice *nssPin;

    uint16_t txCount;
    uint16_t rxCount;

	typedef struct
{
	union {
		struct {
			uint8_t output_power : 4;
			uint8_t max_power : 3;
			uint8_t pa_select : 1;
		};
		uint8_t buffer;
	};
} rfm95_register_pa_config_t;

    RetType writeReg(RFM95_REGISTER_T reg, uint8_t val) {
        RESUME();

        RetType ret = CALL(this->nssPin->set(0));
        if (ret != RET_SUCCESS) return ret;

        uint8_t txBuff[2] = {static_cast<uint8_t>((static_cast<uint8_t>(reg)) | 0x80u), val};
        ret = this->mSpi->write(txBuff, 2);
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(this->nssPin->set(1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;


    }

    RetType readReg(RFM95_REGISTER_T reg, uint8_t *buff, size_t len) {
        RESUME();

        RetType ret = CALL(this->nssPin->set(0));
        if (ret != RET_SUCCESS) return ret;

        uint8_t txBuff = static_cast<uint8_t>(reg) & 0x7fu;
        ret = this->mSpi->write(&txBuff, 1);
        if (ret != RET_SUCCESS) return ret;

        ret = this->mSpi->read(buff, len);
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(this->nssPin->set(1));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return RET_SUCCESS;
    }




};

#endif //LAUNCH_CORE_RFM95W_H

