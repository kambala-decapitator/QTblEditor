#include "tbl.h"
#include "exceptions.h"

#include <iostream>

using namespace std;

int main()
{
    try
    {
        Tbl t{fs::path{"/Volumes/hdd/games/d2/medianxl-rus/tbl/sigma/en/patchstring.tbl"}};
        cout << "Hello World! " << &t << endl;
    }
    catch (const NoFileException& e)
    {
        cerr << "file doesn't exist" << endl;
    }
    return 0;
}
