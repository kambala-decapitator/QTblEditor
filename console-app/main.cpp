#include "tbl.h"
#include "exceptions.h"

using std::cerr;
using std::cout;
using namespace std::string_literals;

int main(int argc, const char* argv[])
{
    args::ArgumentParser parser("CLI for Diablo 2 string files.");
    args::Group commands(parser, "commands");

    args::Group globalArgs(parser, "global arguments", args::Group::Validators::DontCare, args::Options::Global);
    args::PositionalList<fs::path> filePaths(globalArgs, "files", "Input files");
    [[maybe_unused]] args::HelpFlag help(globalArgs, {}, "Display this help menu", {'h', "help"});

    args::Command toPrint(commands, "print", "Print contents of files");
    args::Group printArgs(toPrint, "arguments");
    args::Flag rawNewlines(printArgs, {}, "Don't convert newlines to " + Tbl::foldedNewline, {"raw-newlines"});
    args::Flag rawColors(printArgs, {}, "Don't convert colors to human-readable strings", {"raw-colors"});

    args::Command toConvert(commands, "convert", "Convert between file types");
    args::Group convertArgs(toConvert, "arguments");
    args::ValueFlag<fs::path> outDir(convertArgs, "directory", "Directory where to save converted files, doesn't have to exist. Defaults to input file's directory.", {'o', "out-dir"});

    const auto wrapperArg = "wrap-str"s;
    std::string wrapperArgDesc;
    {
        auto reversedWrapperArg = wrapperArg;
        std::reverse(std::begin(reversedWrapperArg), std::end(reversedWrapperArg));
        wrapperArgDesc = "Wrap keys and values: [" + wrapperArg + "]<key>[" + reversedWrapperArg + "]<tab>[" + wrapperArg + "]<value>[" + reversedWrapperArg + ']';
    }
    args::ValueFlag<std::string> keyValueWrapper(convertArgs, wrapperArg, std::move(wrapperArgDesc), {"wrapper"});

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (const args::Help&)
    {
        cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        cerr << e.what() << '\n';
        cerr << parser;
        return 1;
    }
    catch (const args::ValidationError& e)
    {
        cerr << e.what() << '\n';
        cerr << parser;
        return 1;
    }

    auto files = args::get(filePaths);
    if (!commands.Matched() || files.empty())
    {
        parser.Help(cout);
        return 0;
    }

    try
    {
        auto file = files.at(0);
        Tbl t{file, !rawNewlines, !rawColors};
        if (toPrint)
        {
            for (const auto& entry : t)
                cout << entry.key << '\t' << entry.value << '\n';
        }
        if (toConvert)
        {
            fs::path outPath;
            if (outDir)
            {
                outPath = args::get(outDir);
                fs::create_directories(outPath);
                outPath /= file.filename();
            }
            else
                outPath = file;
            t.saveTxt(outPath.replace_extension(".txt"), args::get(keyValueWrapper));
            cout << "file saved to " << fs::absolute(outPath) << '\n';
        }

        return 0;
    }
    catch (const NoFileException& e)
    {
        cerr << "file doesn't exist\n";
    }
    catch (const TblReadException& e)
    {
        cerr << "couldn't read file\n";
    }
    catch (const TblTruncatedException& e)
    {
        cerr << "file size is incorrect: expected " << e.expectedFileSize << " - actual " << e.actualFileSize << '\n';
    }
    catch (const fs::filesystem_error& e)
    {
        cerr << "filesystem error: " << e.what() << '\n';
    }
    catch (const FileWriteException& e)
    {
        if (e.lastKey.empty())
            cerr << "couldn't open file for writing: " << e.path;
        else
            cerr << "couldn't write file " << e.path << " to end, last processed key: " << e.lastKey;
        cerr << '\n';
    }
    return 1;
}
