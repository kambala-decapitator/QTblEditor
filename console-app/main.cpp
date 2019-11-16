#include "tbl.h"
#include "exceptions.h"

using namespace std;

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
        Tbl t{files.at(0), !rawNewlines, !rawColors};
        if (toPrint)
        {
            for (const auto& entry : t)
                cout << entry.key << '\t' << entry.value << '\n';
        }
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
    return 0;
}
