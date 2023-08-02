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

void test_gga_parse(GPSData *data) {
    const char* test = "$GPGGA,172814.0,3723.46587704,N,12202.26957864,W,2,6,1.2,18.893,M,-25.669,M,2.0 0031*4F";
    int ret = nmea::parse_gga(test, data, 100);

}

void test_blank_gga_parse(GPSData *data) {
    const char* test = "$GNGGA,,,,,,0,00,99.99,,,,,,*56";

    int ret = nmea::parse_gga(test, data, 100);
}

int main() {
    GPSData full_data;

    clock_t begin = clock();
    test_gga_parse(&full_data);
    clock_t end = clock();
    print_gga(full_data);
    printf("Spent %lu ticks parsing a full packet\n", (end - begin));

    GPSData blank_data;
    begin = clock();
    test_blank_gga_parse(&blank_data);
    end = clock();
    print_gga(blank_data);
    printf("Spent %lu ticks parsing a blank packet\n", (end - begin));
    return 0;
}