#pragma once

#include "filesystem.h"

// header of a tbl file
struct TblHeader
{
    uint16_t CRC;             // +0x00 - CRC value for string table
    uint16_t NodesNumber;     // +0x02 - size of Indices array
    uint32_t HashTableSize;   // +0x04 - size of TblHashNode array
    uint8_t  Version;         // +0x08 - file version, either 0 or 1, doesn't matter
    uint32_t DataStartOffset; // +0x09 - string table start offset
    uint32_t HashMaxTries;    // +0x0D - max number of collisions for string key search based on its hash value
    uint32_t FileSize;        // +0x11 - size of the file
};

class Tbl
{
public:
    Tbl() = default;
    Tbl(const fs::path& filename);

private:
    void readHeader(std::ifstream& in);
};
