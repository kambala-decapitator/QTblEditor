#include "tbl.h"
#include "exceptions.h"

#include <iostream>
#include <args.hxx>

using namespace std;

int main(int argc, const char* argv[])
{
    args::ArgumentParser parser("CLI for Diablo 2 string files.");
    [[maybe_unused]] args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::PositionalList<fs::path> filePaths(parser, "files", "Input files");
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
        cerr << e.what() << endl;
        cerr << parser;
        return 1;
    }
    catch (const args::ValidationError& e)
    {
        cerr << e.what() << endl;
        cerr << parser;
        return 1;
    }

    auto files = args::get(filePaths);
    if (files.empty())
    {
        parser.Help(cout);
        return 0;
    }

    try
    {
        Tbl t{files.at(0)};
        cout << "Hello World! " << &t << endl;
    }
    catch (const NoFileException& e)
    {
        cerr << "file doesn't exist" << endl;
    }
    catch (const TblReadException& e)
    {
        cerr << "couldn't read file" << endl;
    }
    catch (const TblTruncatedException& e)
    {
        cerr << "file size is incorrect: expected " << e.expectedFileSize << " - actual " << e.actualFileSize << endl;
    }
    return 0;
}
