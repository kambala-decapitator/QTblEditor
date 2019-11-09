#include "tbl.h"
#include "exceptions.h"

using namespace std;

int main(int argc, const char* argv[])
{
    args::ArgumentParser parser("CLI for Diablo 2 string files.");
    args::Group commands(parser, "commands");
    args::Command toPrint(commands, "print", "Print contents of files");
    args::Group arguments(parser, "arguments", args::Group::Validators::DontCare, args::Options::Global);
    args::PositionalList<fs::path> filePaths(arguments, "files", "Input files");
    [[maybe_unused]] args::HelpFlag help(arguments, "help", "Display this help menu", {'h', "help"});
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
    if (!commands.Matched() || files.empty())
    {
        parser.Help(cout);
        return 0;
    }

    try
    {
        Tbl t{files.at(0)};
        if (toPrint)
        {
            for (const auto& entry : t)
                std::cout << entry.key << '\t' << entry.value << endl;
        }
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
