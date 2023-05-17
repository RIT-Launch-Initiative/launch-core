#ifndef WIZNET_H
#define WIZNET_H

/**
 * Wiznet W5500 Driver
 *
 * @author Aaron Chan
 * @author Wiznet (see below copyright)
 */

#include <cstdio>
#include "wiznet_defs.h"
#include <cstdlib>
#include <cstdint>

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

#define DEFAULT_SOCKET_NUM          0   // Hardcoded 0 since all packets will be tx/rx through sock 0

class Wiznet : public NetworkLayer {
public:
    Wiznet(SPIDevice &spi, GPIODevice &cs_pin, GPIODevice &reset_pin) : m_spi(spi), m_cs(cs_pin), m_reset(reset_pin) {};

    RetType init(uint8_t *mac_addr) {
        RESUME();

        static uint8_t tmp;

        RetType ret = CALL(hw_reset()); // Should always return success
        ret = CALL(getVERSIONR(&tmp)); // Make sure we can read the Wiznet
        if (tmp != 4) {
            ret = RET_ERROR;
            goto init_end;
        }

        ret = CALL(setSHAR(mac_addr));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(setSn_RXBUF_SIZE(DEFAULT_SOCKET_NUM, 16));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(setSn_TXBUF_SIZE(DEFAULT_SOCKET_NUM, 16));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(setSn_MR(DEFAULT_SOCKET_NUM, Sn_MR_MACRAW));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(setSn_CR(DEFAULT_SOCKET_NUM, Sn_CR_OPEN));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(getSn_CR(DEFAULT_SOCKET_NUM, &tmp));
        if (tmp != SOCK_MACRAW) ret = RET_ERROR;

        init_end:
        RESET();
        return ret;
    }

    RetType receive(Packet&, netinfo_t&, NetworkLayer*) {
        return RET_ERROR;
    }

    RetType transmit(Packet&, netinfo_t&, NetworkLayer*) {
        return RET_SUCCESS;
    }

    RetType transmit2(Packet& packet, netinfo_t& info, NetworkLayer* caller) {
        RESUME();

        static uint8_t tmp;
        static uint16_t free_size;

        RetType ret = CALL(setSn_DIPR(DEFAULT_SOCKET_NUM, reinterpret_cast<uint8_t *>(info.dst.ipv4_addr)));
        if (ret != RET_SUCCESS) goto transmit2_end;

        ret = CALL(setSn_DPORT(DEFAULT_SOCKET_NUM, info.dst.udp_port));
        if (ret != RET_SUCCESS) goto transmit2_end;

        // TODO: Packet sizing checks?

        ret = CALL(wiz_send_data(DEFAULT_SOCKET_NUM, packet.read_ptr<uint8_t>(), packet.size()));
        if (ret != RET_SUCCESS) goto transmit2_end;

        ret = CALL(setSn_CR(DEFAULT_SOCKET_NUM, Sn_CR_SEND));
        if (ret != RET_SUCCESS) goto transmit2_end;

        while (true) {
            ret = CALL(getSn_IR(DEFAULT_SOCKET_NUM, &tmp));
            if (ret != RET_SUCCESS) goto transmit2_end;

            if (tmp & Sn_IR_SENDOK) {
                ret = CALL(setSn_IR(DEFAULT_SOCKET_NUM, Sn_IR_SENDOK));
                break;
            } else if (tmp & Sn_IR_TIMEOUT) {
                ret = CALL(setSn_IR(DEFAULT_SOCKET_NUM, Sn_IR_TIMEOUT));

                RESET();
                return RET_ERROR;
            }
        }

        transmit2_end:
        RESET();
        return ret;
    }

    RetType wiz_send_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
        RESUME();
        static uint16_t ptr = 0;
        static uint32_t addr_sel = 0;

        RetType ret;
        if (len == 0) {
            ret = RET_ERROR;
            goto SEND_DATA_END;
        }

