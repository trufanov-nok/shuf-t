#ifndef METADATA_H
#define METADATA_H
#include <QtGlobal>

struct Block
{
    qint64 offset;
    uint length;
    Block(qint64 pOffset = 0, uint pLength = 0) : offset(pOffset), length(pLength) {}
};

struct Block2Buf
{
    qint64 offset_read;
    qint64 offset_write;
    Block2Buf(qint64 pOffsetRead = 0, qint64 pOffsetWrite = 0): offset_read(pOffsetRead), offset_write(pOffsetWrite) {}

    inline bool operator < ( const Block2Buf& b) const
    {
        return offset_read < b.offset_read;
    }
};

#endif // METADATA_H
