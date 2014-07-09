#ifndef FILEDATA_H
#define FILEDATA_H

#include <string>
#include <vector>

using std::string;
using std::vector;

class FileData{
public:
    FileData();
    virtual ~FileData();

    int id;
    int fid;
    string fileName;
    vector<vector<double> > dataVector;
    vector<string> timeStamp;
    vector<string> attrTypeVector;

    unsigned size(){ return dataVector.size();}
    unsigned attrSize(){return dataVector.empty()? 0 : dataVector.front().size();}

private:
};

#endif // FILEDATA_H