        ret = CALL(getSn_TX_WR(sn, &ptr));
        if (ret != RET_SUCCESS) goto SEND_DATA_END;

        addr_sel = ((uint32_t) ptr << 8) + (WIZCHIP_TXBUF_BLOCK(sn) << 3);
        ret = CALL(WIZCHIP_WRITE_BUF(addr_sel, wizdata, len));
        if (ret != RET_SUCCESS) goto SEND_DATA_END;

        ptr += len;
        ret = CALL(setSn_TX_WR(sn, ptr));

        SEND_DATA_END:
        RESET();
        return ret;
    }

    RetType wiz_recv_data(uint8_t sn, uint8_t *wizdata, uint16_t len) {
        RESUME();
        static uint16_t ptr = 0;
        static uint32_t addr_sel = 0;

        RetType ret;
        if (len == 0) {
            ret = RET_ERROR;
            goto RECV_DATA_END;
        }
        ret = CALL(getSn_RX_RD(sn, &ptr));
        if (ret != RET_SUCCESS) goto RECV_DATA_END;
        addr_sel = ((uint32_t) ptr << 8) + (WIZCHIP_RXBUF_BLOCK(sn) << 3);

        ret = CALL(WIZCHIP_READ_BUF(addr_sel, wizdata, len));
        if (ret != RET_SUCCESS) goto RECV_DATA_END;

        ptr += len;

        ret = CALL(setSn_RX_RD(sn, ptr));
        if (ret != RET_SUCCESS) goto RECV_DATA_END;

        RECV_DATA_END:
        RESET();
        return ret;
    }


    RetType wiz_recv_ignore(uint8_t sn, uint16_t len) {
        RESUME();

        static uint16_t ptr;

        RetType ret = CALL(getSn_RX_RD(sn, &ptr));
        if (ret != RET_SUCCESS) goto RECV_IGNORE_END;

        ptr += len;
        ret = CALL(setSn_RX_RD(sn, ptr));

        RECV_IGNORE_END:
        RESET();
        return ret;
    }

    RetType hw_reset() {
        RESUME();

        CALL(m_reset.set(0));
        SLEEP(50);
        CALL(m_reset.set(1));

        RESET();
        return RET_SUCCESS;
    }

private:
    // passed in SPI controller
    SPIDevice &m_spi;
    GPIODevice &m_cs;
    GPIODevice &m_reset;

    // SPI Transaction Buffers
    // Reduce static memory usage by using a single buffer for all SPI transactions
    uint8_t tx_buffer[16] = {};
    uint8_t rx_buffer[16] = {};

    uint8_t rx_int_flag = 0;
    uint8_t tx_int_flag = 0;

    RetType WIZCHIP_READ(uint32_t addr_sel, uint8_t *read_byte) {
        RESUME();
        static uint8_t spi_data[3];

        RetType ret = CALL(m_cs.set(0));

        addr_sel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;
        ret = CALL(m_spi.write(spi_data, 3)); // TODO: Might be able to do this in a single call
        if (ret != RET_SUCCESS) goto WIZCHIP_READ_END;

        ret = m_spi.read(read_byte, 1);

        WIZCHIP_READ_END:
        CALL(m_cs.set(1));
        RESET();
        return ret;
    }

    RetType WIZCHIP_WRITE(uint32_t addr_sel, uint8_t wb) {
        RESUME();
        static uint8_t spi_data[4];

        RetType ret = CALL(m_cs.set(0));

        addr_sel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;
        spi_data[3] = wb;

        ret = CALL(m_spi.write(spi_data, 4));

        RESET();
        CALL(m_cs.set(1));
        return ret;
    }

    RetType WIZCHIP_READ_BUF(uint32_t addr_sel, uint8_t *buff, size_t len) {
        RESUME();

        static uint8_t spi_data[3];

        RetType ret = CALL(m_cs.set(0));

        addr_sel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;

        ret = CALL(m_spi.write(spi_data, 3));
        if (ret != RET_SUCCESS) goto WIZCHIP_READ_BUF_END;

        ret = CALL(m_spi.read(buff, len));

        WIZCHIP_READ_BUF_END:
        CALL(m_cs.set(1));
        RESET();
        return ret;
    }

    RetType WIZCHIP_WRITE_BUF(uint32_t addr_sel, uint8_t *buff, size_t len) {
        RESUME();
        static uint8_t spi_data[3];

        RetType ret = CALL(m_cs.set(0));

        addr_sel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

        spi_data[0] = (addr_sel & 0x00FF0000) >> 16;
        spi_data[1] = (addr_sel & 0x0000FF00) >> 8;
        spi_data[2] = (addr_sel & 0x000000FF) >> 0;

        ret = CALL(m_spi.write(spi_data, 3));
        if (ret != RET_SUCCESS) goto WIZCHIP_WRITE_BUF_END;

        ret = CALL(m_spi.write(buff, len));

        WIZCHIP_WRITE_BUF_END:
        CALL(m_cs.set(1));
        RESET();
        return ret;
    }


    uint16_t getSn_TX_FSR(uint8_t sn, uint16_t *result) {
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

            if (val1 == 0) continue;
            ret = CALL(WIZCHIP_READ(Sn_RX_RSR(sn), (uint8_t *) &val));
            if (ret != RET_SUCCESS) goto GET_SN_RX_RSR_END;

            ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RSR(sn), 1), (uint8_t *) &tmp));
            if (ret != RET_SUCCESS) goto GET_SN_RX_RSR_END;

            val = (val << 8) + tmp;
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
        *rtr += tmp;

        GET_RTR_END:
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

