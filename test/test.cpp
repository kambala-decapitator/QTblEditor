#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "tbl.h"
#include "exceptions.h"

TEST_CASE("constructors", "[tbl]")
{
    auto testFilePath = [](const std::string& fileName){
        return fs::path{"data"} / fileName;
    };
    CHECK_NOTHROW(Tbl{});
    CHECK_THROWS_AS(Tbl{testFilePath("foo.tbl")}, NoFileException);
    CHECK_THROWS_AS(Tbl{testFilePath("empty.tbl")}, TblReadException);
    CHECK_THROWS_MATCHES(Tbl{testFilePath("wrong_size.tbl")}, TblTruncatedException, Catch::Predicate<TblTruncatedException>(
                             [](const TblTruncatedException& e) {
        return e.expectedFileSize == 77181 && e.actualFileSize == 22; }
    ));
}
