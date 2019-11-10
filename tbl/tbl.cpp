#include "tbl.h"
#include "exceptions.h"
#include "d2color.h"

namespace
{
template <typename T>
void readBinaryData(ifstream& in, T& data)
{
    if (!in.good())
        throw TblReadException{};
    // TODO: big-endian
    in.read(reinterpret_cast<char*>(&data), sizeof(T));
}
}

const std::string Tbl::foldedNewline{"\\n"};

Tbl::Tbl(const fs::path& path, bool convertNewlines, bool convertColors)
{
    ifstream in{path, std::ios_base::in | std::ios_base::binary};
    if (!in)
        throw NoFileException{};

    try
    {
        auto header = readHeader(in);
        if (auto fileSize = fs::file_size(path); header.fileSize != fileSize)
            throw TblTruncatedException{header.fileSize, fileSize};

        auto indexes = readIndexes(in, header);
        auto nodes = readNodes(in, header);

        auto bufSize = header.fileSize - header.dataStartOffset;
        std::unique_ptr<char[]> buf{new char[bufSize]}; // TODO: std::byte / GSL
        auto rawBuf = buf.get();
        in.read(rawBuf, bufSize);
        in.close();

        readStringData(rawBuf, header, indexes, nodes, convertNewlines, convertColors);
    }
    catch (const std::ios::failure& e)
    {
        std::cerr << "error reading tbl file: " << e.what() << std::endl;
        throw TblReadException{};
    }
}

TblHeader Tbl::readHeader(ifstream& in)
{
    TblHeader header;
    readBinaryData(in, header.crc);
    readBinaryData(in, header.nodesNumber);
    readBinaryData(in, header.hashTableSize);
    readBinaryData(in, header.version);
    readBinaryData(in, header.dataStartOffset);
    readBinaryData(in, header.hashMaxTries);
    readBinaryData(in, header.fileSize);
    return header;
}

vector<HashTableIndex> Tbl::readIndexes(ifstream& in, TblHeader& header)
{
    vector<HashTableIndex> indexes;
    indexes.reserve(header.nodesNumber);
    for (decltype(header.nodesNumber) i = 0; i < header.nodesNumber; ++i)
    {
        HashTableIndex index;
        readBinaryData(in, index);
        indexes.push_back(index);
    }
    return indexes;
}

vector<TblHashNode> Tbl::readNodes(ifstream& in, TblHeader& header)
{
    vector<TblHashNode> nodes;
    nodes.reserve(header.hashTableSize);
    for (decltype(header.hashTableSize) i = 0; i < header.hashTableSize; ++i)
    {
        TblHashNode node;
        readBinaryData(in, node.isUsed);
        readBinaryData(in, node.index);
        readBinaryData(in, node.keyHash);
        readBinaryData(in, node.keyOffset);
        readBinaryData(in, node.valueOffset);
        readBinaryData(in, node.valueLength);
        nodes.push_back(std::move(node));
    }
    return nodes;
}

void Tbl::readStringData(const char buf[], TblHeader& header, const vector<HashTableIndex>& indexes, const vector<TblHashNode>& nodes, bool convertNewlines, bool convertColors)
{
    const auto startOffset = header.dataStartOffset;
    auto offset = [startOffset](StringOffset offset) { return offset - startOffset; };

    m_entries.reserve(header.hashTableSize);
    for (auto index : indexes)
    {
        const auto& node = nodes.at(index);
        std::string key{buf + offset(node.keyOffset)};
        std::string value{buf + offset(node.valueOffset), node.valueLength};

        if (convertNewlines)
        {
            foldNewlines(key);
            foldNewlines(value);
        }
        if (convertColors)
            D2Color::convertToReadableColors(value);

        m_entries.push_back({key, value, node.isUsed == 1});
    }
}

void Tbl::foldNewlines(std::string& s)
{
    const std::string newLine{'\n'};
    std::string::size_type pos = 0;
    for (;;)
    {
        pos = s.find(newLine, pos);
        if (pos == std::string::npos)
            return;
        s.replace(pos, newLine.length(), foldedNewline);
        pos += foldedNewline.length();
    }
}
