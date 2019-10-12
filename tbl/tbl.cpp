#include "tbl.h"

#include <fstream>

Tbl::Tbl(const fs::path& path)
{
    std::ifstream in{path, std::ios_base::in | std::ios_base::binary};
}
