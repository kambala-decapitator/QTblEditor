#pragma once

#include "filesystem.h"


class Tbl
{
public:
    Tbl() = default;
    Tbl(const fs::path& filename);
};

