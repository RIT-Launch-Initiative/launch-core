#include "filesystem/FileSystem.h"
#include "device/platforms/linux/LinuxBlockDevice.h"


int main() {
    LinuxBlockDevice device = LinuxBlockDevice("file.txt", 512, 32);
    filesystem::FileSystem fs = filesystem::FileSystem(device);




    return 0;
}