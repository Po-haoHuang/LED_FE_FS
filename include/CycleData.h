#ifndef CYCLEDATA_H
#define CYCLEDATA_H

#include <vector>
#include "FileData.h"

using std::vector;

class CycleData{
public:
    CycleData();
    virtual ~CycleData();
    double cycle;
    bool valid;
    vector<FileData> fileDataVector;

    int getCycleSize(){ return fileDataVector.size();}

private:
};

#endif // CYCLEDATA_H
