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
    void printList();
    bool init(string rawDataDir, string useDir, string listFileName);
    bool init(string useDir, string listFileName);
    bool extract(double cycleBegin, double cycleEnd);
    bool valid(){return dbValid;};

private:
    vector<CycleData> mdb;
    int getNextFileNo();
    bool extractList();
    bool singleFileExtract(string fileName, FileData &fileData);
    string dir;
    string listFile;
    bool dbValid;
};

#endif // DATABASE_H
