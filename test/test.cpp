#undef TWOBLUECUBES_SINGLE_INCLUDE_CATCH_HPP_INCLUDED
#define CATCH_CONFIG_IMPL_ONLY
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "tbl.h"
#include "exceptions.h"
#include "d2color.h"

#include <codecvt>

using namespace std::string_literals;

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

TEST_CASE("colors", "[D2Color]")
{
    using namespace D2Color;

    CHECK(header == std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u"\u00FF\u0063"));
    CHECK(header == u8"ÿc");

    CHECK(readableColors.size() == Purple - White + 1);

    CHECK(readableColor(Red).second == R"(\red;)");
    CHECK_THROWS_AS(readableColor(White - 1), std::out_of_range);
    CHECK_THROWS_AS(readableColor(Purple + 1), std::out_of_range);

    auto rawColor = [](ColorCode code){ return header + static_cast<char>(code); };

    auto s = rawColor(White) + "test" + rawColor(Red) + "qwe";
    convertToReadableColors(s);
    CHECK(s == readableColor(White).second + "test" + readableColor(Red).second + "qwe");

    s = "asd" + header + "qwe" + rawColor(Blue);
    convertToReadableColors(s);
    CHECK(s == "asd" + header + "qwe" + readableColor(Blue).second);
}

int main(int argc, const char* argv[])
{
    return Catch::Session().run(argc, argv);
}
