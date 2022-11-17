/**
 * Name: Filesystem.h
 * Purpose: Provide an interface managing how files are stored and retrieved
 * Author: Aaron Chan
 */

#ifndef LAUNCH_CORE_FILESYSTEM_H
#define LAUNCH_CORE_FILESYSTEM_H

#include "return.h"
#include "File.h"

namespace filesystem {
    class FileSystem {
        FileSystem() {}

        RetType readFile() {

            return RET_SUCCESS;
        }

        RetType writeFile() {

            return RET_SUCCESS;
        }

    };
}

#endif //LAUNCH_CORE_FILESYSTEM_H
