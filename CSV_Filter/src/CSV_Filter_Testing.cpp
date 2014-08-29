#include <iostream>
#include <fstream>
#include "..\include\CSV_Filter.h"

using namespace std;

int main(int argc, char *argv[])
{
    if(argc!=5){
        cerr << "Input 4 arguments." << endl;
        return 1;
    }
    CSV_Filter csvFilter(argv[1]);
    csvFilter.use_list_file(argv[2]);
    csvFilter.exclude_list_file(argv[3]);
    ofstream outFile(argv[4]);

    // Warning: It has some bug, may read less column than expected.

    // title
    vector<string> usedTitle;
    csvFilter.get_title(usedTitle);
    for(unsigned i=0; i<usedTitle.size(); i++){
        outFile << usedTitle[i] << ",";
    }
    outFile << endl;

    // data
    vector<vector<double> > usedData;
    csvFilter.get_data(usedData);
    for(unsigned i=0; i<usedData.size(); i++){
        for(unsigned j=0; j<usedData[i].size(); j++){
            outFile << usedData[i][j] << ",";
        }
        outFile << endl;
    }
    return 0;
}
