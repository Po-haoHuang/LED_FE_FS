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
    void printList();
    bool copyToSelection(string rawDataDir, string useDir, string listName);
    bool init(string useDir, string listFileName);
    bool extract(double cycleBegin, double cycleEnd);

private:
    vector<CycleData> mdb;
    int getNextFileNo();
    bool extractList();
    bool singleFileExtract(string fileName, FileData &fileData);
    string dir;
    string listFile;
};

#endif // DATABASE_H
