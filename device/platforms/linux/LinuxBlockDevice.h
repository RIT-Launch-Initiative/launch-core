/**
 * Name: LinuxBlockDevice.h
 *
 */

#ifndef LAUNCH_CORE_LINUXBLOCKDEVICE_H
#define LAUNCH_CORE_LINUXBLOCKDEVICE_H

#include "device/BlockDevice.h"
#include "hashmap/hashmap.h"
#include <iostream>
#include <fstream>

class LinuxBlockDevice : BlockDevice {
    LinuxBlockDevice(const char* name) : BlockDevice(name) {

        for (int i = 0; i < 33; i++) {
            std::ofstream* stream = fileStreams[i];
            stream = new std::ofstream();
        }
    };

    RetType write(size_t block, uint8_t* data) {

        return RET_SUCCESS;
    }

    RetType read(size_t block, uint8_t* buff) {

        return RET_SUCCESS;
    };

     size_t getBlockSize() {
         return 32;
     };

private:
    alloc::Hashmap<size_t, std::ofstream, 32, 32> fileStreams;

};


#endif //LAUNCH_CORE_LINUXBLOCKDEVICE_H
