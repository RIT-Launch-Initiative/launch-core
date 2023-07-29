/**
 * Utilities for NMEA
 *
 * @author Yevgeniy Gorbachev
 * @author Aaron Chan
 */

#ifndef RADIO_MODULE_NMEA_H
#define RADIO_MODULE_NMEA_H

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include "common/MeasurementTypes.h"


namespace nmea {
    int parse_gga(const char *sentence, GPSData *dest, size_t n) {
        // check for terminator
        int i;
        for (i = 0; i < n; i++) {
            if (sentence[i] == '\0') {
                break;
            }
        }

        if (i == n) { // no terminator found before n
            return -1;
        }

        // everything after this is tested
        // initalizing buffers
        char north;
        char east;
        uint8_t hours;
        uint8_t minutes;
        uint8_t lat_deg;
        uint8_t lon_deg;
        float lat_min;
        float lon_min;

        // format includes all fields to validate that this is a GGA sentence
        int ret = sscanf(
                sentence,
                "$%*2cGGA,%2s%2s%f,%2s%f,%c,%3s%f,%c,%1d,%2d,%*f,%f,%*f,%*f,%*f,%*2c\r\n",
                &hours, &minutes, &(dest->time), // parts of time
                &lat_deg, &lat_min, &north, &lon_deg, &lon_min, &east, // parts of lat/lon
                &(dest->quality), &(dest->num_sats), &(dest->alt) // can be read directly
        );

        if (ret != 12) { // we want 12 fields
            return 1;
        }

        // convert lat/longe to decimal degrees, give sign according to cardinals
        dest->latitude = (lat_deg + lat_min / 60) * ((north == 'N') ? 1 : -1);
        dest->longitude = (lon_deg + lon_min / 60) * ((east == 'E') ? 1 : -1);
        // add hours and minutes as seconds
        // seconds were already read by sscanf
        dest->time += 3600 * hours + 60 * minutes;

        return 0;
    }
}


#endif //RADIO_MODULE_NMEA_H
