#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <dirent.h>

#include "../include/DataBase.h"
#include "../include/csv.h"

using namespace std;


const unsigned int LINE_BUFFER_SIZE = 65536;
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

bool DataBase::getCycleReal(double sCycle, CycleData& cd){
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

bool DataBase::getCycle(unsigned nCycle, CycleData& cd){
    if(nCycle > mdb.size())
        return false;

    cd = mdb[nCycle-1];
    return true;
}

// May exceed 2GB memory limit and crashed !
bool DataBase::getAllFileData(vector<FileData>& fdVector){
    for(unsigned i=0; i<mdb.size(); i++){
        if(mdb[i].valid){
            cout << "cycle " << i+1 << endl;
            for(unsigned j=0; j<mdb[i].fileDataVector.size(); j++){
                fdVector.push_back(mdb[i].fileDataVector[j]);
            }
            mdb[i].fileDataVector.clear();
            mdb[i].fileDataVector.shrink_to_fit();
        }
    }
    if(fdVector.empty())
        return false;
    return true;
}

void DataBase::printCycleList(){
    cout << "Avaliable cycles on list: "  << endl;
    for(unsigned i=0; i<mdb.size(); i++){
        cout << i+1 << "(" << mdb[i].cycle << ") " << "\t";
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
    cout << "recognize " << mdb.size() << " cycles." << endl;
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
    return true;
}

void DataBase::csvValueSplit(string s, const char delimiter, vector<double> &lineValue)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    while (end <= std::string::npos){
        lineValue.push_back(atof((s.substr(start, end-start)).c_str()));
	    if (end == std::string::npos)
	    	break;
    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
}

bool DataBase::getAllFileDataPtr(vector<FileData*>& fdPtrVector)
{
    for(unsigned i=0; i<mdb.size(); i++){
        if(mdb[i].valid){
            for(unsigned j=0; j<mdb[i].fileDataVector.size(); j++){
                fdPtrVector.push_back(&(mdb[i].fileDataVector[j]));
            }
        }
    }
    return true;
}

int DataBase::beginOfCycle(unsigned cycle)
{
    return mdb[cycle-1].fileDataVector.front().id;
}

int DataBase::endOfCycle(unsigned cycle)
{
    return mdb[cycle-1].fileDataVector.back().id+1;
}


bool DataBase::singleFileExtract(string fileName, FileData &fileData)
{
    //cout << "FileData ID: " << fileData.id << "  extracting ... ";

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

    fileData.dataVector.reserve(1024);

    // read data by line, 2 method implement

    #ifdef USE_FAST_CSV
    inFile.close();
    vector<double> lineValue(7);
    csvIO::CSVReader<8> in(dir+fileName);
    in.read_header(csvIO::ignore_extra_column, "DataTime", "dP_Filter (X1)", "Cp2Mg_1.source (F120)", "TMIn_2.press (P156)",
                    "RunHyd1.feed1 (F300)", "FilterConsumption (C6)", "ScrubberConsumption (C5)", "Position (P21)");
    string DateTime;
    while(in.read_row(DateTime,lineValue[0], lineValue[1], lineValue[2], lineValue[3], lineValue[4], lineValue[5], lineValue[6])) {
        //fileData.timeStamp.push_back(DateTime);
        fileData.dataVector.push_back(lineValue);
    }
    #endif

    #ifndef USE_FAST_CSV
    vector<double> lineValue;
    while(inFile.getline(lineBuffer, LINE_BUFFER_SIZE)){
        char *firstComma = strstr(lineBuffer, ",");
        //fileData.timeStamp.push_back(string(lineBuffer, firstComma-lineBuffer));
        lineValue.clear();
        csvValueSplit(string(firstComma+1), ',', lineValue);
        fileData.dataVector.push_back(lineValue);
    }
    inFile.close();
    #endif

    //cout << " read " << fileData.dataVector.size() << " lines" << endl;
    return true;
}

bool DataBase::extractById(int id, FileData &fileData)
{
    for(unsigned c=0; c<mdb.size(); c++){
        for(unsigned i=0; i<mdb[c].fileDataVector.size(); i++){
            if(id==mdb[c].fileDataVector[i].id){
                if(singleFileExtract(mdb[c].fileDataVector[i].fileName, fileData)){
                    fileData.id = mdb[c].fileDataVector[i].id;
                    fileData.fid = mdb[c].fileDataVector[i].fid;
                    return true;
                }
                else
                    return false;
            }
        }
    }
    return false;
}

bool DataBase::extract(unsigned cycleBegin, unsigned cycleEnd)
{
    // Error check
    double lowerBound = 1, upperBound = mdb.size();
    if(cycleEnd < cycleBegin || cycleBegin < lowerBound || cycleEnd > upperBound){
        cout << "Cycle range error." << endl;
        return false;
    }

    // start file extraction
    unsigned totalDataQuantity = 0;
    unsigned cycleDataQuantity = 0;
    unsigned attributeSize = 0;
    unsigned stringLength = 0;
    for(unsigned i=cycleBegin-1; i<=cycleEnd-1; i++){ // for each cycle
        cycleDataQuantity = 0;
        cout << "Extracting cycle " << i+1 << " ... " << endl;
        vector<FileData> &fdVector = mdb[i].fileDataVector;
        bool extractSuccessful = false;
        for(unsigned j=0; j<mdb[i].fileDataVector.size(); j++){ // for each file
            extractSuccessful |= singleFileExtract(fdVector[j].fileName, fdVector[j]);
            fdVector[j].cycle = mdb[i].cycle;
            fdVector[j].nCycle = i+1;
            mdb[i].nCycle = i+1;
            cycleDataQuantity+= fdVector[j].dataVector.size();
        }
        if(!fdVector.front().dataVector.empty()){
            attributeSize = fdVector.front().attrSize();
            //stringLength = fdVector.front().timeStamp.front().size();
        }
        mdb[i].valid = extractSuccessful;
        if(!extractSuccessful)
            return false;
        cout << "Load " << cycleDataQuantity << " lines (" << cycleDataQuantity*(attributeSize*sizeof(double)
                            +stringLength*sizeof(char))/1024.0/1024.0 << " MB)" << endl << endl;
        totalDataQuantity += cycleDataQuantity;
    }
    cout << endl << "Cycle extraction finished !" << endl;
    cout << "Totally load " << totalDataQuantity << " lines (" << totalDataQuantity*(attributeSize*sizeof(double)
                                +stringLength*sizeof(char))/1024.0/1024.0 << " MB)" << endl;
    return true;
}
