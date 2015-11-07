/*
Copyright (c) by Alexander Trufanov. Released under a Simplified BSD
license as described in the file LICENSE.
 */
#ifndef METADATA_H
#define METADATA_H

#include <cstdlib>

struct Block
{
    size_t offset_read;
    size_t length;
    size_t offset_write;
    Block(size_t pOffsetR = 0, size_t pLength = 0, size_t pOffsetWr = 0) : offset_read(pOffsetR),
        length(pLength), offset_write(pOffsetWr) {}

    inline bool operator < ( const Block& b) const
    {
        return offset_read < b.offset_read;
    }
    static bool sort_by_write_offset (Block a, Block b) { return a.offset_write < b.offset_write; }
};

#endif // METADATA_H
