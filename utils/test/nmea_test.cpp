/**
* Test NMEA parser
*/

// TODO: Write real tests

#include <utils/nmea.h>
#include <stdio.h>
#include <assert.h>
bool test_gga_parse() {
    const char* test = "$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0 0031*4F";
    nmea::GGA_PACKET_T packet;
    int ret = nmea::parse_gga(test, &packet, 100);

    return ret == 0 &&
        packet.time == 123519 &&
        packet.latitude == 48.1173 &&
        packet.longitude == 11.5167 &&
        packet.alt == 545.4 &&
        packet.quality == 1 &&
        packet.num_sats == 8;
}

int main() {
    assert(test_gga_parse());

    return 0;
}