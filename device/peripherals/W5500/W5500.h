/*******************************************************************************
*
*  Name: w5500.h
*
*  Purpose: Implementation of a Wiznet W5500 Ethernet chip driver. Written to be
*           platform independent, requires a "StreamDevice" that handles SPI
*           communication for the platform and a "GPIODevice" to control the
*           chip select (TODO add the GPIODevice here).
*
*           Currently only works in variable data length mode, so the chip
*           select must be used and cannot be tied to ground like for fixed
*           data length mode.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef W5500_H
#define W5500_H

#include <stdlib.h>
#include <stdint.h>

#include "device/peripherals/W5500/W5500_defines.h"
#include "device/peripherals/W5500/W5500_socket.h"
#include "device/StreamDevice.h"
#include "sched/macros.h"
#include "return.h"

/// @brief controller for W5500 device
class W5500 {
public:
    /// @brief constructor
    /// @param spi      SPI controller device
    // TODO pass in GPIO for CS
    W5500(SPIDevice& spi, GPIODevice& gpio) : m_spi(spi), m_gpio(gpio) {
        for(size_t i = 0; i < static_cast<int>(W5500_NUM_SOCKETS); i++) {
            m_states[i] = {false, false};
            m_claimed[i] = false;
            m_tids[i] = -1;
        }
    };

    /// @brief initialize the chip
    /// @param gw       gateway IPv4 address
    /// @param subnet   subnet IPv4 address mask
    /// @param mac      source MAC address for the device
    /// @param ip       source IP address for the device
    /// @return
    RetType init(uint8_t gw[4], uint8_t subnet[4], uint8_t mac[6], uint8_t ip[4]) {
        RESUME();

        RetType ret;

        // mode configuration:
        //  reset = 1
        //  reserved
        //  wake on lan = 0
        //  ping block = 0
        //  PPPoE = 0
        //  reserved
        //  force ARP = 0
        //  reserved
        uint8_t mode = 0b1000000;

        // PHY configuration:
        //  reset = 1
        //  config operation mode = 1 (use the next 3 bits instead of HW pins)
        //  operation mode = 111 (all capable, auto negotation)
        //  all else read only
        uint8_t phy_cfg = 0b11111000;

        // mode
        ret = CALL(write_bytes(W5500_COMMON_REG, W5500_COMMON_MR, &mode, 1));
        if(ret != RET_SUCCESS) {
            goto init_end;
        }

        // gateway address
        ret = CALL(write_bytes(W5500_COMMON_REG, W5500_COMMON_GAR0, gw, 4));
        if(ret != RET_SUCCESS) {
            goto init_end;
        }

        // subnet mask address
        ret = CALL(write_bytes(W5500_COMMON_REG, W5500_COMMON_SUBR0, subnet, 4));
        if(ret != RET_SUCCESS) {
            goto init_end;
        }

        // source MAC address
        ret = CALL(write_bytes(W5500_COMMON_REG, W5500_COMMON_SHAR0, mac, 6));
        if(ret != RET_SUCCESS) {
            goto init_end;
        }

        // source IP address
        ret = CALL(write_bytes(W5500_COMMON_REG, W5500_COMMON_SIPR0, ip, 4));
        if(ret != RET_SUCCESS) {
            goto init_end;
        }

        // PHY
        ret = CALL(write_bytes(W5500_COMMON_REG, W5500_COMMON_PHYCFGR, &phy_cfg, 1));

        // don't care about any other settings at the moment
        // TODO maybe do some interrupt masking until we actually open the socket?
        // TODO TCP will need some more settings like rety count and retry time
        // TODO read the version and make sure we are talking to a W5500?

    init_end:
        RESET();
        return ret;
    }

    /// @brief set a sockets source address and port
    /// @param sock     the socket to set
    /// @param ipaddr   the new source IPv4 address, in system endianness
    /// @param port     the new source port, in system endianness
    /// @return
    RetType set_src(W5500Socket_t sock, uint32_t ipaddr, uint16_t port) {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief set the destination address and port
    /// @param sock     the socket to set
    /// @param ipaddr   the new destination IPv4 address, in system endianness
    /// @param port     the new destination port, in system endianness
    /// @return
    RetType set_dst(W5500Socket_t sock, uint32_t ipaddr, uint16_t port) {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief open a socket
    /// @param sock         the socket to open
    /// @param mode         the mode to open it in
    /// @param multicast    'true' if this socket should listen for multicasts,
    ///                      or for raw sockets, true if it should listen for IPv6
    /// @return
    RetType open(W5500Socket_t sock, W5500SocketMode_t mode) {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief close a socket
    /// @param sock     the socket to close
    /// @return
    RetType close(W5500Socket_t sock) {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief process interrupt
    /// NOTE: this can be triggered by an ISR or polled
    ///       it uses scheduler macros so it should not be called directly from an ISR
    /// @return
//    RetType process_int() {
//        RESUME();
//
//        RetType ret;
//        uint8_t sir;
//
//        // TODO we don't check for any interrupts in the common block
//        // most of those aren't that interesting, but maybe IP conflict is worth looking at
//
//        // TODO set GPIO for CS
//
//        // read the socket interrupt register
//        ret = CALL(read_bytes(W5500_COMMON_REG, W5500_COMMON_SIR, &sir, 1));
//        if(ret != RET_SUCCESS) {
//            goto process_int_end;
//        }
//
//        // check which sockets have interrupts
//        for(int i = 0; i < static_cast<int>(W5500_NUM_SOCKETS); i++) {
//            if(!(sir & (1 << i))) {
//                // this socket does not have an interrupt
//                continue;
//            } // else we need to process the interrupt for this socket
//
//            W5500SocketDescriptor_t sock = W5500_SOCKET_MAP[i];
//
//            uint8_t ir;
//            ret = CALL(read_bytes(sock.reg, W5500_SOCKET_IR, &ir, 1));
//            if(ret != RET_SUCCESS) {
//                // try the other sockets instead of failing out
//                continue;
//            }
//
//            // TODO we only check receive and send complete interrupt for now
//
//            bool updated = false;
//
//            // send ok
//            if(ir & (0b10000)) {
//                m_states[i].send_ok = true;
//
//                // TODO if anything wrote to the buffer while we were waiting
//                //      for a tx to finish, start another SEND command
//                if(m_states[i].)
//
//                updated = true;
//            }
//
//            // recv
//            if(ir & (0b100)) {
//                m_states[i].rx_ready = true;
//                updated = true;
//            }
//
//            if(updated) {
//                // wake up the task that called registered for this socket if there is one
//                if(m_tids[i] != -1) {
//                    WAKE(m_tids[i]);
//                }
//            }
//
//            // clear the interrupts
//            ret = CALL(write_bytes(sock.reg, W5500_SOCKET_IR, &ir, 1));
//            if(ret != RET_SUCCESS) {
//                // try the other sockets
//                continue;
//            }
//        }
//
//    process_int_end:
//        // TODO reset GPIO for CS
//        RESET();
//        return ret;
//    }

    /// @brief register a task to be woken up when this socket updates
    /// NOTE: there can be only one task registered, the new one overwrites the old one
    /// @param sock     the socket the task should wait for
    /// @param task     the task to register
    void register_task(W5500Socket_t sock, tid_t task) {
        m_tids[static_cast<int>(sock)] = task;
    }

    /// @brief start a transmit a packet over a socket
    /// the transmit is complete when the socket state tx_send_ok flag is 'true'
    /// @param sock     the socket to transmit on
    /// @param buff     the packet to send
    /// @param len      the length of the packet
    /// @return
    RetType start_transmit(W5500Socket_t sock, uint8_t* buff, size_t len) {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief receive a packet from a socket
    /// @param sock     the socket to transmit on
    /// @param buff     the buffer to read int
    /// @param len      the length of the buffer
    /// @param read     a pointer that will be set to the number of bytes read
    /// NOTE: 'read' will report the actual packet size even if greater than 'len',
    //         only len bytes will be copied into 'buff'. If there are no packets
    //         to read, 'read' will be 0 and nothing is copied to 'buff'.
    /// @return
    RetType start_receive(W5500Socket_t sock, uint8_t* buff, size_t len, size_t* read) {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief get the state of a socket
    /// @param sock     the socket to get the state of
    /// @return a reference to the socket's state
    W5500SocketState_t& state(W5500Socket_t sock) {
        return m_states[static_cast<int>(sock)];
    }

    /// @brief attempt to claim a socket
    ///        will claim socket 0 last as it's the only one that can do MACRAW mode
    ///        unless the 'raw' flag is true
    /// @param sock     a pointer that will be set to the claimed socket
    /// @param raw      if we're attemmpting to claim a raw socket
    /// @return
    RetType claim_sock(W5500Socket_t* sock, bool raw = false) {
        if(raw) {
            // we need socket 0
            if(m_claimed[0]) {
                return RET_ERROR;
            }

            *sock = W5500_SOCKET0;
            m_claimed[0] = true;

            return RET_SUCCESS;
        }

        // look for an unclaimed socket (not socket 0)
        bool claimed = false;
        for(size_t i = 1; i < static_cast<int>(W5500_NUM_SOCKETS); i++) {
            if(!m_claimed[i]) {
                // we can claim this socket
                *sock = static_cast<W5500Socket_t>(i);
                m_claimed[i] = true;
                claimed = true;
                break;
            }
        }

        // if we haven't found any unclaimed socket, we can take socket 0
        if(!claimed) {
            if(m_claimed[0]) {
                return RET_ERROR;
            }

            *sock = W5500_SOCKET0;
            m_claimed[0] = true;
        }

        return RET_SUCCESS;
    }

    /// @brief unclaim a socket
    /// @param sock     the socket to unclaim
    void unclaim_sock(W5500Socket_t sock) {
        m_claimed[static_cast<int>(sock)] = false;
    }

        /***
     * @brief Read an eight bit value to a register
     * @param block_select_bit
     * @param reg
     * @param val
     * @return
     */
    RetType read_register(uint8_t block_select_bit, uint8_t reg, uint8_t *result) {
        RESUME();

        txBuffer[0] = 0x00;
        txBuffer[1] = reg;
        txBuffer[2] = block_select_bit | W5500_CTRL_READ;
        rxBuffer[3] = 0x00; // Intentionally zero out the read value

        RetType ret = CALL(m_gpio.set(0));
        if (ret != RET_SUCCESS) goto write_register8_end;

        ret = CALL(m_spi.write_read(txBuffer, rxBuffer, 4));
        if (ret != RET_SUCCESS) goto write_register8_end;

        *result = rxBuffer[0];

        write_register8_end:
        ret = CALL(m_gpio.set(1));
        RESET();
        return ret;
    }


