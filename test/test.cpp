#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "tbl.h"
#include "exceptions.h"

TEST_CASE("constructors", "[tbl]")
{
    CHECK_NOTHROW(Tbl{});
    CHECK_THROWS_AS(Tbl{"foo.tbl"}, NoFileException);
    CHECK_THROWS_AS(Tbl{EMPTY_FILE_NAME}, TblReadException);
    CHECK_THROWS_MATCHES(Tbl{"wrong_size.tbl"}, TblTruncatedException, Catch::Predicate<TblTruncatedException>(
                             [](const TblTruncatedException& e) {
        return e.expectedFileSize == 77181 && e.actualFileSize == 22; }
    ));
}