///////////////////////////////////
//    Configuration Functions   //
/////////////////////////////////

    /**
    * @ingroup Common_register_access_function
    * @brief Set @ref PTIMER register
    * @param (uint8_t)ptimer Value to set @ref PTIMER register.
    * @sa getPTIMER()
    */
    RetType setPTIMER(uint8_t ptimer) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(PTIMER, ptimer));

        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref PTIMER register
     * @return uint8_t. Value of @ref PTIMER register.
     * @sa setPTIMER()
     */
    RetType getPTIMER(uint8_t *ptimer) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(PTIMER, ptimer));

        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Set @ref PMAGIC register
     * @param (uint8_t)pmagic Value to set @ref PMAGIC register.
     * @sa getPMAGIC()
     */
    RetType setPMAGIC(uint8_t pmagic) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(PMAGIC, pmagic));

        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref PMAGIC register
     * @return uint8_t. Value of @ref PMAGIC register.
     * @sa setPMAGIC()
     */
    RetType getPMAGIC(uint8_t *pmagic) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(PMAGIC, pmagic));

        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Set @ref PHAR address
     * @param (uint8_t*)phar Pointer variable to set PPP destination MAC register address. It should be allocated 6 bytes.
     * @sa getPHAR()
     */
    RetType setPHAR(uint8_t *phar) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE_BUF(PHAR, phar, 6));

        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref PHAR address
     * @param (uint8_t*)phar Pointer variable to PPP destination MAC register address. It should be allocated 6 bytes.
     * @sa setPHAR()
     */
    RetType getPHAR(uint8_t *phar) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ_BUF(PHAR, phar, 6));

        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Set @ref PSID register
     * @param (uint16_t)psid Value to set @ref PSID register.
     * @sa getPSID()
     */
    RetType setPSID(uint16_t psid)  {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(PSID, (uint8_t) (psid >> 8)));
        if (ret != RET_SUCCESS) goto SET_PSID_END;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(PSID, 1), (uint8_t) psid));

        SET_PSID_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref PSID register
     * @return uint16_t. Value of @ref PSID register.
     * @sa setPSID()
     */
    RetType getPSID(uint16_t psid) {
        RESUME();

        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(PSID, &tmp));
        if (ret != RET_SUCCESS) goto GET_PSID_END;
        psid = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(PSID, 1), &tmp));
        psid += tmp;

        GET_PSID_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Common_register_access_function
     * @brief Set @ref PMRU register
     * @param (uint16_t)pmru Value to set @ref PMRU register.
     * @sa getPMRU()
     */
    RetType setPMRU(uint16_t pmru) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(PMRU, (uint8_t) (pmru >> 8)));
        if (ret != RET_SUCCESS) goto SET_PMRU_END;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(PMRU, 1), (uint8_t) pmru));

        SET_PMRU_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref PMRU register
     * @return uint16_t. Value of @ref PMRU register.
     * @sa setPMRU()
     */
    RetType getPMRU(uint16_t *pmru) {
        RESUME();

        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(PMRU, &tmp));
        *pmru = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(PMRU, 1), &tmp));
        *pmru += tmp;

        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Get unreachable IP address
     * @param (uint8_t*)uipr Pointer variable to get unreachable IP address. It should be allocated 4 bytes.
     */

    RetType getUIPR(uint8_t *uipr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ_BUF(UIPR,uipr,4));

        RESET();
        return ret;
    }



    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref UPORTR register
     * @return uint16_t. Value of @ref UPORTR register.
     */

    RetType getUPORTR(uint16_t *uportr) {
        RESUME();

        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(UPORTR, &tmp));
        if (ret != RET_SUCCESS) goto GET_UPORTR_END;
        *uportr = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(UPORTR, 1), &tmp));
        *uportr += tmp;

        GET_UPORTR_END:
        RESET();
        return ret;
    }


    /**
     * @ingroup Common_register_access_function
     * @brief Set @ref PHYCFGR register
     * @param (uint8_t)phycfgr Value to set @ref PHYCFGR register.
     * @sa getPHYCFGR()
     */
    RetType setPHYCFGR(uint8_t phycfgr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(PHYCFGR, phycfgr));

        RESET();
        return ret;
    }

    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref PHYCFGR register
     * @return uint8_t. Value of @ref PHYCFGR register.
     * @sa setPHYCFGR()
     */
    RetType getPHYCFGR(uint8_t *phycfgr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(PHYCFGR, phycfgr));

        RESET();
        return ret;
    }

    /**
     * @ingroup Common_register_access_function
     * @brief Get @ref VERSIONR register
     * @return uint8_t. Value of @ref VERSIONR register.
     */
    RetType getVERSIONR(uint8_t *version) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(VERSIONR, version));

        RESET();
        return ret;
    }

