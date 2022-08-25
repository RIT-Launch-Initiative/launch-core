/*
*   Implementation of a Wiznet W5500 Ethernet chip driver.
*   Written to be platform independent, it requires a "StreamDevice" that
*   handles SPI communication with the device and a "GPIODevice" that controls
*   setting the CS pin.
*
*   Currently, the driver only works for variable length data mode, so the CS
*   pin must be used and cannot be tied to ground like for fixed length data mode.
*
*/
#ifndef W5500_H
#define W5500_H

#include <stdlib.h>
#include <stdint.h>

#include "device/peripherals/w5500/w5500_defines.h"
#include "device/peripherals/w5500/w5500_socket.h"
#include "device/StreamDevice.h"
#include "sched/macros.h"
#include "return.h"

/// @brief controller for W5500 device
class W5500 {
public:
    /// @brief constructor
    /// @param spi      SPI controller device
    // TODO pass in GPIO for CS
    W5500(RegisterDevice& spi) : m_spi(spi) {};

    /// @brief initialize the device
    /// @return
    RetType init() {
        // TODO
        return RET_SUCCESS;
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
    /// @param sock     the socket to open
    /// @param mode     the mode to open it in
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

    /// @brief process interrupts
    ///        this can be triggered by the interrupt line or continuously polled
    /// @return
    RetType process_int() {
        // TODO
        return RET_SUCCESS;
    }

    /// @brief register a task to be woken up when this socket updates
    /// NOTE: there can be only one task registered, the new one overwrites the old one
    /// @param sock     the socket the task should wait for
    /// @param task     the task to register
    void register_task(W5500Socket_t sock, tid_t task) {
        // TODO
    }

    /// @brief transmit a packet over a socket
    /// @param sock     the socket to transmit on
    /// @param buff     the packet to send
    /// @param len      the length of the packet
    /// @return
    RetType transmit(W5500Socket_t sock, uint8_t* buff, size_t len) {
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
    RetType receive(W5500Socket_t sock, uint8_t* buff, size_t len, size_t* read) {
        // TODO
        return RET_SUCCESS;
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
    RetType read_bytes(uint8_t block_addr, uint16_t offset_addr, uint8_t* data, size_t len) {
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
    StreamDevice& m_spi;
};

#endif
