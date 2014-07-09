#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <dirent.h>

#include "../include/DataBase.h"

using namespace std;


const unsigned int LINE_BUFFER_SIZE = 1024;
const unsigned int MAX_FILE_LIST_SIZE = 512;
const unsigned int MAX_CYCLE_INFO_ARRAY_LENGTH = 256;


DataBase::DataBase(): dbValid(false)
{
    //ctor
}

DataBase::~DataBase()
{
    //dtor
}

bool DataBase::init(string useDir, string listFileName){
    dir = useDir;
    listFile = listFileName;
    bool loadListSuccessful = loadListFile();
    if(loadListSuccessful){
        addFileFromDir();
        dbValid = true;
        return true;
    }
    return false;
}


bool DataBase::getFileById(int id, FileData &fd){
    for(unsigned i=0; i<mdb.size(); i++){  // for each cycle
        for(unsigned j=0; j<mdb[i].cycleSize(); j++){ // for  each file
            if(mdb[i].fileDataVector[j].id==id){
                if(!mdb[i].valid)
                    return false;
                else{
                    fd = mdb[i].fileDataVector[j];
                    return true;
                }
            }
        }
    }
    return false;
}

bool DataBase::getCycle(double sCycle, CycleData& cd){
    for(unsigned i=0; i<mdb.size(); i++){ // search cycles
        if(mdb[i].cycle == sCycle){
            if(!mdb[i].valid)
                return false;
            else{
                cd = mdb[i];
                return true;
            }
        }
    }
    return false;
}

bool DataBase::getAllFileData(vector<FileData>& fdVector){
    fdVector.clear();
    for(unsigned i=0; i<mdb.size(); i++){ // search cycles
        if(mdb[i].valid){
            for(unsigned j=0; j<mdb[i].fileDataVector.size(); j++){
                fdVector.push_back(mdb[i].fileDataVector[j]);
            }
        }
    }
    if(fdVector.empty())
        return false;
    return true;
}

void DataBase::printCycleList(){
    cout << "Avaliable cycles on list: "  << endl;
    for(unsigned i=0; i<mdb.size(); i++){
        cout << mdb[i].cycle << " \t";
    }
    cout << endl;
}

bool DataBase::loadListFile()
{
    cout << "Load list file: " << listFile << " ... ";

    // open file
    ifstream inFile(listFile.c_str(), ios::in);
    if(!inFile) {
        cout << "Cannot open file!" << endl;
        return false;
    }
    cout << "successfully." << endl;

    // initialize
    char lineBuffer[LINE_BUFFER_SIZE];
    char *pch;
    mdb.clear();
    static int newIdCounter = 0; // give each selected file an unique new ID

    // discard first line (useless)
    inFile.getline(lineBuffer, LINE_BUFFER_SIZE);

    // read data (remaining lines)
    while(inFile.getline(lineBuffer, LINE_BUFFER_SIZE)) {

        CycleData cycleData;  // create a new element

        // split the line by comma (CSV file)
        pch = strtok (lineBuffer,",");
        cycleData.cycle = atof(pch);
        pch = strtok (NULL, ",");
        while(pch != NULL) {
            FileData fileData;
            fileData.fid = atof(pch);
            fileData.id = newIdCounter++;
            this->fileIdVector.push_back(fileData.fid);
            cycleData.fileDataVector.push_back(fileData);
            pch = strtok (NULL, ",");
        }
        mdb.push_back(cycleData); // Add to vector
    }
    inFile.close();
    return true;
}

bool DataBase::addFileFromDir()
{
    cout << "Adding files to database ... ";
    // Resolve file
    DIR *sDir;
    struct dirent *ent;
    if ((sDir = opendir (dir.c_str())) != NULL) {
        string curFileName;
        unsigned fidCounter = 0;
        while ((ent = readdir (sDir)) != NULL){
            curFileName = ent->d_name;
            if(curFileName.find("run")!=string::npos){  // find data file
                int curFid = atoi(curFileName.substr(3,6).c_str()); // get file serial number
                if(fileIdVector[fidCounter] == curFid){
                    fidCounter++;
                    fileNameVector.push_back(curFileName);
                }
            }
        }
    } else {
        // could not open directory
        cout << dir << " : ";
        perror ("");
        return false;
    }
    closedir (sDir);

    for(unsigned i=0; i<mdb.size(); i++){
        for(unsigned j=0; j<mdb[i].fileDataVector.size(); j++){
            mdb[i].fileDataVector[j].fileName = fileNameVector[mdb[i].fileDataVector[j].id];
        }
    }

    cout << "recognize " << fileIdVector.size() << " data files."  << endl;
}

