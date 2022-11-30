/*******************************************************************************
*
*  Name: FileSystem.h
*
*  Purpose: Interface for a file system.
*
*  Author: Will Merges
*
*  RIT Launch Initiative
*
*******************************************************************************/
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stdlib.h>

#include "return.h"

/// @brief interface for a file system
class FileSystem {
public:
    /// @brief constructor
    FileSystem() {};

    /// @brief initialize
    virtual RetType init() = 0;

    /// @brief open a file
    ///        Creates a new file if there isn't one already
    /// @param filename     the name of the file
    /// @param new_file     set to true if a new file was created, false otherwise
    /// @return file descriptor, or -1 on error
    virtual int open(const char* filename, bool* new_file = NULL) = 0;

    /// @brief write to a file
    /// @param fd       descriptor to file to write to
    /// @param buff     data to write
    /// @param len      number of bytes to write
    /// @return
    virtual RetType write(int fd, uint8_t* buff, size_t len) = 0;

    /// @brief read from a file
    /// @param fd       descriptor to file to read from
    /// @param buff     buffer to read data into
    /// @param len      number of bytes to read
    /// @return
    virtual RetType read(int fd, uint8_t* buff, size_t len) = 0;

    /// @brief flush a file to persistent storage
    /// @param fd   descriptor to file to flush
    virtual RetType flush(int fd) = 0;

    /// @brief format the underlying device for this filesystem
    ///        NOTE: destructive operation!!!
    /// @return
    virtual RetType format() = 0;
};

#endif
