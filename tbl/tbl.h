#ifndef TBL_H
#define TBL_H

#ifdef __APPLE__
 #include <AvailabilityMacros.h>
 #ifndef MAC_OS_X_VERSION_10_15
  #define MAC_OS_X_VERSION_10_15 101500
 #endif
 #if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_15 || MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_15
  #define USE_GHC_DYNAMIC_HEADER
 #endif
#else
 #define USE_GHC_DYNAMIC_HEADER
#endif

#ifdef USE_GHC_DYNAMIC_HEADER
#include <ghc/fs_std.hpp>
#else
#include <ghc/filesystem.hpp>
namespace fs {
using namespace ghc::filesystem;
using ifstream = ghc::filesystem::ifstream;
using ofstream = ghc::filesystem::ofstream;
using fstream = ghc::filesystem::fstream;
}
#endif

class Tbl
{
public:
    Tbl() = default;
    Tbl(const fs::path& filename);
};

#endif // TBL_H
