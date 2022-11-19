/**
 * Name: Filesystem.h
 * Purpose: Provide an interface managing how files are stored and retrieved
 * Author: Aaron Chan
 */

#ifndef LAUNCH_CORE_FILESYSTEM_H
#define LAUNCH_CORE_FILESYSTEM_H

#include "return.h"
#include "macros.h"
#include "device/BlockDevice.h"
#include "device/platforms/linux/LinuxBlockDevice.h"

#include "stdlib.h"
#include <stdint.h>
//#include "File.h"

namespace filesystem {
    class FileSystem {
    public:
        FileSystem(LinuxBlockDevice device) : device(device) {
            this->deviceBlockSize = device.getBlockSize();
            this->deviceNumBlocks = device.getNumBlocks();
        }

        RetType readFile(int fileDescriptor, char* buffer, size_t bufferSize) {

            return RET_SUCCESS;
        }

        RetType writeFile(int block_size, uint8_t* buffer, size_t bufferSize) {
            return RET_SUCCESS;
        }

        RetType erase(int fileDescriptor) {
            return RET_SUCCESS;
        }

        int create() {
            uint8_t * buffer = {};
            int i = 0;
            while (buffer != NULL) {
                this->device.read(i++, buffer);
                printf("NULL\n");
            }

            uint8_t data = -1;
            this->device.write(i, &data);
            return i;
        }

        int open(const char* filename) {
            int file_descriptor = -1;
            if (file_descriptor > -1) { // TODO: If FNF
//                file_descriptor = create(filename);
            }


            return file_descriptor; // TODO: File Descriptor most likely
        }

    private:
        LinuxBlockDevice device;
        size_t deviceBlockSize;
        size_t deviceNumBlocks;
        size_t keyCount = 0;
    };
}

#endif //LAUNCH_CORE_FILESYSTEM_H
