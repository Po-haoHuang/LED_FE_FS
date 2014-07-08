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
    bool exListSuccessful = extractList();
    if(exListSuccessful){
        dbValid = true;
        return true;
    }
    return false;
}


bool DataBase::getFileById(int id, FileData &fd){
    for(unsigned i=0; i<mdb.size(); i++){  // for each cycle
        for(unsigned j=0; j<mdb[i].getCycleSize(); j++){ // for  each file
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

bool DataBase::getAllFileData(vector<CycleData>& cycleDataVector){

    for(unsigned i=0; i<mdb.size(); i++){ // search cycles
        if(mdb[i].valid){
            cycleDataVector.push_back(mdb[i]);
        }
    }
    if(cycleDataVector.empty())
        return false;
    return true;
}

void DataBase::printList(){
    cout << "Avaliable cycles on list: "  << endl;
    for(unsigned i=0; i<mdb.size(); i++){
        cout << mdb[i].cycle << " \t";
    }
    cout << endl;
}

int DataBase::getNextFileNo()
{
    static unsigned cycle = 0;
    static unsigned file = 0;
    int nextFileNo = -1;
    if(cycle < mdb.size()) {
        if(file >= mdb[cycle].fileDataVector.size()) {
            cycle++;
            file=0;
        }
        if(cycle < mdb.size())
            nextFileNo = mdb[cycle].fileDataVector[file].originalFileId;
        file++;
    }
    return nextFileNo;
}

bool DataBase::extractList()
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
            fileData.originalFileId = atof(pch);
            fileData.id = newIdCounter++;
            //fileData.fileName =
            cycleData.fileDataVector.push_back(fileData);
            pch = strtok (NULL, ",");
        }
        mdb.push_back(cycleData); // Add to vector
    }
    inFile.close();
    return true;
}

bool DataBase::init(string rawDataDir, string useDir, string listFileName){
    // search data directory
    cout << "Starting selection copy ... " << endl;
    DIR *dir;
    struct dirent *ent;
    int nextFileNo = getNextFileNo();
    int fileNameIndex = -1;
    char fileNameIndexStr[5];
    if ((dir = opendir (rawDataDir.c_str())) != NULL) {
        cout << "rawDataDir: " << rawDataDir << endl;
        mkdir(useDir.c_str());
        while ((ent = readdir (dir)) != NULL){
            if(strstr(ent->d_name, "run")==NULL) continue;  // exclude other file
            strncpy(fileNameIndexStr, ent->d_name+3,4); // get file serial number
            fileNameIndexStr[4]=0;
            fileNameIndex = atoi(fileNameIndexStr);
            // cout << "nextFileNo: " << nextFileNo << endl;
            if(fileNameIndex==nextFileNo){  // file id matched
                string cmd = "copy /y \"" + rawDataDir + ent->d_name + "\"";
                cmd += " \"" + useDir + ent->d_name + "\"" ;
                cout << cmd;
                system(cmd.c_str());
                nextFileNo = getNextFileNo();
            }
        }
    } else {
        // could not open directory
        cout << rawDataDir << " : ";
        perror ("");
        return false;
    }
    closedir (dir);
    init(useDir, listFileName);
    return true;
}

bool DataBase::singleFileExtract(string fileName, FileData &fileData)
{
    cout << "FileData ID: " << fileData.id << "  extracting ...";

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
        fileData.attributeTypeVector.push_back(pch);
        // cout << fileData.attributeTypeVector->back() << " \t";
        pch = strtok (NULL, ",");
    }

    // read data by line
    while(inFile.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        vector<double> dataInLine;
        dataInLine.reserve(8);
        // split the line by comma
        pch = strtok (lineBuffer,",");
        fileData.timeStamp.push_back(pch);
        pch = strtok (NULL, ",");
        for(unsigned i=0; i<fileData.attributeTypeVector.size()-1; i++) {
            dataInLine.push_back(atof(pch));
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
    // search data directory
    DIR *sDir;
    struct dirent *ent;
    int nextFileNo = getNextFileNo();
    int fileNameIndex = -1;
    char fileNameIndexStr[5];
    if ((sDir = opendir (dir.c_str())) != NULL) {
        while ((ent = readdir (sDir)) != NULL){
            if(strstr(ent->d_name, "run")==NULL) continue;  // exclude other file
            strncpy(fileNameIndexStr, ent->d_name+3,4); // get file serial number
            fileNameIndexStr[4]=0;
            fileNameIndex = atoi(fileNameIndexStr);
            // cout << "fileNameIndex: " << fileNameIndex << "  nextFileNo: " << nextFileNo << endl;
            if(fileNameIndex==nextFileNo){  // file id matched
                nextFileNo = getNextFileNo();
                for(unsigned i=0; i<mdb.size(); i++){  // search corresponding FileData
                    vector<FileData> &vec = mdb[i].fileDataVector;
                    for(unsigned j=0; j<vec.size(); j++){
                        if(vec[j].originalFileId==fileNameIndex){
                            vec[j].fileName = ent->d_name;
                            break;
                        }
                    }
                }
            }
        }
        closedir (sDir);
    } else {
        // could not open directory
        cout << dir.c_str() << " : ";
        perror ("");
        return false;
    }

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
        if(mdb[i].cycle>=cycleBegin && mdb[i].cycle<=cycleEnd){  // for target range
            cout << "Extracting cycle " << mdb[i].cycle << " ... " << endl;
            vector<FileData> &fdVector = mdb[i].fileDataVector;
            bool extractSuccessful = false;
            for(unsigned j=0; j<mdb[i].fileDataVector.size(); j++){ // for each file
                extractSuccessful |= singleFileExtract(fdVector[j].fileName.c_str(), fdVector[j]);
                cycleDataQuantity+= fdVector[j].dataVector.size();
            }
            mdb[i].valid = extractSuccessful;
            if(!extractSuccessful)
                return false;
            if(!fdVector.front().dataVector.empty()){
                attributeSize = fdVector.front().dataVector.front().size();
                stringLength = fdVector.front().timeStamp.front().size();
            }
            cout << "Load " << cycleDataQuantity*(attributeSize*sizeof(double)
                                +stringLength*sizeof(char))/1024.0/1024.0 << " MB" << endl << endl;
            totalDataQuantity += cycleDataQuantity;
        }
        if(mdb[i].cycle>cycleEnd)
            break;
    }
    cout << endl << "Cycle extraction finished !" << endl;
    cout << "Totally load " << totalDataQuantity*(attributeSize*sizeof(double)
                                +stringLength*sizeof(char))/1024.0/1024.0 << " MB" << endl;
    return true;
}
