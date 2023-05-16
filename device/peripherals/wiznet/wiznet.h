#ifndef WIZNET_H
#define WIZNET_H
//*****************************************************************************
//
//! \file w5500.c
//! \brief W5500 HAL Interface.
//! \version 1.0.2
//! \date 2013/10/21
//! \par  Revision history
//!       <2015/02/05> Notice
//!        The version history is not updated after this point.
//!        Download the latest version directly from GitHub. Please visit the our GitHub repository for ioLibrary.
//!        >> https://github.com/Wiznet/ioLibrary_Driver
//!       <2014/05/01> V1.0.2
//!         1. Implicit type casting -> Explicit type casting. Refer to M20140501
//!            Fixed the problem on porting into under 32bit MCU
//!            Issued by Mathias ClauBen, wizwiki forum ID Think01 and bobh
//!            Thank for your interesting and serious advices.
//!       <2013/12/20> V1.0.1
//!         1. Remove warning
//!         2. WIZCHIP_READ_BUF WIZCHIP_WRITE_BUF in case _WIZCHIP_IO_MODE_SPI_FDM_
//!            for loop optimized(removed). refer to M20131220
//!       <2013/10/21> 1st Release
//! \author MidnightCow
//! \copyright
//!
//! Copyright (c)  2013, WIZnet Co., LTD.
//! All rights reserved.
//!
//! Redistribution and use in source and binary forms, with or without
//! modification, are permitted provided that the following conditions
//! are met:
//!
//!     * Redistributions of source code must retain the above copyright
//! notice, this list of conditions and the following disclaimer.
//!     * Redistributions in binary form must reproduce the above copyright
//! notice, this list of conditions and the following disclaimer in the
//! documentation and/or other materials provided with the distribution.
//!     * Neither the name of the <ORGANIZATION> nor the names of its
//! contributors may be used to endorse or promote products derived
//! from this software without specific prior written permission.
//!
//! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//! AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//! IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//! ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
//! LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//! CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//! SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//! INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//! CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//! ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
//! THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

#include <stdio.h>
#include "wiznet_defs.h"
#include <stdlib.h>
#include <stdint.h>

#include "device/peripherals/W5500/W5500_defines.h"
#include "device/peripherals/W5500/W5500_socket.h"
#include "device/peripherals/W5500/W5500_defs.h"
#include "device/StreamDevice.h"
#include "net/network_layer/NetworkLayer.h"
#include "net/packet/Packet.h"
#include "sched/macros.h"
#include "return.h"

#define _W5500_SPI_VDM_OP_          0x00
#define _W5500_SPI_FDM_OP_LEN1_     0x01
#define _W5500_SPI_FDM_OP_LEN2_     0x02
#define _W5500_SPI_FDM_OP_LEN4_     0x03

class Wiznet : public NetworkLayer {
public:
    Wiznet(SPIDevice &spi, GPIODevice &gpio) : m_spi(spi), m_gpio(gpio) {};

    void wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
        uint16_t ptr = 0;
        uint32_t addr_sel = 0;

        if (len == 0) return;
        ptr = getSn_TX_WR(sn);
        //M20140501 : implict type casting -> explict type casting
        //addr_sel = (ptr << 8) + (WIZCHIP_TXBUF_BLOCK(sn) << 3);
        addr_sel = ((uint32_t) ptr << 8) + (WIZCHIP_TXBUF_BLOCK(sn) << 3);
        //
        WIZCHIP_WRITE_BUF(addr_sel, wizdata, len);

        ptr += len;
        setSn_TX_WR(sn, ptr);
    }

    void wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
        uint16_t ptr = 0;
        uint32_t addr_sel = 0;

        if (len == 0) return;
        ptr = getSn_RX_RD(sn);
        //M20140501 : implict type casting -> explict type casting
        //addr_sel = ((ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);
        addr_sel = ((uint32_t) ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);
        //
        WIZCHIP_READ_BUF(addr_sel, wizdata, len);
        ptr += len;

        setSn_RX_RD(sn, ptr);
    }


    void wiz_recv_ignore(uint8_t sn, uint16_t len) {
        uint16_t ptr = 0;

        ptr = getSn_RX_RD(sn);
        ptr += len;
        setSn_RX_RD(sn, ptr);
    }

    RetType getSn_RX_RD(uint8_t sn, uint16_t *result) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_RX_RD(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *result = (uint16_t) tmp << 8;

        ret = WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RD(sn), 1), &tmp);
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *result += tmp;

        GET_SN_RX_RD_END:
    RESET();
        return ret;
    }

