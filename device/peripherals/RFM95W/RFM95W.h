/**
 * RFM95W LoRa Driver
 *
 * @author Aaron Chan, Jonathan Russo
 */

#ifndef LAUNCH_CORE_RFM95W_H
#define LAUNCH_CORE_RFM95W_H

#include <stdbool.h>
#include <stdint.h>

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

        SLEEP(1000);
        ret = this->nrstPin->set(0);
        if (ret != RET_SUCCESS) return ret;

        SLEEP(1000);
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
			RetType ret = CALL(writeReg(RFM95_REGISTER_DETECTION_OPTIMIZE, 0xc5));
			if (ret != RET_SUCCESS) return ret;
			RetType ret = CALL(writeReg(RFM95_REGISTER_DETECTION_THRESHOLD, 0x0c));
			if (ret != RET_SUCCESS) return ret;
		}
		else{
			RetType ret = CALL(writeReg(RFM95_REGISTER_DETECTION_OPTIMIZE, 0xc3));
			if (ret != RET_SUCCESS) return ret;
			RetType ret = CALL(writeReg(RFM95_REGISTER_DETECTION_THRESHOLD, 0x0a));
			if (ret != RET_SUCCESS) return ret;
		}

		// writeRegister(REG_MODEM_CONFIG_2, (readRegister(REG_MODEM_CONFIG_2) & 0x0f) | ((sf << 4) & 0xf0));


		RESET();
		return RET_SUCCESS;
	}


    RetType setPower(uint8_t power) {
		RESUME();

		uint8_t RegPaConfig = 0x00;

		if((power < 2) || (power > 17)){
			power = 2;

		}

		RegPaConfig |= (1<<7);

		RegPaConfig |= (power - 2);

		RetType ret = CALL(writeReg(RFM95_REGISTER_PA_CONFIG, RegPaConfig));
		if (ret != RET_SUCCESS) return ret;

		RESET();
		return RET_SUCCESS;

    }

	RetType setMode(uint8_t mode){
		RESUME();

		uint8_t RegOpMode;

		if(mode > 0b00000111){
			mode = 0x01; //standby mode
		}

		uint8_t curr_mode;
		RetType ret = CALL(readReg(RFM95_REGISTER_OP_MODE, &curr_mode, 1));
		if (ret != RET_SUCCESS) return ret;

		curr_mode &= 0b11111000; // clear old mode;
		curr_mode |= mode; //set mode;

		ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, curr_mode));
		if (ret != RET_SUCCESS) return ret;

		while(1){
			uint8_t mode_check;
			RetType ret = CALL(readReg(RFM95_REGISTER_OP_MODE, &mode_check, 1));
			if (ret != RET_SUCCESS) return ret;
			if(mode_check == mode){
				break;
			}
		}

		RESET();
		return RET_SUCCESS;


	}

    RetType receive(uint8_t *buf, uint8_t len){
		RESUME();
		uint8_t irq_flags;
		bool valid = false;


		RetType ret = CALL(setMode(0x01));
		if (ret != RET_SUCCESS) return ret;

		ret = CALL(writeReg(RFM95_REGISTER_PAYLOAD_LENGTH, len));
		if (ret != RET_SUCCESS) return ret;

		do{
			uint8_t rx_addr;
			RetType ret = CALL(readReg(RFM95_REGISTER_FIFO_RX_BASE_ADDR, &rx_addr, 1));
			if (ret != RET_SUCCESS) return ret;

			ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, rx_addr));
			if (ret != RET_SUCCESS) return ret;

			//set to rx mode
			ret = CALL(setMode(0x06));
			if (ret != RET_SUCCESS) return ret

			do {
				ret = CALL(readReg(RFM95_REGISTER_IRQ_FLAGS, &irq_flags, 1));
				if (ret != RET_SUCCESS) return ret;
			} while(!(irq_flags & ((1<<7) | (1<<6))));

			uint8_t rec_len;
			RetType ret = CALL(readReg(RFM95_REGISTER_FIFO_RX_BYTES_NB, &rec_len, 1));
			if (ret != RET_SUCCESS) return ret;

			valid = (rec_len == len) && (irq_flags & (1<<6)) && !(irq_flags & (1<<5));

			ret = CALL(writeReg(RFM95_REGISTER_IRQ_FLAGS, (1<<6) | (1<<5) | (1<<7)));
			if (ret != RET_SUCCESS) return ret;
		} while(!valid);

		uint8_t curr_addr;
		RetType ret = CALL(readReg(0x10, &curr_addr, 1)); // 0x10 = curr address register
		if (ret != RET_SUCCESS) return ret;

		ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, curr_addr));
		if (ret != RET_SUCCESS) return ret;

		//read packet into buffer from FIFO
		// TODO

		RESET();
		return RET_SUCCESS;


    }

    RetType transmit(uint8_t *buffer, uint8_t len){
    	RESUME();

    	RetType ret = CALL(setMode(0x01));
    	if (ret != RET_SUCCESS) return ret;

    	ret = CALL(writeReg(RFM95_REGISTER_PAYLOAD_LENGTH, len));
    	if (ret != RET_SUCCESS) return ret;

    	uint8_t tx_addr;
    	RetType ret = CALL(readReg(RFM95_REGISTER_FIFO_TX_BASE_ADDR, &tx_addr, 1));
    	if (ret != RET_SUCCESS) return ret;

    	ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, tx_addr));
    	if (ret != RET_SUCCESS) return ret;


    	//fill fifo
    	// TODO

    	RetType ret = CALL(setMode(0x03));
    	if (ret != RET_SUCCESS) return ret;

    	while(1){
			uint8_t mode_check;
			RetType ret = CALL(readReg(RFM95_REGISTER_OP_MODE, &mode_check, 1));
			if (ret != RET_SUCCESS) return ret;
			if((mode_check & 0b00000111) == 0x03){
				break;
			}
    	}

    	ret = CALL(writeReg(RFM95_REGISTER_OP_MODE, (1<<3)));
    	if (ret != RET_SUCCESS) return ret;

    	RESET();

    	return RET_SUCCESS;

    }

    RetType receiveAsync(uint8_t *buf, uint8_t len){
    	RESUME();

    	RetType ret = CALL(setMode(0x01));
    	if (ret != RET_SUCCESS) return ret;

    	ret = CALL(writeReg(RFM95_REGISTER_PAYLOAD_LENGTH, len));
    	if (ret != RET_SUCCESS) return ret;

		ret = CALL(writeReg(RFM95_REGISTER_IRQ_FLAGS, (1<<6) | (1<<5) | (1<<7)));
		if (ret != RET_SUCCESS) return ret;

		uint8_t curr_addr;
		ret = CALL(readReg(0x10, &curr_addr, 1)); // 0x10 = curr address register
		if (ret != RET_SUCCESS) return ret;

		ret = CALL(writeReg(RFM95_REGISTER_FIFO_ADDR_PTR, curr_addr));
		if (ret != RET_SUCCESS) return ret;

    	RESET();
    	return RET_SUCCESS;

    }

    RetType packetWaiting(){
    	RESUME();

    	uint8_t irq_flags;

		ret = CALL(readReg(RFM95_REGISTER_IRQ_FLAGS, &irq_flags, 1)); // 0x10 = curr address register
		if (ret != RET_SUCCESS) return ret;

		//TODO

		RESET();
		return RET_SUCCESS;
    }

    // not sure if needed
    RetType bulkRead(uint8_t addr){

    }

    //not sure if needed
    RetType bulkWrite(uint8_t addr, uint8_t *buf, uint8_t len){

    }

private:
    SPIDevice *mSpi;
    GPIODevice *csPin;
    GPIODevice *nrstPin;
    GPIODevice *nssPin;

    uint16_t txCount;
    uint16_t rxCount;


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