///////////////////////////////////
// Socket N register I/O function //
///////////////////////////////////
    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_MR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)mr Value to set @ref Sn_MR
     * @sa getSn_MR()
     */
    RetType setSn_MR(uint8_t sn, uint8_t mr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_MR(sn),mr));

        RESET();
        return ret;
    }
    
    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_MR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_MR.
     * @sa setSn_MR()
     */
    RetType getSn_MR(uint8_t sn, uint8_t *mr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_MR(sn), mr));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_CR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)cr Value to set @ref Sn_CR
     * @sa getSn_CR()
     */
    RetType setSn_CR(uint8_t sn, uint8_t cr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_CR(sn), cr));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_CR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_CR.
     * @sa setSn_CR()
     */
    RetType getSn_CR(uint8_t sn, uint8_t *cr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_CR(sn), cr));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_IR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)ir Value to set @ref Sn_IR
     * @sa getSn_IR()
     */
    RetType setSn_IR(uint8_t sn, uint8_t ir) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_IR(sn), (ir & 0x1F)));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_IR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_IR.
     * @sa setSn_IR()
     */
    RetType getSn_IR(uint8_t sn, uint8_t *ir) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_IR(sn), ir));
        *ir &= 0x1F;

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_IMR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)imr Value to set @ref Sn_IMR
     * @sa getSn_IMR()
     */
    RetType setSn_IMR(uint8_t sn, uint8_t imr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_IMR(sn), (imr & 0x1F)));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_IMR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_IMR.
     * @sa setSn_IMR()
     */
    RetType getSn_IMR(uint8_t sn, uint8_t *imr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_IMR(sn), imr));
        *imr &= 0x1F;

        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_SR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_SR.
     */
    RetType getSn_SR(uint8_t sn, uint8_t *mr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_SR(sn), mr));

        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_PORT register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint16_t)port Value to set @ref Sn_PORT.
     * @sa getSn_PORT()
     */
    RetType setSn_PORT(uint8_t sn, uint8_t port) {
        RESUME();
        RetType ret = CALL(WIZCHIP_WRITE(Sn_PORT(sn), (uint8_t) (port >> 8)));
        if (ret != RET_SUCCESS) goto SET_SN_PORT_END;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_PORT(sn), 1), (uint8_t) port)); \

        SET_SN_PORT_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_PORT register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_PORT.
     * @sa setSn_PORT()
     */
    RetType getSn_PORT(uint8_t sn, uint16_t *sn_port) {
        RESUME();

        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_PORT(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_PORT_END;
        *sn_port = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_PORT(sn),1), &tmp));
        *sn_port += tmp;

        GET_SN_PORT_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_DHAR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t*)dhar Pointer variable to set socket n destination hardware address. It should be allocated 6 bytes.
     * @sa getSn_DHAR()
     */
    RetType setSn_DHAR(uint8_t sn, uint8_t *dhar) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE_BUF(Sn_DHAR(sn), dhar, 6));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_MR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t*)dhar Pointer variable to get socket n destination hardware address. It should be allocated 6 bytes.
     * @sa setSn_DHAR()
     */
    RetType getSn_DHAR(uint8_t sn, uint8_t *dhar) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ_BUF(Sn_DHAR(sn), dhar, 6));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_DIPR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t*)dipr Pointer variable to set socket n destination IP address. It should be allocated 4 bytes.
     * @sa getSn_DIPR()
     */
    RetType setSn_DIPR(uint8_t sn, uint8_t *dipr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE_BUF(Sn_DIPR(sn), dipr, 4));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_DIPR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t*)dipr Pointer variable to get socket n destination IP address. It should be allocated 4 bytes.
     * @sa setSn_DIPR()
     */
    RetType getSn_DIPR(uint8_t sn, uint8_t *dipr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ_BUF(Sn_DIPR(sn), dipr, 4));

        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_DPORT register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint16_t)dport Value to set @ref Sn_DPORT
     * @sa getSn_DPORT()
     */
    RetType setSn_DPORT(uint8_t sn, uint16_t dport) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_DPORT(sn), (uint8_t) (dport >> 8)));
        if (ret != RET_SUCCESS) goto SET_SN_DPORT_END;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_DPORT(sn), 1), (uint8_t) dport));

        SET_SN_DPORT_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_DPORT register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_DPORT.
     * @sa setSn_DPORT()
     */
    RetType getSn_DPORT(uint8_t sn, uint16_t *dport) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_DPORT(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_DPORT_END;

        *dport = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_DPORT(sn),1), &tmp));

        *dport += tmp;

        GET_SN_DPORT_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_MSSR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint16_t)mss Value to set @ref Sn_MSSR
     * @sa setSn_MSSR()
     */
    RetType setSn_MSSR(uint8_t sn, uint16_t mss) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_MSSR(sn), (uint8_t) (mss >> 8)));
        if (ret != RET_SUCCESS) goto SET_SN_MSSR;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_MSSR(sn), 1), (uint8_t) mss));

        SET_SN_MSSR:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_MSSR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_MSSR.
     * @sa setSn_MSSR()
     */
    RetType getSn_MSSR(uint8_t sn, uint16_t *mssr) {
        RESUME();

        static uint8_t tmp;
        RetType ret = CALL(WIZCHIP_READ(Sn_MSSR(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_MSSR_END;

        *mssr = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_MSSR(sn), 1), &tmp));
        *mssr += tmp;

        GET_SN_MSSR_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_TOS register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)tos Value to set @ref Sn_TOS
     * @sa getSn_TOS()
     */
    RetType setSn_TOS(uint8_t sn, uint8_t tos) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_TOS(sn), tos));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_TOS register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of Sn_TOS.
     * @sa setSn_TOS()
     */
    RetType getSn_TOS(uint8_t sn, uint8_t *tos) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_TOS(sn), tos));

        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_TTL register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)ttl Value to set @ref Sn_TTL
     * @sa getSn_TTL()
     */
    RetType setSn_TTL(uint8_t sn, uint8_t ttl) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_TTL(sn), ttl));

        RESET();
        return ret;
    }



    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_TTL register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_TTL.
     * @sa setSn_TTL()
     */
    RetType getSn_TTL(uint8_t sn, uint8_t *ttl) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_TTL(sn), ttl));

        RESET();
        return ret;
    }



    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_RXBUF_SIZE register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)rxbufsize Value to set @ref Sn_RXBUF_SIZE
     * @sa getSn_RXBUF_SIZE()
     */
    RetType setSn_RXBUF_SIZE(uint8_t sn, uint8_t rx_buf_size) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_RXBUF_SIZE(sn), rx_buf_size));

        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_RXBUF_SIZE register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_RXBUF_SIZE.
     * @sa setSn_RXBUF_SIZE()
     */
    RetType getSn_RXBUF_SIZE(uint8_t sn, uint8_t *rx_buf_size) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_RXBUF_SIZE(sn), rx_buf_size));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_TXBUF_SIZE register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)txbufsize Value to set @ref Sn_TXBUF_SIZE
     * @sa getSn_TXBUF_SIZE()
     */
    RetType setSn_TXBUF_SIZE(uint8_t sn, uint8_t tx_buf_size) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_TXBUF_SIZE(sn), tx_buf_size));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_TXBUF_SIZE register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_TXBUF_SIZE.
     * @sa setSn_TXBUF_SIZE()
     */
    RetType getSn_TXBUF_SIZE(uint8_t sn, uint8_t *tx_buf_size) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_TXBUF_SIZE(sn), tx_buf_size));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_TX_RD register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_TX_RD.
     */
    RetType getSn_TX_RD(uint8_t sn, uint16_t *tx_rd) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_TX_RD(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *tx_rd = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_RD(sn),1), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *tx_rd += tmp;

        GET_SN_RX_RD_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_TX_WR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint16_t)tx_wr Value to set @ref Sn_TX_WR
     * @sa GetSn_TX_WR()
     */
    RetType setSn_TX_WR(uint16_t sn, uint16_t tx_wr) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_TX_WR(sn), (uint8_t) (tx_wr >> 8)));
        if (ret != RET_SUCCESS) goto SET_SN_TX_WR_END;

        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_TX_WR(sn), 1), (uint8_t) tx_wr));

        SET_SN_TX_WR_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_TX_WR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_TX_WR.
     * @sa setSn_TX_WR()
     */
    RetType getSn_TX_WR(uint8_t sn, uint16_t *tx_wr) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_TX_WR(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *tx_wr = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_TX_WR(sn), 1), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *tx_wr += tmp;

        GET_SN_RX_RD_END:
        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_RX_RD register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint16_t)rx_rd Value to set @ref Sn_RX_RD
     * @sa getSn_RX_RD()
     */
    RetType setSn_RX_RD(uint8_t sn, uint16_t rx_rd) {
        RESUME();
        
        RetType ret = CALL(WIZCHIP_WRITE(Sn_RX_RD(sn), (uint8_t)(rx_rd >> 8)));
        if (ret != RET_SUCCESS) goto SET_SN_RX_RD_END;
        
        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_RX_RD(sn), 1), (uint8_t) rx_rd));
        
        SET_SN_RX_RD_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_RX_RD register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_RX_RD.
     * @sa setSn_RX_RD()
     */
    RetType getSn_RX_RD(uint8_t sn, uint16_t *rx_rd) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_RX_RD(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *rx_rd = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_RD(sn), 1), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *rx_rd += tmp;

        GET_SN_RX_RD_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_RX_WR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_RX_WR.
     */
    RetType getSn_RX_WR(uint8_t sn, uint16_t *rx_wr) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_RX_WR(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *rx_wr = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_RX_WR(sn),1), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *rx_wr += tmp;

        GET_SN_RX_RD_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_FRAG register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint16_t)frag Value to set @ref Sn_FRAG
     * @sa getSn_FRAD()
     */
    RetType setSn_FRAG(uint8_t sn, uint16_t frag) { 
        RESUME();
        
        RetType ret = CALL(WIZCHIP_WRITE(Sn_FRAG(sn),  (uint8_t) (frag >> 8)));
        if (ret != RET_SUCCESS) goto SET_SN_FRAG_END;
        
        ret = CALL(WIZCHIP_WRITE(WIZCHIP_OFFSET_INC(Sn_FRAG(sn),1), (uint8_t) frag));
        
        SET_SN_FRAG_END:
        RESET();
        return ret;
    }

    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_FRAG register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of @ref Sn_FRAG.
     * @sa setSn_FRAG()
     */
    RetType getSn_FRAG(uint8_t sn, uint16_t *frag) {
        RESUME();
        static uint8_t tmp;

        RetType ret = CALL(WIZCHIP_READ(Sn_FRAG(sn), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *frag = (uint16_t) tmp << 8;

        ret = CALL(WIZCHIP_READ(WIZCHIP_OFFSET_INC(Sn_FRAG(sn),1), &tmp));
        if (ret != RET_SUCCESS) goto GET_SN_RX_RD_END;

        *frag += tmp;

        GET_SN_RX_RD_END:
        RESET();
        return ret;
    }
    
    /**
     * @ingroup Socket_register_access_function
     * @brief Set @ref Sn_KPALVTR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @param (uint8_t)kpalvt Value to set @ref Sn_KPALVTR
     * @sa getSn_KPALVTR()
     */
    RetType setSn_KPALVTR(uint8_t sn, uint8_t kpalvt) {
        RESUME();

        RetType ret = CALL(WIZCHIP_WRITE(Sn_KPALVTR(sn), kpalvt));

        RESET();
        return ret;
    }


    /**
     * @ingroup Socket_register_access_function
     * @brief Get @ref Sn_KPALVTR register
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint8_t. Value of @ref Sn_KPALVTR.
     * @sa setSn_KPALVTR()
     */
    RetType getSn_KPALVTR(uint8_t sn, uint8_t *kpalvt) {
        RESUME();

        RetType ret = CALL(WIZCHIP_READ(Sn_KPALVTR(sn), kpalvt));

        RESET();
        return ret;
    }


/////////////////////////////////////
// Sn_TXBUF & Sn_RXBUF IO function //
/////////////////////////////////////
    /**
     * @brief Socket_register_access_function
     * @brief Gets the max buffer size of socket sn passed as parameter.
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of Socket n RX max buffer size.
     */
     RetType getSn_RxMAX(uint8_t sn, uint16_t *rx_max) {
         RESUME();
         static uint8_t tmp;

         RetType ret = CALL(getSn_RXBUF_SIZE(sn, &tmp));
         *rx_max = tmp << 10;

         RESET();
         return ret;
     }

    /**
     * @brief Socket_register_access_function
     * @brief Gets the max buffer size of socket sn passed as parameters.
     * @param (uint8_t)sn Socket number. It should be <b>0 ~ 7</b>.
     * @return uint16_t. Value of Socket n TX max buffer size.
     */
    RetType getSn_TxMAX(uint8_t sn, uint16_t *tx_max) {
         RESUME();
         static uint8_t tmp;

         RetType ret = CALL(getSn_TXBUF_SIZE(sn, &tmp));
         *tx_max = tmp << 10;

         RESET();
         return ret;
     }
};

#endif //WIZNET_H

//*****************************************************************************
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
