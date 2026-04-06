#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime> 
#include <set>
#include <map>
#include <algorithm>

#include "fm.h"

using namespace std;


int main(int argc, char *argv[]) {
    cout << "I'm going to implement FM algorithm" << endl;
    if (argc < 2) {
        cerr << "Error: No input file\n";
        return 1;
    }
    ifstream InputFile(argv[1]);
    if (!InputFile.is_open()) {
        cerr << "Error: Unable to open file " << argv[1] << "\n";
        return 1;
    }
    string OutputFile = argv[2];

    FM fm(InputFile);
    fm.Run();
    fm.OutputResult(OutputFile);

    cout << "Finish FM" << endl;


    

    return 0;
}