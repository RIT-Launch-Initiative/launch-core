/*******************************************************************************
*
*  Name: NetworkStatistics.h
*
*  Purpose: Contains interface for a network layer that collects statistics.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef NETWORK_STATISTICS_H
#define NETWORK_STATISTICS_H

#include <stdint.h>
#include "config.h"


#ifdef NET_STATISTICS

class NetworkStatistics {
public:
    /// @brief constructor
    NetworkStatistics() : OutgoingPackets(0),
                          IncomingPackets(0),
                          DroppedOutgoingPackets(0),
                          DroppedIncomingPackets(0) {};

    // number of packets successfully sent out of this layer
    uint32_t OutgoingPackets;

    // number of packets successfully received in this layer
    uint32_t IncomingPackets;

    // number of outgoing packets that were dropped at this layer
    uint32_t DroppedOutgoingPackets;

    // number of incoming packetts that were dropped at this layer
    uint32_t DroppedIncomingPackets;
};

#endif


#endif
