cmake_minimum_required(VERSION 3.14)
project(launch_core)


set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(device)
include_directories(hashmap)
include_directories(pool)
include_directories(queue)
include_directories(ringbuffer)
include_directories(sched)
include_directories(spinlock)
include_directories(string)

add_library(launch_core
        device/BlockDevice.h
        device/Device.h
        device/RegisterDevice.h
        device/SocketDevice.h
        device/StreamDevice.h

        hashmap/hashmap.h
        pool/test/test.cpp
        pool/pool.h
        queue/allocated_queue.h
        queue/queue.h

        ringbuffer/ringbuffer.h

        sched/macros.h
        sched/sched.h
        spinlock/spinlock.h
        string/lstring.h
        return.h
        )


add_executable(preprocessor_test sched/test/preprocessor_test.cpp)

add_executable(sched_basic_test sched/test/test_basic.cpp)

add_executable(hashmap_test hashmap/test/test.cpp)

add_executable(queue_test queue/test/test.cpp)

add_executable(ringbuffer_test ringbuffer/test/test.cpp)

add_executable(sched_test sched/sched.cpp)

add_executable(string_test string/test/string_test.cpp)




