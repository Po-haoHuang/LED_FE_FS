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
    int originalFileId;
    string fileName;
    vector<vector<double> > dataVector;
    vector<string> timeStamp;
    vector<string> attributeTypeVector;

private:
};

#endif // FILEDATA_H
