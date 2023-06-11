/*******************************************************************************
*
*  Name: SlipRingDevice.h
*
*  Purpose: Implements a network layer that communicates using the Serial
*           Line Internet Protocol.
*
*           The serial devices should be chained in a ring, for example:
*
*           device 1 -> device 2 -> device 3 -> device 1
*
*           The TX of device 1 is connected to the RX of 2, the TX of 2 is
*           connected to the RX of 3, the TX of 3 is connected to the TX of 1.
*
*           Network layer (IPv4 typically) frames are encapsulated in custom
*           "ring" frames, which will be transmitted to each device in the ring
*           until every device has received the frame. This is accomplished very
*           simply with a TTL the size of the ring, the TTL is set to the ring
*           size minus 1 and decremented each receive, when it reaches zero at
*           the last device in the ring the frame is dropped to avoid routing
*           circles.
*
*           While the decision whether to drop or pass a frame on could be
*           handled at the network layer (e.g. IPv4 with routing), this is a
*           much simpler approach that allows for any network protocol to be
*           used and doesn't require thought on how to handle multicast traffic
*           and group management.
*
*  Author: Will Merges
*
*******************************************************************************/

#ifndef SLIP_RING_DEVICE_H
#define SLIP_RING_DEVICE_H

#include <stdlib.h>
#include <stdint.h>

#include "net/NetworkLayer.h"
#include "net/device/Device.h"
#include "net/device/StreamDevice.h"
#include "net/slip/slip.h"
#include "net/packet/Packet.h"


class SlipRingDevice : public NetworkLayer, public Device {
public:
    /// @brief header for a slip ring frame
    typedef struct {
        uint32_t ttl;
    } slip_ring_header_t;

    /// @brief initialize the device
    /// @return
    RetType init() {
        return RET_SUCCESS;
    }

    /// @brief obtain the device
    /// @return
    RetType obtain() {
        return RET_SUCCESS;
    }

    /// @brief release the device
    /// @return
    RetType release() {
        return RET_SUCCESS;
    }

    /// @brief poll the device
    RetType poll() {
        // TODO

        // grab new serial data
        // deecode
        // if got whole frame:
        //      send to net layer
        //      decrement TTL
        //      if TTL > 0:
        //          re-encode and transmit
        // repeat

        return RET_SUCCESS;
    }

    /// @brief transmit a packet over the SLIP ring
    /// @param packet   the packet to transmit
    /// @return
    RetType transmit(Packet& packet, netinfo_t&, NetworkLayer*) {
        // TODO

        // allocate header for slip ring frame
        // set TTL = ring length - 1

        return RET_SUCCESS
    }

    /// @brief transmit a packet over the SLIP ring (second pass)
    /// @param packet   the packet to transmit
    /// @return
    RetType transmit(Packet& packet, netinfo_t&, NetworkLayer*) {
        // TODO

        // encode into SLIP frame
        // transmit

        return RET_SUCCESS
    }

    /// @brief invalid
    RetType receive(Packet&, netinfo_t&, NetworkLayer*) {
        return RET_FAILURE;
    }

protected:
    /// @brief protected constructor
    /// @brief size     the number of devices in the ring
    /// @param serial   the serial device
    /// @param net      the network layer to pass received frames to
    /// @param packet   an allocated packet
    /// @param encoder  SLIP encoder
    /// @param decoder  SLIP decoder
    SlipRingDevice(size_t size,
                   StreamDevice& serial,
                   NetworkLayer& net,
                   Packet& packet,
                   UnallocatedSLIPEncoder& encoder,
                   UnallocatedSLIPDecoder& decoder,) : m_size(size),
                                                       m_serial(serial),
                                                       m_net(net),
                                                       m_packet(packet),
                                                       m_encoder(encoder),
                                                       m_decoder(decoder) {};

private:
    // number of devices in the ring
    size_t m_size;

    // serial device
    StreamDevice& m_stream;

    // network layer to pass packets up to
    NetworkLayer& net;

    // packet
    Packet& m_packet;

    // encoder
    UnallocatedSLIPEncoder& m_decoder;

    // decoder
    UnallocatedSLIPDecoder& m_encoder;
};

#endif
