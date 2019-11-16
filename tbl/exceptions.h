#pragma once

#include <cstdint>

class TblException {};

class NoFileException : public TblException {};

class TblReadException : public TblException {};

class TblTruncatedException : public TblException
{
public:
    TblTruncatedException(uintmax_t expected, uintmax_t actual) : TblException(), expectedFileSize(expected), actualFileSize(actual) {}
    uintmax_t expectedFileSize, actualFileSize;
};


class FileWriteException : public TblException
{
public:
    FileWriteException(const std::string& path_, const std::string& key = {}) : TblException(), path(path_), lastKey(key) {}
    std::string path, lastKey;
};
