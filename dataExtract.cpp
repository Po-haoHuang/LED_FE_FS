#include <cstdlib>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>

#include "include/FileData.h"
#include "include/DataBase.h"

using namespace std;


const string cycleListFileName = "use_file_list.csv";
const string dataDir = ".\\dp_variable_selection\\";
const string dataSelectionDir = ".\\dp_variable_selection\\selection\\";

int main(int argc, char *argv[])
{
    // database
    DataBase db;

    if(argc==2 && strcmp(argv[1],"init")==0){
        db.copyToSelection(dataDir, dataSelectionDir, cycleListFileName);  // copy listed files to run directory
    }

    db.init(dataSelectionDir, cycleListFileName);   // use filtered files directory as working directory

    db.printList();

    // Set cycle range
    double cycleBegin, cycleEnd;
    cout << "set cycle begin: ";
    cin >> cycleBegin;
    cout << "set cycle end: ";
    cin >> cycleEnd;
    bool extractSuccess = db.extract(cycleBegin, cycleEnd); // start extracting file data

    if(!extractSuccess){
        cout << "Extracting failed." << endl;
        return 1;
    }

    // usage example
    int fid;
    cout << "Select file id: ";
    cin >> fid;
    FileData fd;
    bool getFileDataSuccessful = db.getFileById(fid, fd);
    if(getFileDataSuccessful){
        cout << "fileName = \"" << fd.fileName << "\"" << endl;
        cout << "originalFileId = " << fd.originalFileId << endl;
        cout << "id = " << fd.id << endl;
        cout << fd.dataVector.size() << endl;
        for(unsigned i=0; i<5; i++){ // for first 5 lines
            cout << fd.timeStamp[i] << endl;
            for(unsigned j=0; j<fd.dataVector[i].size(); j++){
                cout << fd.dataVector[i][j] << " ";
            }
            cout << endl;
        }
    }else{
        cout << "Unable to get " << fid << " FileData" << endl;
    }
    return 0;
}
