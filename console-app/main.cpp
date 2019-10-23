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
    return 0;
}
