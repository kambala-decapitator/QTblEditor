#pragma once

#include "filesystem.h"

using std::ifstream;
using std::vector;

using HashTableIndex = uint16_t;
using StringOffset = uint32_t;


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

    static constexpr auto size = sizeof crc + sizeof nodesNumber + sizeof hashTableSize + sizeof version + sizeof dataStartOffset + sizeof hashMaxTries + sizeof fileSize;
};

// node of the hash table in a tbl file
struct TblHashNode
{
    uint8_t isUsed;           // +0x00 - shows if the entry is used: if 0, then it has been "deleted" from the table
    HashTableIndex index;     // +0x01 - index in the Indexes array
    uint32_t keyHash;         // +0x03 - hash value of the current string key
    StringOffset keyOffset;   // +0x07 - offset of the current string key
    StringOffset valueOffset; // +0x0B - offset of the current string value
    uint16_t valueLength;     // +0x0F - length of the current string value
};


struct TblEntry
{
    std::string key;
    std::string value;
    bool isUsed;
};

class Tbl
{
public:
    static const std::string foldedNewline;

public:
    Tbl() = default;
    Tbl(const fs::path& filename, bool convertNewlines = true, bool convertColors = true);

    using TblEntries = vector<TblEntry>;
    TblEntries::iterator begin() { return m_entries.begin(); }
    TblEntries::iterator end() { return m_entries.end(); }
    TblEntries::const_iterator cbegin() { return m_entries.cbegin(); }
    TblEntries::const_iterator cend() { return m_entries.cend(); }

private:
    TblHeader readHeader(ifstream& in);
    vector<HashTableIndex> readIndexes(ifstream& in, TblHeader& header);
    vector<TblHashNode> readNodes(ifstream& in, TblHeader& header);
    void readStringData(const char buf[], TblHeader& header, const vector<HashTableIndex>& indexes, const vector<TblHashNode>& nodes, bool convertNewlines, bool convertColors);

    static void foldNewlines(std::string& s);

private:
    TblEntries m_entries;
};
