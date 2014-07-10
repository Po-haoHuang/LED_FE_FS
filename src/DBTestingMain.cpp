#include <cstdlib>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>

#include "../include/FileData.h"
#include "../include/DataBase.h"
#include "../include/CycleData.h"

using namespace std;


const string cycleListFileName = "use_file_list.csv";
const string dataDir = ".\\dp_variable_selection\\";

int main(int argc, char *argv[])
{
    // database
    DataBase db;

    db.init(dataDir, cycleListFileName);   // use filtered files directory as working directory

    if(!db.valid()){
        cout << "Database initializing failed." << endl;
        return 1;
    }

    db.printCycleList();

    // Set cycle range
    double cycleBegin, cycleEnd;
    cout << endl << "Set cycle begin: ";
    cin >> cycleBegin;
    cout << "Set cycle end: ";
    cin >> cycleEnd;
    cout << endl;
    bool extractSuccess = db.extract(cycleBegin, cycleEnd); // start extracting file data

    if(!extractSuccess){
        cout << "Extracting failed." << endl;
        return 1;
    }

    // get all file data example
    vector<FileData> fileDataVector;
    bool getAllFileSuccessful = db.getAllFileData(fileDataVector);
    if(getAllFileSuccessful){
        cout << endl << "There are " << fileDataVector.size() << " files extracted:";
        cout << ", containing: " << endl;
        for(unsigned i = 0; i< fileDataVector.size(); i++){
            cout << fileDataVector[i].id << " \t";
        }

        FileData &fd = fileDataVector[0];
        cout << endl << endl << "First extracted file: " << endl;
        cout << "id: " << fd.id << endl;
        cout << "filename: " << fd.fileName << endl;
        for(unsigned i=0; i<fd.attrTypeVector.size(); i++){
            cout << fd.attrTypeVector[i] << "  ";
        }
        cout << endl << "First 5 lines: " << endl;
        for(unsigned i=0; i<5; i++){
            for(unsigned j=0; j<fd.attrSize(); j++){
                cout << fd.dataVector[i][j] << "  ";
            }
            cout << endl;
        }
        cout << endl;
    }else{
        cout << "No file extracted." << endl;
    }


    return 0;

    // Cycle Data usage example
    /*double cid;
    cout << endl << "Select cycle id: ";
    cin >> cid;
    CycleData cd;
    bool getCycleDataSuccessful = db.getCycle(cid, cd);
    if(getCycleDataSuccessful){
        cout << "There are " << cd.fileDataVector.size() << " files in cycle " << cid << ":" << endl;
        for(unsigned i = 0; i< cd.fileDataVector.size(); i++){
            cout << cd.fileDataVector[i].id << " \t";
        }
    }else{
        cout << "Get cycle " << cid << " failed." << endl;
    }*/

    // FileData usage example
    int fid;
    cout << endl <<  "Select file id: ";
    cin >> fid;
    FileData fd;
    bool getFileDataSuccessful = db.getFileById(fid, fd);
    if(getFileDataSuccessful){
        cout << "fileName = \"" << fd.fileName << "\"" << endl;
        cout << "fid = " << fd.fid << endl;
        cout << "id = " << fd.id << endl;
        cout << "total lines = " << fd.dataVector.size() << endl;
        cout << "First 5 lines = " << endl;
        for(unsigned i=0; i<5; i++){ // for first 5 lines
            cout << fd.timeStamp[i] << endl;
            for(unsigned j=0; j<fd.dataVector[i].size(); j++){
                cout << fd.dataVector[i][j] << " ";
            }
            cout << endl;
        }
    }else{
        cout << "Unable to get id = " << fid << " FileData" << endl;
    }
    return 0;
}
