/**
 * Name: File.h
 * Purpose: Stores blocks of data
 * Author: Aaron Chan
 */

#ifndef LAUNCH_CORE_FILE_H
#define LAUNCH_CORE_FILE_H

namespace filesystem {
    typedef enum {
        READ_WRITE,
        READ,
        WRITE,
    } FILE_STATUS_T;

    class File {
    public:
        File() {
            this->permissions = READ_WRITE;
        }

        void changePermissions(FILE_STATUS_T status) {
            this->permissions = status;
        }


    private:
        FILE_STATUS_T permissions;
    };
}

#endif //LAUNCH_CORE_FILE_H
