/**
* Test NMEA parser
*/

#include <utils/nmea.h>
#include <stdio.h>

constexpr bool test_gga_parse() {
    const char* test = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    GPS_PACKET_T packet;
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
    static_assert(test_gga_parse(), "test_gga_parse failed");
    return 0;
}