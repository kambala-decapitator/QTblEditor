#include "tbl.h"
#include "exceptions.h"

using std::cerr;
using std::cout;

int main(int argc, const char* argv[])
{
    args::ArgumentParser parser("CLI for Diablo 2 string files.");
    args::Group commands(parser, "commands");

    args::Group globalArgs(parser, "global arguments", args::Group::Validators::DontCare, args::Options::Global);
    args::PositionalList<fs::path> filePaths(globalArgs, "files", "Input files");
    [[maybe_unused]] args::HelpFlag help(globalArgs, "help", "Display this help menu", {'h', "help"});

    args::Command toPrint(commands, "print", "Print contents of files");
    args::Group printArgs(toPrint, "arguments");
    args::Flag rawNewlines(printArgs, "raw-newlines", "Don't convert newlines to " + Tbl::foldedNewline, {"raw-newlines"});
    args::Flag rawColors(printArgs, "raw-colors", "Don't convert colors to human-readable strings", {"raw-colors"});

    args::Command toConvert(commands, "convert", "Convert between file types");

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
            t.saveTxt(file.replace_extension(".txt"));

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
