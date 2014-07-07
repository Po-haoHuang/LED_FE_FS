#include <stdlib.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>

const int LINE_BUFFER_SIZE = 1024;
const int MAX_FILE_LIST_SIZE = 512;
const int MAX_CYCLE_INFO_ARRAY_LENGTH = 256;

using namespace std;

const string cycleListFileName = "use_file_list.csv";
const string dataDir = ".\\dp_variable_selection\\";
const string dataSelectionDir = ".\\dp_variable_selection\\selection\\";  // Windows only

struct FileData{
    int id;
    int originalFileId;
    string fileName;
    vector<vector<double> > dataVector;
    vector<string> timeStamp;
    vector<string> attributeTypeVector;
};

struct CycleData {
    double cycle;
    vector<FileData> fileDataVector;
};

FileData& getFileById(vector<CycleData> &cycleDataVector, int id){
    for(unsigned i=0; i<cycleDataVector.size(); i++){  // search corresponding FileData
        vector<FileData> &vec = cycleDataVector[i].fileDataVector;
        for(unsigned j=0; j<vec.size(); j++){
            if(vec[j].id==id){
                return vec[j];
                break;
            }
        }
    }
    return *(new FileData);
}

int getNextFileNo(vector<CycleData> &cycleDataVector)
{
    static unsigned cycle = 0;
    static unsigned file = 0;
    int nextFileNo = -1;
    if(cycle < cycleDataVector.size()) {
        if(file >= cycleDataVector[cycle].fileDataVector.size()) {
            cycle++;
            file=0;
        }
        if(cycle < cycleDataVector.size())
            nextFileNo = cycleDataVector[cycle].fileDataVector[file].originalFileId;
        file++;
    }
    return nextFileNo;
}

void extractList(const char *listFileName, vector<CycleData> &cycleDataVector)
{
    // open file
    ifstream inFile(listFileName, ios::in);
    if(!inFile) {
        printf("Cannot open file: %s\n",listFileName);
        exit(1);
    }

    // initialize
    char lineBuffer[LINE_BUFFER_SIZE];
    char *pch;
    cycleDataVector.clear();

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
            cycleData.fileDataVector.push_back(fileData);
            pch = strtok (NULL, ",");
        }
        #ifdef PRINT_LIST
        cout << "cycle " << cycleData.cycle << ": total " << cycleData.fileDataVector->size() << " data." << endl;
        for(int i=0; i<cycleData.fileDataVector->size(); i++) {
            cout << (*cycleData.fileDataVector)[i].originalFileId << " \t";
        }
        cout << endl;
        #endif
        cycleDataVector.push_back(cycleData); // Add to vector
    }
    inFile.close();
}

void copyToSelection(vector<CycleData> &cycleDataVector){
    // search data directory
    cout << "Starting copy ..." << endl;
    DIR *dir;
    struct dirent *ent;
    int nextFileNo = getNextFileNo(cycleDataVector);
    int fileNameIndex = -1;
    char fileNameIndexStr[5];
    if ((dir = opendir (dataDir.c_str())) != NULL) {
        cout << "dataDirName: " << dataDir << endl;
        while ((ent = readdir (dir)) != NULL){
            if(strstr(ent->d_name, "run")==NULL) continue;  // exclude other file
            strncpy(fileNameIndexStr, ent->d_name+3,4); // get file serial number
            fileNameIndexStr[4]=0;
            fileNameIndex = atoi(fileNameIndexStr);
            // cout << "nextFileNo: " << nextFileNo << endl;
            if(fileNameIndex==nextFileNo){  // file id matched
                string cmd = "copy /y \"" + dataDir + ent->d_name + "\"";
                cmd += " \"" + dataSelectionDir + ent->d_name + "\"" ;
                system(cmd.c_str());
                nextFileNo = getNextFileNo(cycleDataVector);
            }
        }
        closedir (dir);
    } else {
        // could not open directory
        perror ("");
        return;
    }
}

void singleFileExtract(const char *fileName, FileData &fileData)
{
    // open file
    ifstream inFile((dataSelectionDir+fileName).c_str(), ios::in);
    if(!inFile) {
        printf("Error! Cannot open file: %s\n",(dataSelectionDir+fileName).c_str());
    }
    cout << "File ID: " << fileData.originalFileId << "  extracting ...";


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
    #ifdef PRINT_FILE
    /*for(int i=0; i<10; i++){
        for(int j=0; j<fileData.dataVector[i].size(); j++){
            cout << fileData.dataVector[i][j] << " \t";
        }
        cout << endl;
    }*/
    #endif
}

