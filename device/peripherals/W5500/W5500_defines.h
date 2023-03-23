/*******************************************************************************
*
*  Name: w5500_defines.h
*
*  Purpose: Contains preprocessor definitions for constants relating to the
*           Wiznet W5500 device.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef W5500_DEFINES_H
#define W5500_DEDINES_H

// *** bit masks for control phase byte *** //
// block select
#define W5500_BS_MASK               0b11111000
#define W5500_BS_SHIFT              3

// read/write access mode
#define W5500_RW_MASK               0b00000100
#define W5500_RW_SHIFT              2

// operation mode
#define W5500_OM_MASK               0b00000011
#define W5500_OM_SHIFT              0

#define W5500_CTRL_READ             0x00
#define W5500_CTRL_WRITE            0x04
// **************************************** //

// ********* block select values ********** //
// these values must be shifted by W5500_BS_SHIFT when actually used
// a block select is 5-bits total
// all undefined block selects are reserved

// common register
#define W5500_COMMON_REG            0b00000

// socket 0 register
#define W5500_SOCKET0_REG           0b00001
// socket 0 TX buffer
#define W5500_SOCKET0_TX_BUFF       0b00010
// socket 0 RX buffer
#define W5500_SOCKET0_RX_BUFF       0b00011

// socket 1 register
#define W5500_SOCKET1_REG           0b00101
// socket 1 TX buffer
#define W5500_SOCKET1_TX_BUFF       0b00110
// socket 1 RX buffer
#define W5500_SOCKET1_RX_BUFF       0b00111

// socket 2 register
#define W5500_SOCKET2_REG           0b01001
// socket 2 TX buffer
#define W5500_SOCKET2_TX_BUFF       0b01010
// socket 2 RX buffer
#define W5500_SOCKET2_RX_BUFF       0b01011

// socket 3 register
#define W5500_SOCKET3_REG           0b01101
// socket 3 TX buffer
#define W5500_SOCKET3_TX_BUFF       0b01110
// socket 3 RX buffer
#define W5500_SOCKET3_RX_BUFF       0b01111

// socket 4 register
#define W5500_SOCKET4_REG           0b10001
// socket 4 TX buffer
#define W5500_SOCKET4_TX_BUFF       0b10010
// socket 4 RX buffer
#define W5500_SOCKET4_RX_BUFF       0b10011

// socket 5 register
#define W5500_SOCKET5_REG           0b10101
// socket 5 TX buffer
#define W5500_SOCKET5_TX_BUFF       0b10110
// socket 5 RX buffer
#define W5500_SOCKET5_RX_BUFF       0b10111

// socket 6 register
#define W5500_SOCKET6_REG           0b11001
// socket 6 TX buffer
#define W5500_SOCKET6_TX_BUFF       0b11010
// socket 6 RX buffer
#define W5500_SOCKET6_RX_BUFF       0b11011

// socket 7 register
#define W5500_SOCKET7_REG           0b11101
// socket 7 TX buffer
#define W5500_SOCKET7_TX_BUFF       0b11110
// socket 7 RX buffer
#define W5500_SOCKET7_RX_BUFF       0b11111

// **************************************** //

// ******** read/write access bit ********* //

#define W5500_READ                  0
#define W5500_WRITE                 1

// **************************************** //

// ******** SPI operation mode bits ******* //

// variable data length
#define W5500_VDL_MODE              0b00

// fixed data length mode, 1 byte
#define W5500_FIXED1_MODE           0b01

// fixed data length mode, 2 bytes
#define W5500_FIXED2_MODE           0b10

// fixed data length mode, 4 bytes
#define W5500_FIXED4_MODE           0b11

// **************************************** //

// **** common register block offsets ***** //
// offset addresses are 16-bits
// registers are 8-bits

// mode
#define W5500_COMMON_MR             0x0000

// gateway address
#define W5500_COMMON_GAR0           0x0001
#define W5500_COMMON_GAR1           0x0002
#define W5500_COMMON_GAR2           0x0003
#define W5500_COMMON_GAR3           0x0004

// subnet mask address
#define W5500_COMMON_SUBR0          0x0005
#define W5500_COMMON_SUBR1          0x0006
#define W5500_COMMON_SUBR2          0x0007
#define W5500_COMMON_SUBR3          0x0008

// source hardware address
#define W5500_COMMON_SHAR0          0x0009
#define W5500_COMMON_SHAR1          0x000A
#define W5500_COMMON_SHAR2          0x000B
#define W5500_COMMON_SHAR3          0x000C
#define W5500_COMMON_SHAR4          0x000D
#define W5500_COMMON_SHAR5          0x000E

// source IP address
#define W5500_COMMON_SIPR0          0x000F
#define W5500_COMMON_SIPR1          0x0010
#define W5500_COMMON_SIPR2          0x0011
#define W5500_COMMON_SIPR3          0x0012

// interrupt low level timer
#define W5500_COMMON_INTLEVEL0      0x0013
#define W5500_COMMON_INTLEVEL1      0x0014

// interrupt
#define W5500_COMMON_IR             0x0015

// interrupt mask
#define W5500_COMMON_IMR            0x0016

// socket interrupt
#define W5500_COMMON_SIR            0x0017

// socket interrupt mask
#define W5500_COMMON_SIMR           0x0018

// retry time
#define W5500_COMMON_RTR0           0x0019
#define W5500_COMMON_RTR1           0x001A

// retry count
#define W5500_COMMON_RCR            0x001B

// PPP LCP Magic number
#define W5500_COMMON_PMAGIC         0x001D

// PPP Destination MAC Address
#define W5500_COMMON_PHAR0          0x001E
#define W5500_COMMON_PHAR1          0x001F
#define W5500_COMMON_PHAR2          0x0020
#define W5500_COMMON_PHAR3          0x0021
#define W5500_COMMON_PHAR4          0x0022
#define W5500_COMMON_PHAR5          0x0023

// PPP Session Identification
#define W5000_COMMON_PSID0          0x0024
#define W5000_COMMON_PSID1          0x0025

/// PPP Maximum Segment Size
#define W5500_COMMON_PMRU0          0x0026
#define W5500_COMMON_PMRU1          0x0027

// Unreachable IP address
#define W5500_COMMON_UIPR0          0x0028
#define W5500_COMMON_UIPR1          0x0029
#define W5500_COMMON_UIPR2          0x002A
#define W5500_COMMON_UIPR3          0x002B

// Unreachable Port
#define W5500_COMMON_UPORTR0        0x002C
#define W5500_COMMON_UPORTR1        0x002D

// PHY Configuration
#define W5500_COMMON_PHYCFGR        0x002E

// Chip version
#define W5500_COMMON_VERSIONR       0x0039

// **************************************** //

// **** socket register block offsets ***** //
// offset addresses are 16-bits
// registers are 8-bits

// mode
#define W5500_SOCKET_MR             0x0000

// command
#define W5500_SOCKET_CR             0x0001

// interrupt
#define W5500_SOCKET_IR             0x0002

// status
#define W5500_SOCKET_SR             0x0003

// source port
#define W5500_SOCKET_PORT0          0x0004
#define W5500_SOCKET_PORT1          0x0005

// destination hardware addr
#define W5500_SOCKET_DHAR0          0x0006
#define W5500_SOCKET_DHAR1          0x0007
#define W5500_SOCKET_DHAR2          0x0008
#define W5500_SOCKET_DHAR3          0x0009
#define W5500_SOCKET_DHAR4          0x000A
#define W5500_SOCKET_DHAR5          0x000B

// destination IP address
#define W5500_SOCKET_DIPR0          0x000C
#define W5500_SOCKET_DIPR1          0x000D
#define W5500_SOCKET_DIPR2          0x000E
#define W5500_SOCKET_DIPR3          0x000F

// destination port
#define W5500_SOCKET_DPORT0         0x0010
#define W5500_SOCKET_DPORT1         0x0011

// maximum segment size
#define W5500_SOCKET_MSSR0          0x0012
#define W5500_SOCKET_MSSR1          0x0013

// IP TOS
#define W5500_SOCKET_TOS            0x0015

// IP TTL
#define W5500_SOCKET_TTL            0x0016

// receive buffer size
#define W5500_SOCKET_RXBUF_SIZE     0x001E

// transmit buffer size
#define W5500_SOCKET_TXBUF_SIZE     0x001F

// TX Free Size
#define W5500_SOCKET_TX_FSR0        0x0020
#define W5500_SOCKET_TX_FSR1        0x0021

// TX read pointer
#define W5500_SOCKET_TX_RD0         0x0022
#define W5500_SOCKET_TX_RD1         0x0023

// TX write pointer
#define W5500_SOCKET_TX_WR0         0x0024
#define W5500_SOCKET_TX_WR1         0x0025

// RX received size
#define W5500_SOCKET_RSR0           0x0026
#define W5500_SOCKET_RSR1           0x0027

// RX Read Pointer
#define W5500_SOCKET_RX_RD0         0x0028
#define W5500_SOCKET_RX_RD1         0x0029

// RX Write Pointer
#define W5500_SOCKET_RX_WR0         0x002A
#define W5500_SOCKET_RX_WR1         0x002B

// Interrupt Mask
#define W5500_SOCKET_IMR            0x002C

// Fragment Offset in IP header
#define W5500_SOCKET_FRAG0          0x002D
#define W5500_SOCKET_FRAG1          0x002E

// Keep alive timer
#define W5500_SOCKET_KPALVTR        0x002F

// **************************************** //

#endif
