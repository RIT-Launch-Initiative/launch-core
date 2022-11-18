#include "filesystem/FileSystem.h"
#include "device/platforms/linux/LinuxBlockDevice.h"
#include <stdint.h>


int main() {
    LinuxBlockDevice device = LinuxBlockDevice("file.txt", 512, 32);
    device.init();
    filesystem::FileSystem fs = filesystem::FileSystem(device);
    uint8_t buffer[] = {1, 2, 3, 4};
    fs.create();
//    fs.writeFile(0, buffer, 0);




    return 0;
}