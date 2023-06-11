#include "net/slip/SLIPRingDevice.h"
#include "net/ipv4/IPv4Router.h"
#include "device/platforms/linux/DebugDevice.h"

int main() {
    LinuxDebugDevice serial;
    ipv4::IPv4Router router;
    alloc::SLIPRingDevice<1500> dev(3, serial, router);
}
