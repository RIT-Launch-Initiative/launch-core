/**
* Test NMEA parser
*/

// TODO: Write real tests

#include <time.h>
#include "common/utils/nmea.h"
#include <stdio.h>
#include <assert.h>

void print_gga(GPSData data) {
    printf("time: %f\n", data.time);
    printf("lat: %f\n", data.latitude);
    printf("lon: %f\n", data.longitude);
    printf("alt: %f\n", data.alt);
    printf("quality: %d\n", data.quality);
    printf("num_sats: %d\n", data.num_sats);
    printf("\n");

}

void test_gga_parse() {
    const char* test = "$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0 0031*4F";
    GPSData data;
    int ret = nmea::parse_gga(test, &data, 100);

    print_gga(data);
}

void test_blank_gga_parse() {
    const char* test = "$GNGGA,,,,,,0,00,99.99,,,,,,*56";
    GPSData data;
    int ret = nmea::parse_gga(test, &data, 100);

    print_gga(data);
}

int main() {
    clock_t begin = clock();
    test_gga_parse();
    clock_t end = clock();

    printf("Spent %lu ticks parsing a full packet\n", (end - begin));

    begin = clock();
    test_blank_gga_parse();
    end = clock();
    printf("Spent %lu ticks parsing a blank packet\n", (end - begin));
    return 0;
}