private:

    /// @brief helper function to write data to the chip
    /// @param block_addr       the block address to write to
    /// @param offset_addr      the offset address to write to
    /// @param buff             the data to write
    /// @param len              the length of 'buff' in bytes
    /// @return
    RetType write_bytes(uint8_t block_addr, uint16_t offset_addr, uint8_t* buff, size_t len) {
        RESUME();
        RetType ret;

        uint8_t cmd[3];

        // address phase (MSB -> LSB)
        cmd[0] = (uint8_t)(offset_addr >> 8); // MSB
        cmd[1] = (uint8_t)(offset_addr);      // LSB

        // control phase
        cmd[2] = (block_addr << W5500_BS_SHIFT); // block select
        cmd[2] |= (W5500_WRITE << W5500_RW_SHIFT);       // write access mode
        cmd[2] |= (W5500_VDL_MODE << W5500_OM_SHIFT);    // variable data length mode
        // TODO this would need to change to support fixed data length mode

        // TODO toggle CS here!!!!!

        // send the address and control phase data
        ret = CALL(m_spi.write(cmd, 3));

        // something bad happened
        if(ret != RET_SUCCESS) {
            goto write_bytes_end;
        }

        // send the data
        ret = CALL(m_spi.write(buff, len));

        // something bad happened
        if(ret != RET_SUCCESS) {
            goto write_bytes_end;
        }


    write_bytes_end:
        // TODO toggle CS back here!!!!!

        RESET();
        return RET_SUCCESS;
    }

    /// @brief helper function to read data from the chip
    /// @param block_addr       the block address to read from
    /// @param offset_addr      the offset address to read from
    /// @param buff             the buffer pointer to read into
    /// @param len              the length of 'buff' in bytes
    /// @return
    RetType read_bytes(uint8_t block_addr, uint16_t offset_addr, uint8_t* buff, size_t len) {
        RESUME();
        RetType ret;

        uint8_t cmd[3];

        // address phase (MSB -> LSB)
        cmd[0] = (uint8_t)(offset_addr >> 8); // MSB
        cmd[1] = (uint8_t)(offset_addr);      // LSB

        // control phase
        cmd[2] = (block_addr << W5500_BS_SHIFT); // block select
        cmd[2] |= (W5500_WRITE << W5500_RW_SHIFT);       // write access mode
        cmd[2] |= (W5500_VDL_MODE << W5500_OM_SHIFT);    // variable data length mode
        // TODO this would need to change to support fixed data length mode

        // TODO toggle CS here!!!!!

        // send the address and control phase data
        ret = CALL(m_spi.write(cmd, 3));

        // something bad happened
        if(ret != RET_SUCCESS) {
            goto read_bytes_end;
        }

        // send the data
        ret = CALL(m_spi.read(buff, len));

        // something bad happened
        if(ret != RET_SUCCESS) {
            goto read_bytes_end;
        }


    read_bytes_end:
        // TODO toggle CS back here!!!!!

        RESET();
        return RET_SUCCESS;
    }

    // passed in SPI controller
    SPIDevice& m_spi;
    GPIODevice& m_gpio;

    // SPI Transaction Buffers
    // Reduce static memory usage by using a single buffer for all SPI transactions
    uint8_t txBuffer[8] = {};
    uint8_t rxBuffer[8] = {};

    // socket states
    W5500SocketState_t m_states[static_cast<int>(W5500_NUM_SOCKETS)];

    // which sockets are currently claimed
    bool m_claimed[static_cast<int>(W5500_NUM_SOCKETS)];

    // tasks that should be woken up when each socket is updated (tx complete or rx ready)
    tid_t m_tids[static_cast<int>(W5500_NUM_SOCKETS)];
};

#endif
