#include "tbl.h"
#include "exceptions.h"
#include "d2color.h"

#include <cassert>

using std::ifstream;
using std::string;
using std::vector;

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

const string Tbl::foldedNewline{"\\n"};

Tbl::Tbl(const fs::path& path, bool convertNewlines, bool convertColors)
{
    ifstream in{path, std::ios::in | std::ios::binary};
    if (!in)
        throw NoFileException{};

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

void Tbl::saveTxt(const fs::path& filename)
{
    std::ofstream out{filename, std::ios::out | std::ios::trunc};
    if (!out)
        throw FileWriteException{filename.native()};
    for (const auto& entry : m_entries)
    {
        out << entry.key << '\t' << entry.value << '\n';
        if (!out.good())
            throw FileWriteException{filename.native(), entry.key};
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

void Tbl::readStringData(const char buf[], TblHeader& header, const vector<HashTableIndex>& indexes, const vector<TblHashNode>& nodes, bool convertNewlines, bool convertColors) noexcept
{
    const auto startOffset = header.dataStartOffset;
    auto offset = [startOffset](StringOffset offset) { return offset - startOffset; };

    m_entries.reserve(header.hashTableSize);
    for (auto index : indexes)
    {
        const auto& node = nodes.at(index);
        assert(node.valueLength > 0);

        string key{buf + offset(node.keyOffset)};
        string value{buf + offset(node.valueOffset), static_cast<string::size_type>(node.valueLength - 1)};

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

void Tbl::foldNewlines(string& s) noexcept
{
    const string newLine{'\n'};
    string::size_type pos = 0;
    for (;;)
    {
        pos = s.find(newLine, pos);
        if (pos == string::npos)
            return;
        s.replace(pos, newLine.length(), foldedNewline);
        pos += foldedNewline.length();
    }
}
