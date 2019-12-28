#pragma once

#include "../common/lib_shared.h"

#ifdef TBL_LIBRARY
#define TBL_EXPORT SHARED_DECL_EXPORT
#else
#define TBL_EXPORT SHARED_DECL_IMPORT
#endif