private:
    // passed in SPI controller
    SPIDevice &m_spi;
    GPIODevice &m_gpio;

    // SPI Transaction Buffers
    // Reduce static memory usage by using a single buffer for all SPI transactions
    uint8_t tx_buffer[16] = {};
    uint8_t rx_buffer[16] = {};

    uint8_t rx_int_flag = 0;
    uint8_t tx_int_flag = 0;

    RetType WIZCHIP_READ(uint32_t addr_sel, uint8_t *read_byte) {
        RESUME();
        static uint8_t spi_data[3];

        RetType ret = CALL(m_gpio.set(0));

        addr_sel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;
        ret = CALL(m_spi.write(spi_data, 3)); // TODO: Might be able to do this in a single call
        if (ret != RET_SUCCESS) goto WIZCHIP_READ_END;

        ret = m_spi.read(read_byte, 1);

        WIZCHIP_READ_END:
        CALL(m_gpio.set(1));
        RESET();
        return ret;
    }

    RetType WIZCHIP_WRITE(uint32_t addr_sel, uint8_t wb) {
        RESUME();
        static uint8_t spi_data[4];

        RetType ret = CALL(m_gpio.set(0));

        addr_sel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;
        spi_data[3] = wb;

        ret = CALL(m_spi.write(spi_data, 4));

        RESET();
        CALL(m_gpio.set(1));
        return ret;
    }

    RetType WIZCHIP_READ_BUF(uint32_t addr_sel, uint8_t *buff, size_t len) {
        RESUME();

        static uint8_t spi_data[3];

        RetType ret = CALL(m_gpio.set(0));

        addr_sel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;

        ret = CALL(m_spi.write(spi_data, 3));
        if (ret != RET_SUCCESS) goto WIZCHIP_READ_BUF_END;

        ret = CALL(m_spi.read(buff, len));

        WIZCHIP_READ_BUF_END:
        CALL(m_gpio.set(1));
        RESET();
        return ret;
    }

    RetType WIZCHIP_WRITE_BUF(uint32_t addr_sel, uint8_t *buff, size_t len) {
        RESUME();
        static uint8_t spi_data[3];

        RetType ret = CALL(m_gpio.set(0));

        addr_sel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;

        ret = CALL(m_spi.write(spi_data, 3));
        if (ret != RET_SUCCESS) goto WIZCHIP_WRITE_BUF_END;

        ret = CALL(m_spi.write(buff, len));

        WIZCHIP_WRITE_BUF_END:
        CALL(m_gpio.set(1));
        RESET();
        return ret;
    }


    uint16_t getSn_TX_FSR(uint8_t sn, uint8_t *result) {
        RESUME();

        static uint16_t val = 0;
        static uint16_t val1 = 0;
        static uint16_t tmp = 0;
        val = 0;
        val1 = 0;
        tmp = 0;

        RetType ret;
        do {
            ret = CALL(WIZCHIP_READ(Sn_TX_FSR(sn), (uint8_t *) &val1));
            if (ret != RET_SUCCESS) goto GET_SN_TX_FSR_END;

            ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn), 1), (uint8_t *) &tmp));
            if (ret != RET_SUCCESS) goto GET_SN_TX_FSR_END;

            val1 = (val1 << 8) + tmp;

            if (val1 != 0) {
                ret = CALL(WIZCHIP_READ(Sn_TX_FSR(sn), (uint8_t *) &val));
                if (ret != RET_SUCCESS) goto GET_SN_TX_FSR_END;

                ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_FSR(sn), 1), (uint8_t *) &tmp));
                if (ret != RET_SUCCESS) goto GET_SN_TX_FSR_END;
            }
        } while (val != val1);

        *result = val;

        GET_SN_TX_FSR_END:
    RESET();
        return ret;
    }


    uint16_t getSn_RX_RSR(uint8_t sn, uint16_t *result) {
        RESUME();
        static uint16_t val = 0;
        static uint16_t val1 = 0;
        static uint16_t tmp = 0;

        RetType ret;
        do {
            ret = CALL(WIZCHIP_READ(Sn_RX_RSR(sn), (uint8_t *) &val1));
            if (ret != RET_SUCCESS) goto GET_SN_RX_RSR_END;

            ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn), 1), (uint8_t *) &tmp));
            if (ret != RET_SUCCESS) goto GET_SN_RX_RSR_END;

            val1 = (val1 << 8) + tmp;

            if (val1 != 0) {
                ret = CALL(WIZCHIP_READ(Sn_RX_RSR(sn), (uint8_t *) &val));
                if (ret != RET_SUCCESS) goto GET_SN_RX_RSR_END;

                ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn), 1), (uint8_t *) &tmp));
                if (ret != RET_SUCCESS) goto GET_SN_RX_RSR_END;

                val = (val << 8) + tmp;
            }
        } while (val != val1);

        *result = val;
        GET_SN_RX_RSR_END:
    RESET();
        return ret;
    }

