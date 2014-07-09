#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include "FileData.h"
#include "CycleData.h"

using std::vector;
using std::string;

class DataBase{
public:
    DataBase();
    virtual ~DataBase();
    bool getFileById(int id, FileData &fd);
    bool getCycle(double cycle, CycleData& cd);
    bool getAllFileData(vector<FileData>& fdVector);
    void printCycleList();
    bool init(string useDir, string listFileName);
    bool extract(double cycleBegin, double cycleEnd);
    bool valid(){return dbValid;};

private:
    vector<CycleData> mdb;
    int getNextFileNo();
    bool loadListFile();
    bool addFileFromDir();
    bool singleFileExtract(string fileName, FileData &fileData);
    void csvValueSplit(string s, const char delimiter, vector<double> &lineValue);
    string dir;
    string listFile;
    bool dbValid;
    vector<string> fileNameVector;
    vector<int> fileIdVector;
};

#endif // DATABASE_H
