#pragma once

class TblException {};

class NoFileException : public TblException {};

class TblReadException : public TblException {};
