#include "tbl.h"
#include "exceptions.h"

#include <iostream>

using namespace std;

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        cerr << "tbl param missing" << endl;
        return 1;
    }

    try
    {
        Tbl t{fs::path{argv[1]}};
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
