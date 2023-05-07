/*******************************************************************************
*
*  Name: w5500_socket.h
*
*  Purpose: Contains definitions for socket structures used by the Wiznet W5500
*           device.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef W5500_SOCKET_H
#define W5500_SOCKET_H

#include <stdint.h>

#include "device/peripherals/W5500/W5500_defines.h"

/// @brief socket for a W5500
typedef enum {
    W5500_SOCKET0 = 0,
    W5500_SOCKET1,
    W5500_SOCKET2,
    W5500_SOCKET3,
    W5500_SOCKET4,
    W5500_SOCKET5,
    W5500_SOCKET6,
    W5500_SOCKET7,
    W5500_NUM_SOCKETS
} W5500Socket_t;

/// @brief socket descriptor
typedef struct {
    uint8_t reg;        // socket register block select address
    uint8_t tx_buff;    // transmit buffer block select address
    uint8_t rx_buff;    // receive buffer block select address
} W5500SocketDescriptor_t;

typedef enum {
    CLOSED_MODE = 0x00,
    TCP_MODE = 0x01,
    UDP_MODE = 0x02,
    IP_RAW_MODE = 0x02,
    MAC_RAW_MODE = 0x04
} W5500SocketMode_t;

typedef enum {
    OPEN_SOCKET = 0x01,
    LISTEN_SOCKET = 0x02,
    CONNECT_SOCKET = 0x04,
    DISCONNECT_SOCKET = 0x08,
    CLOSE_SOCKET = 0x10,

    SEND_SOCKET = 0x20,
    SEND_MAC_SOCKET = 0x21,
    SEND_KEEP_SOCKET = 0x22,

    RECV_SOCKET = 0x40
} W5500SocketCtrl_t;

/// @brief current state of a socket
typedef struct {
    bool send_ok;       // if the last transmit command has completed
    bool sending;       // there's currently a transmit operation taking place
    bool send_queued;   // there's a transmit that needs to be started
    bool rx_ready;      // if there is data ready to be received
    // TODO there are some other interrupt flags TCP needs,
    //      but we mostly care about UDP at the moment
} W5500SocketState_t;

/// @brief maps socket descriptors to sockets
static W5500SocketDescriptor_t W5500_SOCKET_MAP[W5500_NUM_SOCKETS] =
{
    {W5500_SOCKET0_REG, W5500_SOCKET0_TX_BUFF, W5500_SOCKET0_RX_BUFF},
    {W5500_SOCKET1_REG, W5500_SOCKET1_TX_BUFF, W5500_SOCKET1_RX_BUFF},
    {W5500_SOCKET2_REG, W5500_SOCKET2_TX_BUFF, W5500_SOCKET2_RX_BUFF},
    {W5500_SOCKET3_REG, W5500_SOCKET3_TX_BUFF, W5500_SOCKET3_RX_BUFF},
    {W5500_SOCKET4_REG, W5500_SOCKET4_TX_BUFF, W5500_SOCKET4_RX_BUFF},
    {W5500_SOCKET5_REG, W5500_SOCKET5_TX_BUFF, W5500_SOCKET5_RX_BUFF},
    {W5500_SOCKET6_REG, W5500_SOCKET6_TX_BUFF, W5500_SOCKET6_RX_BUFF},
    {W5500_SOCKET7_REG, W5500_SOCKET7_TX_BUFF, W5500_SOCKET7_RX_BUFF}
};

#endif
