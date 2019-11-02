#pragma once

#include "filesystem.h"

// header of a tbl file
struct TblHeader
{
    uint16_t crc;             // +0x00 - CRC value for string table
    uint16_t nodesNumber;     // +0x02 - size of Indexes array
    uint32_t hashTableSize;   // +0x04 - size of TblHashNode array
    uint8_t  version;         // +0x08 - file version
    uint32_t dataStartOffset; // +0x09 - string table start offset
    uint32_t hashMaxTries;    // +0x0D - max number of collisions for string key search based on its hash value
    uint32_t fileSize;        // +0x11 - size of the file
};

class Tbl
{
public:
    Tbl() = default;
    Tbl(const fs::path& filename);

private:
    void readHeader(std::ifstream& in);
};
