#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "tbl.h"
#include "exceptions.h"

TEST_CASE("constructors", "[tbl]")
{
    CHECK_NOTHROW(Tbl{});
    CHECK_THROWS_AS(Tbl{"foo.tbl"}, NoFileException);
}
