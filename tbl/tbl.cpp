#include "tbl.h"
#include "exceptions.h"

#include <fstream>
#include <iostream>

using std::ifstream;

template <typename T>
void readBinaryData(ifstream& in, T& data)
{
    in.read(reinterpret_cast<char*>(&data), sizeof(T));
}

Tbl::Tbl(const fs::path& path)
{
    ifstream in{path, std::ios_base::in | std::ios_base::binary};
    if (!in)
    {
        throw NoFileException{};
        return;
    }
    readHeader(in);
}

void Tbl::readHeader(ifstream& in)
{
    // TODO: big-endian
    TblHeader h;
    readBinaryData(in, h.crc);
    readBinaryData(in, h.nodesNumber);
    readBinaryData(in, h.hashTableSize);
    readBinaryData(in, h.version);
    readBinaryData(in, h.dataStartOffset);
    readBinaryData(in, h.hashMaxTries);
    readBinaryData(in, h.fileSize);
}
