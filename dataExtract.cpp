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

const char cycleListFileName[] = "use_file_list.csv";
const char dataDirName[] = "dp_variable_selection";
const char selectionPath[] = ".\\dp_variable_selection\\selection\\";  // Windows only

struct FileData{
    int id;
    int originalFileId;
    string fileName;
    vector<vector<double> > dataVector;
    vector<string> *timeStamp;
    vector<string> *attributeTypeVector;
};

struct CycleData {
    double cycle;
    vector<FileData> *fileDataVector;
};

FileData& getFileById(vector<CycleData> &cycleDataVector, int id){
    for(unsigned i=0; i<cycleDataVector.size(); i++){  // search corresponding FileData
        vector<FileData> &vec = *cycleDataVector[i].fileDataVector;
        for(unsigned j=0; j<vec.size(); j++){
            if(vec[j].id==id){
                return vec[j];
                break;
            }
        }
    }
    return *(new FileData);
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
        cycleData.fileDataVector = new vector<FileData>;

        // split the line by comma (CSV file)
        pch = strtok (lineBuffer,",");
        cycleData.cycle = atof(pch);
        pch = strtok (NULL, ",");
        while(pch != NULL) {
            FileData fileData;
            fileData.originalFileId = atof(pch);
            cycleData.fileDataVector->push_back(fileData);
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

void singleFileExtract(const char *fileName)
{
    // open file
    ifstream inFile(fileName, ios::in);
    if(!inFile) {
        printf("Cannot open file: %s\n",fileName);
    }
    // cout << fileName << endl;

    char lineBuffer[LINE_BUFFER_SIZE];
    char *pch;

    // Initialize
    FileData fileData;
    //fileData.dataVector = new vector<vector<double> >;
    fileData.attributeTypeVector = new vector<string>;
    fileData.timeStamp = new vector<string>;

    // read first line (attribute type)
    inFile.getline(lineBuffer, LINE_BUFFER_SIZE);
    pch = strtok (lineBuffer,",");
    while (pch != NULL) {
        fileData.attributeTypeVector->push_back(pch);
        // cout << fileData.attributeTypeVector->back() << " \t";
        pch = strtok (NULL, ",");
    }

    // read data by line
    while(inFile.getline(lineBuffer, LINE_BUFFER_SIZE)) {
        vector<double> dataInLine;
        dataInLine.reserve(8);
        // split the line by comma
        pch = strtok (lineBuffer,",");
        fileData.timeStamp->push_back(pch);
        pch = strtok (NULL, ",");
        for(unsigned i=0; i<fileData.attributeTypeVector->size()-1; i++) {
            dataInLine.push_back(atof(pch));
            pch = strtok (NULL, ",");
        }
        fileData.dataVector.push_back(dataInLine);
    }
    #ifdef PRINT_FILE
    cout << "In file " << fileName << " read " << fileData.dataVector.size() << " lines" << endl;
    for(int i=0; i<10; i++){
        for(int j=0; j<fileData.dataVector[i].size(); j++){
            cout << fileData.dataVector[i][j] << " \t";
        }
        cout << endl;
    }
    #endif
    inFile.close();
}

int getNextFileNo(vector<CycleData> &cycleDataVector)
{
    static unsigned cycle = 0;
    static unsigned file = 0;
    int nextFileNo = -1;
    if(cycle < cycleDataVector.size()) {
        if(file >= cycleDataVector[cycle].fileDataVector->size()) {
            cycle++;
            file=0;
        }
        if(cycle < cycleDataVector.size())
            nextFileNo = (*cycleDataVector[cycle].fileDataVector)[file].originalFileId;
        file++;
    }
    return nextFileNo;
}

void cycleExtract(vector<CycleData> &cycleDataVector)
{
    vector<string> fileNameVector;

    static int newIdCounter = 0; // give each selected file an unique new ID

    // choose file on list
    extractList(cycleListFileName, cycleDataVector);
    printf("cycleDataVector.size() = %d\n", cycleDataVector.size());

    DIR *dir;
    struct dirent *ent;
    int nextFileNo = getNextFileNo(cycleDataVector);
    int fileNameIndex = -1;
    char fileNameIndexStr[5];

    // search data directory
    string path = selectionPath;
    if ((dir = opendir (dataDirName)) != NULL) {
        while ((ent = readdir (dir)) != NULL){
            if(strstr(ent->d_name, "run")==NULL) continue;  // exclude other file
            strncpy(fileNameIndexStr, ent->d_name+3,4); // get file serial number
            fileNameIndexStr[4]=0;
            fileNameIndex = atoi(fileNameIndexStr);
            if(fileNameIndex==nextFileNo){  // file id matched
                nextFileNo = getNextFileNo(cycleDataVector);
                for(unsigned i=0; i<cycleDataVector.size(); i++){  // search corresponding FileData
                    vector<FileData> &vec = *cycleDataVector[i].fileDataVector;
                    for(unsigned j=0; j<vec.size(); j++){
                        if(vec[j].originalFileId==fileNameIndex){
                            vec[j].id = newIdCounter++;
                            vec[j].fileName = ent->d_name;
                            break;
                        }
                    }
                }
                fileNameVector.push_back(path + ent->d_name);
                // cout << fileNameVector.back() << endl;
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
    // i<fileNameVector.size()
    for(unsigned i=0; i<2; i++){
        singleFileExtract(fileNameVector[i].c_str());
    }
    printf("Finished !\n");

}

int main()
{
    vector<CycleData> cycleDataVector;
    cycleExtract(cycleDataVector);
    FileData fd = getFileById(cycleDataVector, 0);
    cout << "FileName = \"" << fd.fileName << "\"" << endl;
    cout << "originalFileId = " << fd.originalFileId << endl;
    cout << fd.dataVector.size() << endl;
    /*for(unsigned i=0; i<fd.dataVector->size(); i++){
        cout << "fd.dataVector->size() = " << fd.dataVector->size() << endl;
        cout << "(*fd.dataVector)[i].size() = " << (*fd.dataVector)[i].size() << endl;
        for(unsigned j=0; j<(*fd.dataVector)[i].size(); j++){
            cout << (*fd.dataVector)[i][j] << " ";
        }
        cout << endl;
    }*/
    return 0;
}