bool DataBase::singleFileExtract(string fileName, FileData &fileData)
{
    cout << "FileData ID: " << fileData.id << "  extracting ... ";

    // open file
    ifstream inFile((dir+fileName).c_str(), ios::in);
    if(!inFile) {
        cout << "Error! Cannot open file: " << fileName << endl;
        return false;
    }

    char lineBuffer[LINE_BUFFER_SIZE];
    char *pch;

    // read first line (attribute type)
    inFile.getline(lineBuffer, LINE_BUFFER_SIZE);
    pch = strtok (lineBuffer,",");
    while (pch != NULL) {
        fileData.attrTypeVector.push_back(pch);
        pch = strtok (NULL, ",");
    }

    // read data by line
    fileData.dataVector.reserve(1024);
    vector<double> dataInLine(fileData.attrTypeVector.size()-1);
    while(inFile.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        // split the line by comma
        pch = strtok (lineBuffer,",");
        fileData.timeStamp.push_back(pch);
        pch = strtok (NULL, ",");
        for(unsigned i=0; i<fileData.attrTypeVector.size()-1; i++) {
            dataInLine[i]= atof(pch);
            pch = strtok (NULL, ",");
        }
        fileData.dataVector.push_back(dataInLine);
    }
    inFile.close();
    cout << " read " << fileData.dataVector.size() << " lines" << endl;
    return true;
}

bool DataBase::extract(double cycleBegin, double cycleEnd)
{
    // Error check
    double lowerBound = DBL_MAX, upperBound = -DBL_MAX;
    for(unsigned i=0; i<mdb.size(); i++){
        if(mdb[i].cycle > upperBound)
            upperBound = mdb[i].cycle;
        if(mdb[i].cycle < lowerBound)
            lowerBound = mdb[i].cycle;
    }
    if(cycleEnd < cycleBegin || cycleBegin < lowerBound || cycleEnd > upperBound){
        cout << "Cycle range error." << endl;
        return false;
    }

    // start file extraction
    unsigned totalDataQuantity = 0;
    unsigned cycleDataQuantity = 0;
    unsigned attributeSize = 0;
    unsigned stringLength = 0;
    for(unsigned i=0; i<mdb.size(); i++){ // for each cycle
        cycleDataQuantity = 0;
        if(mdb[i].cycle>=cycleBegin && mdb[i].cycle<=cycleEnd){  // for target range
            cout << "Extracting cycle " << mdb[i].cycle << " ... " << endl;
            vector<FileData> &fdVector = mdb[i].fileDataVector;
            bool extractSuccessful = false;
            for(unsigned j=0; j<mdb[i].fileDataVector.size(); j++){ // for each file
                extractSuccessful |= singleFileExtract(fdVector[j].fileName, fdVector[j]);
                cycleDataQuantity+= fdVector[j].dataVector.size();
            }
            if(!fdVector.front().dataVector.empty()){
                attributeSize = fdVector.front().attrSize();
                stringLength = fdVector.front().timeStamp.front().size();
            }
            mdb[i].valid = extractSuccessful;
            if(!extractSuccessful)
                return false;
            cout << "Load " << cycleDataQuantity << " lines (" << cycleDataQuantity*(attributeSize*sizeof(double)
                                +stringLength*sizeof(char))/1024.0/1024.0 << " MB)" << endl << endl;
            totalDataQuantity += cycleDataQuantity;
        }
        if(mdb[i].cycle>cycleEnd)
            break;
    }
    cout << endl << "Cycle extraction finished !" << endl;
    cout << "Totally load " << totalDataQuantity << " lines (" << totalDataQuantity*(attributeSize*sizeof(double)
                                +stringLength*sizeof(char))/1024.0/1024.0 << " MB)" << endl;
    return true;
}
