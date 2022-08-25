#ifndef W5500_SOCKET_H
#define W5500_SOCKET_H

#include <stdint.h>

#include "device/peripherals/w550/w5500_defines.h"

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

/// @brief socket mode
// TODO only UDP is supported at the moment
typedef enum {
    UDP_MODE
} W550SocketMode_t;

/// @brief current state of a socket
typedef struct {
    bool send_okay;     // if the last transmit command has completed
    bool rx_ready;      // if there is data ready to be received
    // TODO there are some other interrupt flags TCP needs,
    //      but we mostly care about UDP at the moment
} W5500SocketState_t;

/// @brief maps socket descriptors to sockets
static W5500SocketDescriptor_t W5500_SOCKET_MAP[W5500_NUM_SOCKETS] =
{
    {W5500_SOCKET0_REG, W5500_SOCKET0_TX_BUFF, W5500, SOCKET0_RX_BUFF},
    {W5500_SOCKET1_REG, W5500_SOCKET1_TX_BUFF, W5500, SOCKET1_RX_BUFF},
    {W5500_SOCKET2_REG, W5500_SOCKET2_TX_BUFF, W5500, SOCKET2_RX_BUFF},
    {W5500_SOCKET3_REG, W5500_SOCKET3_TX_BUFF, W5500, SOCKET3_RX_BUFF},
    {W5500_SOCKET4_REG, W5500_SOCKET4_TX_BUFF, W5500, SOCKET4_RX_BUFF},
    {W5500_SOCKET5_REG, W5500_SOCKET5_TX_BUFF, W5500, SOCKET5_RX_BUFF},
    {W5500_SOCKET6_REG, W5500_SOCKET6_TX_BUFF, W5500, SOCKET6_RX_BUFF},
    {W5500_SOCKET7_REG, W5500_SOCKET7_TX_BUFF, W5500, SOCKET7_RX_BUFF}
};

#endif
