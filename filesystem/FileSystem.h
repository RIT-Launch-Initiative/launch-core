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
//#include "File.h"

namespace filesystem {
    class FileSystem {
    public:
        FileSystem(LinuxBlockDevice device) : device(device) {}

        RetType readFile(int fileDescriptor, char* buffer, size_t bufferSize) {

            return RET_SUCCESS;
        }

        RetType writeFile(const char* buffer, size_t bufferSize) {

            return RET_SUCCESS;
        }

        RetType erase(int fileDescriptor) {
            return RET_SUCCESS;
        }

        int create(const char* filename) {

            return 0; // TODO: File Descriptor most likely
        }

        int open(const char* filename) {
            int file_descriptor = -1;
            if (file_descriptor > -1) { // TODO: If FNF
                file_descriptor = create(filename);
            }


            return file_descriptor; // TODO: File Descriptor most likely
        }

    private:
        LinuxBlockDevice device;
    };
}

#endif //LAUNCH_CORE_FILESYSTEM_H
