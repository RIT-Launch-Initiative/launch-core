/*
*   Implementation of a Wiznet W5500 Ethernet chip driver.
*   Written to be platform independent, it requires a "RegisterDevice" that
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
#include "device/RegisterDevice.h"
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
    /// @return
    RetType open(W5500Socket_t sock) {
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
    // passed in SPI controller
    RegisterDevice& m_spi;
};

#endif