/////////////////////////////////
// Common Register I/O function //
/////////////////////////////////

    /**
    * @ingroup Common_register_access_function
    * @brief Set Mode Register
    * @param (uint8_t)mr The value to be set.
    * @sa getMR()
    */
    RetType setMR(uint8_t mode_reg) {
        RESUME();
        RetType ret = CALL(WIZCHIP_WRITE(MR, mode_reg));
        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Get Mode Register
    * @return uint8_t. The value of Mode register.
    * @sa setMR()
    */
    RetType setMR(uint8_t *mode_reg) {
        RESUME();
        RetType ret = CALL(WIZCHIP_READ(MR, mode_reg));
        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Set gateway IP address
    * @param (uint8_t*)gar Pointer variable to set gateway IP address. It should be allocated 4 bytes.
    * @sa getGAR()
    */
    RetType setGAR(uint8_t *gar) {
        RESUME();
        RetType ret = CALL(WIZCHIP_WRITE_BUF(GAR, gar, 4));
        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Get gateway IP address
     * @param (uint8_t*)gar Pointer variable to get gateway IP address. It should be allocated 4 bytes.
     * @sa setGAR()
     */
    RetType getGAR(uint8_t *gar) {
        RESUME();
        RetType ret = CALL(WIZCHIP_READ_BUF(GAR, gar, 4));
        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Set subnet mask address
    * @param (uint8_t*)subr Pointer variable to set subnet mask address. It should be allocated 4 bytes.
    * @sa getSUBR()
    */
    RetType setSUBR(uint8_t *subr) {
        RESUME();
        RetType ret = CALL(WIZCHIP_WRITE_BUF(SUBR, subr, 4));
        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Get subnet mask address
    * @param (uint8_t*)subr Pointer variable to get subnet mask address. It should be allocated 4 bytes.
    * @sa setSUBR()
    */
    RetType getSUBR(uint8_t *subr) {
        RESUME();
        RetType ret = CALL(WIZCHIP_READ_BUF(SUBR, subr, 4));
        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Set local MAC address
    * @param (uint8_t*)shar Pointer variable to set local MAC address. It should be allocated 6 bytes.
    * @sa getSHAR()
    */
    RetType setSHAR(uint8_t *shar) {
        RESUME();
        RetType ret = CALL(WIZCHIP_WRITE_BUF(SHAR, shar, 6));
        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Get local MAC address
    * @param (uint8_t*)shar Pointer variable to get local MAC address. It should be allocated 6 bytes.
    * @sa setSHAR()
    */
    RetType getSHAR(uint8_t *shar) {
        RESUME();
        RetType ret = CALL(WIZCHIP_READ_BUF(SHAR, shar, 6));
        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Set local IP address
    * @param (uint8_t*)sipr Pointer variable to set local IP address. It should be allocated 4 bytes.
    * @sa getSIPR()
    */
    RetType setSIPR(uint8_t *sipr) {
        RESUME();
        RetType ret = CALL(WIZCHIP_WRITE_BUF(SIPR, sipr, 4));
        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Get local IP address
    * @param (uint8_t*)sipr Pointer variable to get local IP address. It should be allocated 4 bytes.
    * @sa setSIPR()
    */
    RetType getSIPR(uint8_t *sipr) {
        RESUME();
        RetType ret = CALL(WIZCHIP_READ_BUF(SIPR, sipr, 4));
        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Set INTLEVEL register
    * @param (uint16_t)intlevel Value to set @ref INTLEVEL register.
    * @sa getINTLEVEL()
    */
    RetType setINTLEVEL(uint16_t intlevel) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(INTLEVEL, (uint8_t) (intlevel >> 8)));
        if (ret != RET_SUCCESS) goto SET_INT_LEVEL_END;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(INTLEVEL, 1), (uint8_t) intlevel));

        SET_INT_LEVEL_END:
    RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Get INTLEVEL register
    * @return uint16_t. Value of @ref INTLEVEL register.
    * @sa setINTLEVEL()
    */
    RetType getINTLEVEL(uint16_t *intlevel) {
        RESUME();
        static uint8_t tmp;
        RetType ret = CALL(WIZCHIP_READ(INTLEVEL, &tmp));
        if (ret != RET_SUCCESS) goto GET_INT_LEVEL_END;

        *intlevel = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(INTLEVEL, 1), &tmp));
        *intlevel += tmp;

        GET_INT_LEVEL_END:
    RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Set @ref IR register
    * @param (uint8_t)ir Value to set @ref IR register.
    * @sa getIR()
    */
    RetType setIR(uint8_t ir) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(IR, (ir & 0xF0)));

        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Get @ref IR register
    * @return uint8_t. Value of @ref IR register.
    * @sa setIR()
    */
    RetType getIR(uint8_t *ir) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(IR, ir));
        *ir &= 0xF0;
        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Set @ref _IMR_ register
    * @param (uint8_t)imr Value to set @ref _IMR_ register.
    * @sa getIMR()
    */
    RetType setIMR(uint8_t imr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(_IMR_, imr));

        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Get @ref _IMR_ register
    * @return uint8_t. Value of @ref _IMR_ register.
    * @sa setIMR()
    */
    RetType getIMR(uint8_t *imr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(_IMR_, imr));

        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Set @ref SIR register
    * @param (uint8_t)sir Value to set @ref SIR register.
    * @sa getSIR()
    */
    RetType setSIR(uint8_t sir) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(SIR, sir));

        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Get @ref SIR register
    * @return uint8_t. Value of @ref SIR register.
    * @sa setSIR()
    */
    RetType getSIR(uint8_t *sir) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(SIR, sir));

        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Set @ref SIMR register
    * @param (uint8_t)simr Value to set @ref SIMR register.
    * @sa getSIMR()
    */
    RetType setSIMR(uint8_t simr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(SIMR, simr));

        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Get @ref SIMR register
    * @return uint8_t. Value of @ref SIMR register.
    * @sa setSIMR()
    */
    RetType getSIMR(uint8_t *simr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(SIMR, simr));

        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Set @ref _RTR_ register
    * @param (uint16_t)rtr Value to set @ref _RTR_ register.
    * @sa getRTR()
    */
    RetType setRTR(uint16_t rtr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(_RTR_, (uint8_t) (rtr >> 8)));
        if (ret != RET_SUCCESS) goto SET_RTR_END;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(_RTR_, 1), (uint8_t) rtr));

        SET_RTR_END:
    RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Get @ref _RTR_ register
    * @return uint16_t. Value of @ref _RTR_ register.
    * @sa setRTR()
    */
    RetType getRTR(uint16_t *rtr) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(_RTR_, &tmp));
        if (ret != RET_SUCCESS) goto GET_RTR_END;

        *rtr = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(_RTR_, 1), &tmp));
              * rtr += tmp;


        GET_RTR_END;
        RESET();
        return ret;
    }

    /**
    * @ingroup Common_register_access_function
    * @brief Set @ref _RCR_ register
    * @param (uint8_t)rcr Value to set @ref _RCR_ register.
    * @sa getRCR()
    */
    RetType setRCR(uint8_t rcr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(_RCR_, rcr));

        RESET();
        return ret;
    }


    /**
    * @ingroup Common_register_access_function
    * @brief Get @ref _RCR_ register
    * @return uint8_t. Value of @ref _RCR_ register.
    * @sa setRCR()
    */
    RetType getRCR(uint8_t *rcr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(_RCR_, rcr));

        RESET();
        return ret;
    }

};

#endif //WIZNET_H
