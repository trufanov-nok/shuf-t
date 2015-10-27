/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#ifndef METADATA_H
#define METADATA_H
#include <cstdlib>
struct Block
{
    size_t offset;
    size_t length;
    Block(size_t pOffset = 0, size_t pLength = 0) : offset(pOffset), length(pLength) {}
};

struct Block2Buf
{
    size_t offset_read;
    size_t offset_write;
    Block2Buf(size_t pOffsetRead = 0, size_t pOffsetWrite = 0): offset_read(pOffsetRead), offset_write(pOffsetWrite) {}

    inline bool operator < ( const Block2Buf& b) const
    {
        return offset_read < b.offset_read;
    }
};

#endif // METADATA_H
