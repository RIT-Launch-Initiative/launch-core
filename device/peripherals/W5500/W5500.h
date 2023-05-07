/*******************************************************************************
*
*  Name: w5500.h
*
*  Purpose: Implementation of a Wiznet W5500 Ethernet chip driver
*
*           Currently only works in variable data length mode, so the chip
*           select must be used and cannot be tied to ground like for fixed
*           data length mode.
*
*  Author:  Aaron Chan
*           Will Merges
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
#include "device/peripherals/W5500/W5500_defs.h"
#include "device/StreamDevice.h"
#include "net/network_layer/NetworkLayer.h"
#include "net/packet/Packet.h"
#include "sched/macros.h"
#include "return.h"

#include "stm32f4xx_hal_uart.h"

extern UART_HandleTypeDef huart2;

#define DEFAULT_SOCKET_NUM 0


/// @brief controller for W5500 device
class W5500 : public NetworkLayer {
public:
    /// @brief constructor
    /// @param spi      SPI controller device
    W5500(SPIDevice &spi, GPIODevice &gpio) : m_spi(spi), m_gpio(gpio) {
        for (size_t i = 0; i < static_cast<int>(W5500_NUM_SOCKETS); i++) {
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

        static uint8_t chip_id = 0;
        RetType ret = CALL(get_chip_version(&chip_id));
        if (chip_id != 4) {

            ret = RET_ERROR;
            goto init_end;
        }

//        ret = CALL(set_tx_rx_rate(0x0800, 0x0800)); // TODO: No magic allowed
//        if (ret != RET_SUCCESS) goto init_end;

        // mode configuration:
        //  reset = 1
        //  reserved
        //  wake on lan = 0
        //  ping block = 0
        //  PPPoE = 0
        //  reserved
        //  force ARP = 0
        //  reserved
        static uint8_t mode = 0b1000000;
        ret = CALL(write_mode(mode));


        // PHY configuration:
        //  reset = 1
        //  config operation mode = 1 (use the next 3 bits instead of HW pins)
        //  operation mode = 111 (all capable, auto negotation)
        //  all else read only
        static uint8_t phy_cfg = 0b11111000;
        ret = CALL(write_phy_cfg(phy_cfg));
        if (ret != RET_SUCCESS) goto init_end;


        // Set our local MAC address
        ret = CALL(set_mac_addr(mac));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_socket_mode_reg(DEFAULT_SOCKET_NUM, MAC_RAW_MODE));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_socket_control_reg(DEFAULT_SOCKET_NUM, OPEN_SOCKET));
        if (ret != RET_SUCCESS) goto init_end;

// TODO: Check if Sn_SR is in MAC_RAW mode
        ret = CALL(set_socket_mode_tx_size(DEFAULT_SOCKET_NUM, 16));
        if (ret != RET_SUCCESS) goto init_end;

        ret = CALL(set_socket_mode_rx_size(DEFAULT_SOCKET_NUM, 16));
        if (ret != RET_SUCCESS) goto init_end;

        // gateway address
//        ret = CALL(set_gateway_addr(gw));
//        if (ret != RET_SUCCESS) return ret;
//
//        // subnet mask address
//        ret = CALL(set_subnet_mask(subnet));
//        if (ret != RET_SUCCESS) goto init_end;
//
//        // source MAC address
//        ret = CALL(set_mac_addr(mac));
//        if (ret != RET_SUCCESS) goto init_end;
//
//        // source IP address
        ret = CALL(set_src_ip(ip));
        if (ret != RET_SUCCESS) goto init_end;
//
//        // dst IP address
//        ret = CALL(set_dst(W5500_SOCKET0, 0xFFFFFFFF, 10000));
//        if (ret != RET_SUCCESS) goto init_end;

        // don't care about any other settings at the moment
        // TODO maybe do some interrupt masking until we actually open the socket?
        // TODO TCP will need some more settings like rety count and retry time

        init_end:
    RESET();
        return ret;
    }

    RetType receive(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        RESUME();

        static uint16_t len;
        static uint8_t head[2];
        static uint16_t data_len;

//        uint16_t len = getSn_RX_RSR();

//        if (len > 0) {
//            data_len = 0;
//
//            wizchip_recv_data(head, 2);
//            setSn_CR(Sn_CR_RECV);
//
//            data_len = head[0];
//            data_len = (data_len << 8) + head[1];
//            data_len -= 2;
//
//            if (data_len > packet.available()) {
//                // Packet is bigger than buffer - drop the packet
//                wizchip_recv_ignore(data_len);
//                setSn_CR(Sn_CR_RECV);
//                return 0;
//            }
//
//            wizchip_recv_data(buffer, data_len);
//            setSn_CR(Sn_CR_RECV);
//
//            // Had problems with W5500 MAC address filtering (the Sn_MR_MFEN option)
//            // Do it in software instead:
//            if ((buffer[0] & 0x01) || memcmp(&buffer[0], _mac_address, 6) == 0) {
//                // Addressed to an Ethernet multicast address or our unicast address
//                return data_len;
//            } else {
//                return 0;
//            }
//        }

        receive_end:
        RESET();
        return RET_SUCCESS;
    }

    RetType transmit(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        return RET_SUCCESS;
    }

    RetType transmit2(Packet &packet, netinfo_t &info, NetworkLayer *caller) override {
        RESUME();

        static uint8_t *buff;
        static uint16_t len;
        static uint8_t tmp;
        buff = packet.raw();
        len = packet.header_size() + packet.size();


        static uint8_t socket_status;
        static uint16_t free_size;

        RetType ret;
        // Spinlock until we have space in TX buff
        while (1) {
            ret = CALL(get_socket_tx_fsr(DEFAULT_SOCKET_NUM, &free_size));
            if (ret != RET_SUCCESS) goto transmit2_end;

            ret = CALL(read_reg(W5500_Sn_SR(DEFAULT_SOCKET_NUM), &socket_status));
            if (ret != RET_SUCCESS) goto transmit2_end;

            if (socket_status == CLOSE_SOCKET) {
                RESET();
                return RET_ERROR;
            }

            if (len <= free_size) {
                break;
            }

            YIELD(); // Try again later
        };

        ret = CALL(send_data(buff, len));
        if (ret != RET_SUCCESS) goto transmit2_end;

        ret = CALL(set_socket_control_reg(DEFAULT_SOCKET_NUM, SEND_SOCKET));
        if (ret != RET_SUCCESS) goto transmit2_end;

        while (1) {
            ret = CALL(read_reg(W5500_Sn_IR(DEFAULT_SOCKET_NUM), &tmp));
            if (ret != RET_SUCCESS) goto transmit2_end;

            if (tmp & W5500_Sn_IR_SENDOK) {
                ret = CALL(set_socket_interrupt_reg(DEFAULT_SOCKET_NUM, W5500_Sn_IR_SENDOK));
                if (ret != RET_SUCCESS) goto transmit2_end;
                break; // Sent!
            }

            if (tmp & W5500_Sn_IR_TIMEOUT) {
                CALL(set_socket_interrupt_reg(DEFAULT_SOCKET_NUM, W5500_Sn_IR_TIMEOUT));
                goto transmit2_end; // ret doesn't matter. It's still a fail :(
            }

            YIELD();
        }


        ret = CALL(set_socket_control_reg(DEFAULT_SOCKET_NUM, OPEN_SOCKET));


        transmit2_end:
        RESET();
        return ret;
    }

    RetType send_data(uint8_t *buff, uint16_t len) {
        RESUME();

        static uint16_t ptr;
        static uint8_t addr_sel;
        ptr = 0;

        if (len == 0) {
            RESET();
            return RET_SUCCESS;
        }

        RetType ret = CALL(get_socket_register_tx_wr(DEFAULT_SOCKET_NUM, &ptr));
        if (ret != RET_SUCCESS) goto send_data_end;

        addr_sel = ((uint32_t) ptr << 8) + (W5500_WIZCHIP_TXBUF_BLOCK(DEFAULT_SOCKET_NUM) << 3);

        ret = CALL(write_buff(addr_sel, buff, len));
        if (ret != RET_SUCCESS) goto send_data_end;

        ptr += len;
        ret = CALL(set_socket_register_tx_wr(DEFAULT_SOCKET_NUM, ptr));
        if (ret != RET_SUCCESS) goto send_data_end;

        ret = CALL(set_socket_control_reg(DEFAULT_SOCKET_NUM, SEND_SOCKET));

        send_data_end:
        RESET();
        return RET_SUCCESS;
    }

//    RetType recv_data(uint8_t* buff, uint16_t len) {
//        RESUME();
//        static uint16_t ptr;
//
//        if(len == 0) goto recv_data_end;
//
//        RetType ret = CALL(get_socket_read_ptr_reg(DEFAULT_SOCKET_NUM, &ptr));
//        if (ret != RET_SUCCESS) goto recv_data_end;
//
//        ret = CALL(read_buff(ptr, buff, len, W5500_WIZCHIP_RXBUF_BLOCK(DEFAULT_SOCKET_NUM)));
//        if (ret != RET_SUCCESS) goto recv_data_end;
//
//        ptr += len;
//
//        setSn_RX_RD(ptr);
//
//
//        recv_data_end:
//        RESET();
//        return ret;
//    }

    RetType set_socket_mode_reg(uint8_t socket_num, uint8_t mode) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_MR(socket_num), mode));

        RESET();
        return ret;
    }

    RetType get_socket_mode_reg(uint8_t socket_num, uint8_t *mode) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_Sn_MR(socket_num), mode));

        RESET();
        return ret;
    }

    RetType set_socket_control_reg(uint8_t socket_num, uint8_t mode) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_CR(socket_num), mode));

        RESET();
        return ret;
    }

    RetType get_socket_control_reg(uint8_t socket_num, uint8_t *mode) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_Sn_CR(socket_num), mode));

        RESET();
        return ret;
    }

    RetType set_socket_mode_tx_size(uint8_t socket_num, uint8_t size) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_TXBUF_SIZE(socket_num), size));

        RESET();

        return ret;
    }

    RetType get_socket_mode_tx_size(uint8_t socket_num, uint8_t *size) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_Sn_TXBUF_SIZE(socket_num), size));

        RESET();

        return ret;
    }

    RetType set_socket_mode_rx_size(uint8_t socket_num, uint8_t size) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_TXBUF_SIZE(socket_num), size));

        RESET();

        return ret;
    }

    RetType get_socket_mode_rx_size(uint8_t socket_num, uint8_t *size) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_Sn_TXBUF_SIZE(socket_num), size));

        RESET();

        return ret;
    }

    RetType get_socket_tx_fsr(uint8_t socket_num, uint16_t *result) {
        RESUME();

        static uint16_t val;
        static uint16_t val1;
        static uint8_t current_sock_num;
        val = 0;
        val1 = 0;

        static uint8_t tmp;

        RetType ret;
        do {
            current_sock_num = W5500_Sn_TX_FSR(socket_num);

            ret = CALL(read_reg(current_sock_num, (uint8_t *) &val1));
            if (ret != RET_SUCCESS) goto get_socket_tx_fsr_end;

            ret = CALL(read_reg(W5500_WIZCHIP_OFFSET_INC(current_sock_num, 1), (uint8_t *) &tmp));
            if (ret != RET_SUCCESS) goto get_socket_tx_fsr_end;
            val1 = (val1 << 8) + tmp;

            if (val1 != 0) {
                tmp = W5500_Sn_TX_FSR(socket_num);

                ret = CALL(read_reg(tmp, (uint8_t *) &val));
                if (ret != RET_SUCCESS) goto get_socket_tx_fsr_end;

                ret = CALL(read_reg(W5500_WIZCHIP_OFFSET_INC(tmp, 1), (uint8_t *) &tmp));
                if (ret != RET_SUCCESS) goto get_socket_tx_fsr_end;

                val = (val << 8) + tmp;
                *result = val;
            }
        } while (val != val1);

        get_socket_tx_fsr_end:
        RESET();
        return ret;
    }

    RetType get_socket_interrupt_reg(uint8_t socket_num, uint8_t *result) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_Sn_IR(socket_num), result));

        RESET();

        return ret;
    }

    RetType set_socket_interrupt_reg(uint8_t socket_num, uint8_t val) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_IR(socket_num), val));

        RESET();

        return ret;
    }

    RetType set_socket_read_ptr_reg(uint8_t socket_num, uint8_t val) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_RX_RD(socket_num), (uint8_t) (val >> 8)));
        if (ret != RET_SUCCESS) goto write_socket_read_ptr_reg_end;

        ret = CALL(write_reg(W5500_WIZCHIP_OFFSET_INC(W5500_Sn_RX_RD(socket_num), 1), val));
        write_socket_read_ptr_reg_end:
        RESET();
        return ret;
    }

//    RetType get_socket_read_ptr_reg(uint8_t socket_num, uint16_t *result) {
//        RESUME();
//
//        static uint8_t tmp;
//
//        RetType ret = CALL(write_reg(W5500_Sn_RX_RD(socket_num) << 8, tmp));
//        if (ret != RET_SUCCESS) goto get_socket_read_ptr_reg_end;
//        result = tmp;
//
//        CALL(write_reg(W5500_WIZCHIP_OFFSET_INC(Sn_RX_RD(sn), 1), &tmp));
//        if (ret != RET_SUCCESS) goto get_socket_read_ptr_reg_end;
//        result += tmp;
//
//        get_socket_read_ptr_reg_end:
//        RESET();
//        return ret;
//    }


    RetType write_mode(uint8_t mode) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_MR, mode));

        RESET();
        return ret;
    }

    RetType read_mode(uint8_t *mode) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_MR, mode));

        RESET();
        return ret;

    }

    RetType write_phy_cfg(uint8_t phy_cfg) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_PHYCFGR, phy_cfg));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }

    RetType set_src_ip(uint8_t source_ip[4]) {
        RESUME();

        RetType ret = CALL(write_buff(W5500_SIPR, source_ip, 4));

        RESET();
        return ret;
    }

    RetType get_src_ip(uint8_t *source_ip) {
        RESUME();

        RetType ret = CALL(read_buff(W5500_SIPR, source_ip, 4));

        RESET();
        return ret;
    }


    /// @brief set the destination address and port
    /// @param sock     the socket to set
    /// @param ipaddr   the new destination IPv4 address, in system endianness
    /// @param port     the new destination port, in system endianness
    /// @return
    RetType set_dst(W5500Socket_t sock, uint32_t ipaddr, uint16_t port) {
        RESUME();

        static uint8_t ip[4] = {};
        static uint8_t port_bytes[2] = {};

        ip[0] = (ipaddr >> 24) & 0xFF;
        ip[1] = (ipaddr >> 16) & 0xFF;
        ip[2] = (ipaddr >> 8) & 0xFF;
        ip[3] = ipaddr & 0xFF;

        port_bytes[0] = (port >> 8) & 0xFF;
        port_bytes[1] = port & 0xFF;

        RetType ret = CALL(write_buff(W5500_Sn_DIPR(sock), ip, 4));
        if (ret != RET_SUCCESS) return ret;

        ret = CALL(write_buff(W5500_Sn_DPORT(sock), port_bytes, 2));
        if (ret != RET_SUCCESS) return ret;

        RESET();
        return ret;
    }


    /// @brief open a socket
    /// @param sock         the socket to open
    /// @param mode         the mode to open it in
    /// @param multicast    'true' if this socket should listen for multicasts,
    ///                      or for raw sockets, true if it should listen for IPv6
    /// @return
//    RetType open(W5500Socket_t sock, W5500SocketMode_t mode) {
//        RESUME();
//
//        RetType ret = CALL(set_socket_register_mode(sock, mode));
//        if (ret != RET_SUCCESS) return ret;
//
//        ret = CALL(set_socket_register_command(sock, W5500SocketCommand_t::OPEN));
//        if (ret != RET_SUCCESS) return ret;
//
//    open_end:
//        RESET();
//
//        return RET_SUCCESS;
//    }
//
//    RetType set_socket_register_mode(W5500Socket_t sock, W5500SocketMode_t mode) {
//        RESUME();
//
//        static uint8_t mode_byte = 0;
//
//        mode_byte = static_cast<uint8_t>(mode);
//
//        RetType ret = CALL(write_buff(W5500_Sn_MR(sock), &mode_byte, 1));
//        if (ret != RET_SUCCESS) return ret;
//
//        set_socket_register_mode_end:
//        RESET();
//        return ret;
//    }

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
    RetType start_transmit(W5500Socket_t sock, uint8_t *buff, size_t len) {
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
    RetType start_receive(W5500Socket_t sock, uint8_t *buff, size_t len, size_t *read) {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief get the state of a socket
    /// @param sock     the socket to get the state of
    /// @return a reference to the socket's state
    W5500SocketState_t &state(W5500Socket_t sock) {
        return m_states[static_cast<int>(sock)];
    }

    /// @brief attempt to claim a socket
    ///        will claim socket 0 last as it's the only one that can do MACRAW mode
    ///        unless the 'raw' flag is true
    /// @param sock     a pointer that will be set to the claimed socket
    /// @param raw      if we're attemmpting to claim a raw socket
    /// @return
    RetType claim_sock(W5500Socket_t *sock, bool raw = false) {
        if (raw) {
            // we need socket 0
            if (m_claimed[0]) {
                return RET_ERROR;
            }

            *sock = W5500_SOCKET0;
            m_claimed[0] = true;

            return RET_SUCCESS;
        }

        // look for an unclaimed socket (not socket 0)
        bool claimed = false;
        for (size_t i = 1; i < static_cast<int>(W5500_NUM_SOCKETS); i++) {
            if (!m_claimed[i]) {
                // we can claim this socket
                *sock = static_cast<W5500Socket_t>(i);
                m_claimed[i] = true;
                claimed = true;
                break;
            }
        }

        // if we haven't found any unclaimed socket, we can take socket 0
        if (!claimed) {
            if (m_claimed[0]) {
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

        tx_buffer[0] = 0x00;
        tx_buffer[1] = reg;
        tx_buffer[2] = block_select_bit | W5500_CTRL_READ;
        rx_buffer[3] = 0x00; // Intentionally zero out the read value

        RetType ret = CALL(m_gpio.set(0));
        if (ret != RET_SUCCESS) goto read_register8_end;

        ret = CALL(m_spi.write_read(tx_buffer, rx_buffer, 4));
        if (ret != RET_SUCCESS) goto read_register8_end;

        *result = rx_buffer[0];

        read_register8_end:
        ret = CALL(m_gpio.set(1));

        RESET();
        return RET_SUCCESS;
    }

    RetType set_mac_addr(uint8_t mac[6]) {
        RESUME();

        RetType ret = CALL(write_buff(W5500_SHAR, mac, 6));

        RESET();
        return ret;
    }

    RetType get_mac_addr(uint8_t *mac) {
        RESUME();

        RetType ret = CALL(read_buff(W5500_SHAR, mac, 6));

        RESET();
        return ret;
    }

    RetType set_gateway_addr(uint8_t gateway[4]) {
        RESUME();

        RetType ret = CALL(write_buff(W5500_GAR, gateway, 4));

        RESET();
        return ret;
    }

    RetType get_gateway_addr(uint8_t gateway[4]) {
        RESUME();

        RetType ret = CALL(read_buff(W5500_GAR, gateway, 4));

        RESET();
        return ret;
    }

    RetType set_subnet_mask(uint8_t subnet[4]) {
        RESUME();

        RetType ret = CALL(write_buff(W5500_SUBR, subnet, 4));

        RESET();
        return ret;
    }

    RetType get_subnet_mask(uint8_t subnet[4]) {
        RESUME();

        RetType ret = CALL(read_buff(W5500_SUBR, subnet, 4));

        RESET();
        return ret;
    }

    RetType set_phy_reg(uint8_t reg) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_PHYCFGR, reg));

        RESET();
        return ret;
    }


    RetType get_phy_reg(uint8_t *reg) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_PHYCFGR, reg));

        RESET();
        return ret;
    }

    RetType get_chip_version(uint8_t *version) {
        RESUME();

        RetType ret = CALL(read_reg(W5500_VERSIONR, version));

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

    // socket states
    W5500SocketState_t m_states[static_cast<int>(W5500_NUM_SOCKETS)];

    // which sockets are currently claimed
    bool m_claimed[static_cast<int>(W5500_NUM_SOCKETS)];

    // tasks that should be woken up when each socket is updated (tx complete or rx ready)
    tid_t m_tids[static_cast<int>(W5500_NUM_SOCKETS)];

    RetType read_reg(uint32_t addr_select, uint8_t *result) {
        RESUME();
        static uint8_t data[3];
        RetType ret = CALL(m_gpio.set(0));
        if (ret != RET_SUCCESS) goto read_reg_end;

        addr_select |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);

        data[0] = (addr_select & 0x00FF0000) >> 16;
        data[1] = (addr_select & 0x0000FF00) >> 8;
        data[2] = (addr_select & 0x000000FF) >> 0;

        ret = CALL(m_spi.write(data, 3));
        if (ret != RET_SUCCESS) goto read_reg_end;

        ret = CALL(m_spi.read(result, 1));
        if (ret != RET_SUCCESS) goto read_reg_end;

        read_reg_end:
        CALL(m_gpio.set(1));
        RESET();
        return ret;
    }

    RetType write_reg(uint32_t addr_sel, uint8_t write_byte) {
        RESUME();

        static uint8_t data[4];

        RetType ret = CALL(m_gpio.set(0));
        if (ret != RET_SUCCESS) goto write_reg_end;

        addr_sel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

        data[0] = (addr_sel & 0x00FF0000) >> 16;
        data[1] = (addr_sel & 0x0000FF00) >> 8;
        data[2] = (addr_sel & 0x000000FF) >> 0;
        data[3] = write_byte;

        ret = CALL(m_spi.write(data, 4));
        if (ret != RET_SUCCESS) goto write_reg_end;

        write_reg_end:
        CALL(m_gpio.set(1));
        RESET();
        return ret;
    }

    // TODO: Hardfault after some time when doing a write_read
    RetType read_buff(uint32_t addr_sel, uint8_t *buff, uint16_t len, uint8_t block = 0) {
        RESUME();

        // Original write with (data, 3) and read with (buff, len). Going to try write_read
        static uint8_t data[16];
        RetType ret = CALL(m_gpio.set(0));
        if (ret != RET_SUCCESS) goto read_buff_end;

        addr_sel |= (_W5500_SPI_READ_ | _W5500_SPI_VDM_OP_);
        data[0] = (addr_sel & 0x00FF0000) >> 16;
        data[1] = (addr_sel & 0x0000FF00) >> 8;
        if (block) {
            data[2] = (addr_sel & 0x000000FF) >> 0;
        }
        for (int i = 0; i < len; i++) {
            data[i + (block ? 3 : 2)] = *(buff + i);
        }

        ret = CALL(m_spi.write_read(data, data, block ? len + 3 : len + 2));
        if (ret != RET_SUCCESS) goto read_buff_end;

        read_buff_end:
        CALL(m_gpio.set(1));
        RESET();
        return ret;
    }

    RetType write_buff(uint32_t addr_sel, uint8_t *buff, uint16_t len, uint8_t block = 0) {
        RESUME();
        static uint8_t data[3];

        RetType ret = CALL(m_gpio.set(0));
        if (ret != RET_SUCCESS) goto write_buff_end;

        addr_sel |= (_W5500_SPI_WRITE_ | _W5500_SPI_VDM_OP_);

        data[0] = (addr_sel & 0x00FF0000) >> 16;
        data[1] = (addr_sel & 0x0000FF00) >> 8;
        data[2] = (addr_sel & 0x000000FF) >> 0;
        ret = CALL(m_spi.write(data, 3));
        if (ret != RET_SUCCESS) goto write_buff_end;

        ret = CALL(m_spi.write(buff, len));
        if (ret != RET_SUCCESS) goto write_buff_end;


//        if (block) {
//            data[2] |= _W5500_SPI_WRITE_;
//        }
//
//        for (int i = 0; i < len; i++) {
//            data[i + (block ? 3 : 2)] = *(buff + i);
//        }
//
//        ret = CALL(m_spi.write(data, block ? len + 3 : len + 2));
//        if (ret != RET_SUCCESS) goto write_buff_end;

        write_buff_end:
        CALL(m_gpio.set(1));
        RESET();
        return ret;
    }

    RetType get_socket_register_tx_wr(uint8_t socket, uint16_t *val) {
        RESUME();

        static uint8_t result = 0;
        RetType ret = CALL(read_reg(W5500_Sn_TX_WR(socket) << 8, &result));
        if (ret != RET_SUCCESS) goto get_socket_register_tx_wr_end;

        *val = result;

        ret = CALL(read_reg(W5500_Sn_TX_WR(W5500_WIZCHIP_OFFSET_INC(W5500_Sn_TX_WR(socket), 1)) << 8, &result));
        if (ret != RET_SUCCESS) goto get_socket_register_tx_wr_end;

        *val += result;

        get_socket_register_tx_wr_end:
        RESET();
        return ret;
    }

    RetType set_socket_register_tx_wr(uint8_t socket, uint16_t val) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_TX_WR(socket), static_cast<uint8_t>(val >> 8)));
        if (ret != RET_SUCCESS) goto set_socket_reg_tx_wr_end;

        ret = CALL(write_reg(W5500_WIZCHIP_OFFSET_INC(W5500_Sn_TX_WR(socket), 1), static_cast<uint8_t>(val)));

        set_socket_reg_tx_wr_end:
    RESET();
        return ret;
    }

    RetType get_socket_register_rx_rd(uint8_t socket, uint16_t *val) {
        RESUME();

        static uint8_t result = 0;
        RetType ret = CALL(read_reg(W5500_Sn_RX_RD(socket) << 8, &result));
        if (ret != RET_SUCCESS) goto get_socket_register_rx_rd_end;

        *val = result;

        ret = CALL(read_reg(W5500_Sn_TX_WR(W5500_WIZCHIP_OFFSET_INC(W5500_Sn_RX_RD(socket), 1)) << 8, &result));
        if (ret != RET_SUCCESS) goto get_socket_register_rx_rd_end;

        *val += result;

        get_socket_register_rx_rd_end:
    RESET();
        return ret;
    }

    RetType set_socket_register_rx_rd(uint8_t socket, uint16_t val) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_RX_RD(socket), static_cast<uint8_t>(val >> 8)));
        if (ret != RET_SUCCESS) goto set_socket_reg_rx_rd_end;

        ret = CALL(write_reg(W5500_WIZCHIP_OFFSET_INC(W5500_Sn_RX_RD(socket), 1), static_cast<uint8_t>(val)));

        set_socket_reg_rx_rd_end:
    RESET();
        return ret;
    }

    RetType set_socket_port(uint8_t socket, uint16_t port) {
        RESUME();

        RetType ret = CALL(write_reg(W5500_Sn_PORT(socket), static_cast<uint8_t>(port >> 8)));
        if (ret != RET_SUCCESS) goto set_socket_port_end;

        ret = CALL(write_reg(W5500_WIZCHIP_OFFSET_INC(W5500_Sn_PORT(socket), 1), static_cast<uint8_t>(port)));

        set_socket_port_end:
        RESET();
        return ret;
    }

    RetType reset() {
        RESUME();

        RetType ret = CALL(set_socket_mode_reg(W5500_MR, W5500_MR_RST));
        if (ret != RET_SUCCESS) goto reset_end;

        SLEEP(500);

        reset_end:
        RESET();
        return ret;
    }
};

#endif