void cycleExtract(vector<CycleData> &cycleDataVector, bool doCopy)
{
    vector<string> fileNameVector;

    static int newIdCounter = 0; // give each selected file an unique new ID

    // choose file on list
    extractList(cycleListFileName.c_str(), cycleDataVector);
    cout << "cycleDataVector.size() = " << cycleDataVector.size() << endl;

    if(doCopy){
        // copy data to selection if first time run
        copyToSelection(cycleDataVector);
    }

    // search data directory
    DIR *dir;
    struct dirent *ent;
    int nextFileNo = getNextFileNo(cycleDataVector);
    int fileNameIndex = -1;
    char fileNameIndexStr[5];
    if ((dir = opendir (dataSelectionDir.c_str())) != NULL) {
        cout << "dataSelectionDir: " << dataSelectionDir << endl;
        while ((ent = readdir (dir)) != NULL){
            if(strstr(ent->d_name, "run")==NULL) continue;  // exclude other file
            strncpy(fileNameIndexStr, ent->d_name+3,4); // get file serial number
            fileNameIndexStr[4]=0;
            fileNameIndex = atoi(fileNameIndexStr);
            // cout << "fileNameIndex: " << fileNameIndex << "  nextFileNo: " << nextFileNo << endl;
            if(nextFileNo==237)
                cout << endl << endl;
            if(fileNameIndex==nextFileNo){  // file id matched
                nextFileNo = getNextFileNo(cycleDataVector);
                for(unsigned i=0; i<cycleDataVector.size(); i++){  // search corresponding FileData
                    vector<FileData> &vec = cycleDataVector[i].fileDataVector;
                    for(unsigned j=0; j<vec.size(); j++){
                        if(vec[j].originalFileId==fileNameIndex){
                            vec[j].id = newIdCounter++;
                            vec[j].fileName = ent->d_name;
                            break;
                        }
                    }
                }
                fileNameVector.push_back(dataSelectionDir + ent->d_name);
                cout << "using file: " << ent->d_name << endl;
                #ifdef COPY_FILE_TO_SELECTION
                char cmd[512] = "copy /y .\\dp_variable_selection\\\"";  // copy to selection directory
                strcat(cmd, ent->d_name);
                strcat(cmd, "\" .\\dp_variable_selection\\selection");
                system(cmd);
                #endif

            }
        }
        closedir (dir);
    } else {
        // could not open directory
        perror ("");
        return;
    }

    // start file extraction
    //cout << "size of cycle[0] = " << cycleDataVector[0].fileDataVector.size() << endl;
    //cout << cycleDataVector[0].fileDataVector[0].fileName << endl;
    //singleFileExtract(cycleDataVector[0].fileDataVector[0].fileName.c_str(), cycleDataVector[0].fileDataVector[0]);
    for(unsigned i=0; i<1; i++){ // for each cycle
        cout << "Cycle = " << cycleDataVector[i].cycle << endl;
        for(unsigned j=0; j<cycleDataVector[i].fileDataVector.size(); j++){ // for each file
            singleFileExtract(cycleDataVector[i].fileDataVector[j].fileName.c_str(),
                              cycleDataVector[i].fileDataVector[j]);
        }
    }
    printf("Finished !\n");

}

int main(int argc, char *argv[])
{
    vector<CycleData> cycleDataVector; // database

    bool doCopy = false;
    if(argc==2 && strcmp(argv[1],"init")==0)
        doCopy = false;
    cycleExtract(cycleDataVector, doCopy);

    FileData fd = getFileById(cycleDataVector, 0);
    cout << "fileName = \"" << fd.fileName << "\"" << endl;
    cout << "originalFileId = " << fd.originalFileId << endl;
    cout << "id = " << fd.id << endl;
    cout << fd.dataVector.size() << endl;
    for(unsigned i=0; i<100; i++){ // each line
            cout << fd.timeStamp[i] << endl;
        for(unsigned j=0; j<fd.dataVector[i].size(); j++){
            cout << fd.dataVector[i][j] << " ";
        }
        cout << endl;
    }
    return 0;
}
