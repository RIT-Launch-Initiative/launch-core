#ifndef ARP_LAYER_H
#define ARP_LAYER_H

#include "net/common.h"
#include "net/network_layer/NetworkLayer.h"
#include "net/eth/eth.h"
#include "net/ipv4/ipv4.h"
#include "sched/macros.h"

using ipv4::IPv4Router;
using ipv4::IPv4Addr_t;

namespace arp {

/// @brief implements the ARP layer.
///
///      receiving:
///        Received ARP packets will be parsed and the IPv4 address will be
///        looked up in the IPv4 router. For ARP requests, if the device that
///        the address belongs too is the address that received the packet, a
///        response is sent with the devices MAC address. MAC addresses of
///        devices need to be manually added using the "add_dev" function.
///
///      lookup:
///        MAC addresses corresponding to an IPv4 address can be looked up. If
///        the address is already known it is returned immediately, if it is not
///        and ARP request is sent. This layer will handle blocking the calling
///        task until a response is received (or timeout).
///
///      transmit:
///        Transmits are invalid for the ARP layer.
class ArpLayer : public NetworkLayer {
public:
    /// @brief constructor
    /// @param timeout      the amount of time to wait for a reply to a request
    ///                     in units of system time
    ArpLayer(uint32_t timeout) : m_timeout(timeout) {};


    /// @brief add an Ethernet device to the ARP table
    ///        this tells the ARP table the MAC address of this device so valid
    ///        replies can be sent
    /// @param dev  the device
    /// @param mac  the mac address of the device
    /// @return
    RetType add_dev(NetworkLayer& eth_dev, uint8_t mac[6]) {
        mac_t addr;
        addr.addr[0] = mac[0];
        addr.addr[1] = mac[1];
        addr.addr[2] = mac[2];
        addr.addr[3] = mac[3];
        addr.addr[4] = mac[4];
        addr.addr[5] = mac[5];

        NetworkLayer** ptr = m_devs.add(addr)

        if(ptr == NULL) {
            // no room
            return RET_ERROR;
        }

        // set the device
        *ptr = &eth_dev;

        return RET_SUCCESS;
    }

    /// @brief lookup a MAC address in the ARP table
    /// @param addr     address to lookup
    /// @param mac      where the MAC will be copied to
    /// @return
    /// NOTE: this function may block in order to query
    RetType lookup(IPv4Addr_t addr, uint8_t mac[6]) {
        RESUME();

        mac_t* ptr = m_table[addr];

        if(ptr == NULL) {
            // uh oh, we don't have this address
            // we need to send a request and wait for it :(
            auto q_ptr = m_waiting[addr];
            if(q_ptr == NULL) {
                // we need to add this address
                q_ptr = m_waiting.add(addr);
            }

            if(q_ptr == NULL) {
                // no space
                return RET_ERROR;
            }

            if(!q_ptr->push(sched_dispatched)) {
                // couldn't add to queue
                return RET_ERROR;
            }

            // at this point the current task's tid is on a queue waiting for
            // the address to come in
            // (it's important to note this creates a race condition for received replies)
            // make sure to call receive outside of an ISR for this reason

            // TODO
            // send the ARP request for the address

            // sleep the task
            // NOTE: this relies on the fact that if we unblock something it is
            //       if taken off the sleep queue and added to the ready queue
            SLEEP(m_timeout);

            // try and get the pointer again
            // if we woke up without getting the address, ptr will be NULL
            ptr = m_table[addr];

            if(ptr == NULL) {
                return RET_ERROR;
            }
        }

        mac[0] = ptr->addr[0];
        mac[1] = ptr->addr[1];
        mac[2] = ptr->addr[2];
        mac[3] = ptr->addr[3];
        mac[4] = ptr->addr[4];
        mac[5] = ptr->addr[5];

        RESET();
        return RET_SUCCESS;
    }

    /// @brief receive an ARP packet
    /// @return
    RetType receive(Packet& packet, sockmsg_t& info, NetworkLayer* caller) {
        if(packet.available() < sizeof(ArpHeader_t)) {
            // not enough ARP data
            return RET_ERROR;
        }

        ArpHeader_t* hdr = packet.read_ptr<ArpHeader_t>();

        if(hdr == NULL) {
            return RET_ERROR;
        }

        if(ntoh16(hdr->htype) != ETH_HTYPE) {
            return RET_ERROR;
        }

        if(ntoh16(hdr->ptype) != IPV4_PTYPE) {
            return RET_ERROR;
        }

        if(hdr->hlen != 6) {
            return RET_ERROR;
        }

        if(hdr->plen != sizeof(IPv4Addr_t)) {
            return RET_ERROR;
        }

        switch(ntoh16(hdr->oper)) {
            case REQUEST_OPER:
                // TODO
                // check if the tpa is the address of the 'caller' device
                // if it is send a reply through caller
                // make sure to set the socket type to raw and fill in mac addr in sockaddr

                // get the target IP of the request
                IPv4Addr_t tip;
                ipv4::IPv4Address(hdr->tpa[0], hdr->tpa[1],hdr->tpa[2], hdr->tpa[3], &tip);

                // get the address of caller

                break;
            case REPLY_OPER:
                // TODO
                // add mapping to the table
                // check if this maps any addresses we're waiting on and wake up those tasks
                break;
            default:
                return RET_ERROR;
        }

        return RET_SUCCESS;
    }

    /// @brief transmit
    /// @return error, always invalid
    RetType transmit(Packet&, sockmsg_t&, NetworkLayer*) {
        return RET_ERROR;
    }

private:
    uint32_t m_timeout;

    typedef struct {
        uint8_t addr[6];
    } mac_t;

    // TODO hardcoding size!
    // maps IPv4 address to MAC addresses
    alloc::Hashmap<IPv4Addr_t, mac_t, 25, 25> m_table;

    // TODO hardcoding size!
    // maps MAC addresses to devices
    // TODO should map device to MAC + IPv4
    // use for requests + replies
    alloc::Hashmap<mac_t, NetworkLayer*, 25, 25> m_devs;

    // TODO hardcoding sizes!
    // maps IPv4 addresses to tasks waiting for those addresses
    alloc::Hashmap<IPv4Addr_t, alloc::Queue<tid_t, 10>, 25, 25> m_waiting;
};

} // namespace arp

#endif
