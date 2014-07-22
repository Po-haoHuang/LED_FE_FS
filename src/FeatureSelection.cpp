#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cfloat>

#include "../include/FeatureSelection.h"

using namespace std;

const unsigned LINE_BUFFER_SIZE = 4096;

FeatureSelection::FeatureSelection()
{
    //ctor
}

FeatureSelection::~FeatureSelection()
{
    //dtor
}

bool FeatureSelection::init(string FE_fileName)
{
    cout << "Loading file \"" << FE_fileName << "\": ";

    // open file
    ifstream inFile((FE_fileName).c_str(), ios::in);
    if(!inFile) {
        cout << "Error: Cannot open file." << endl;
        return false;
    }
    cout << "Successful" << endl;

    // read attribute title
    char lineBuffer[LINE_BUFFER_SIZE];
    inFile.getline(lineBuffer, LINE_BUFFER_SIZE);
    string attrTitle = lineBuffer;
    unsigned thirdCommaPos = attrTitle.find(",");  // delete id, fid and cycle info
    thirdCommaPos = attrTitle.find(",",thirdCommaPos+1);
    thirdCommaPos = attrTitle.find(",",thirdCommaPos+1);
    attrTitle = attrTitle.substr(thirdCommaPos+1);

    // read attribute name
    csvSplit(attrTitle, ',', attrNameVec);
    attrNameVec.pop_back(); // remove redundant value
    cout << "feature number: " << featureNumber() << endl;

    // read data by line
    featureData.reserve(1024);
    vector<double> lineValue;
    vector<string> lineStrValue;
    while(inFile.getline(lineBuffer, LINE_BUFFER_SIZE)){
        lineValue.clear();
        lineStrValue.clear();
        csvSplit(string(lineBuffer), ',', lineStrValue);
        featureDataCycle.push_back(atof(lineStrValue[2].c_str()));
        for(unsigned i=3; i<lineStrValue.size(); i++){
            lineValue.push_back(atof(lineStrValue[i].c_str()));
        }
        featureData.push_back(lineValue);
    }
    cout << "total lines: " << featureData.size() << endl;

    inFile.close();
    return true;
}

string FeatureSelection::attrTitle()
{
    string title;
    for(unsigned i=0; i<attrNameVec.size(); i++){
        title += attrNameVec[i] + ",";
    }
    return title;
}

int FeatureSelection::idOfAttr(string attrName)
{
    for(unsigned col=0; col<featureNumber(); col++){
        if(attrNameVec[col].find(attrName) != string::npos){ // match
            return col;
        }
    }
    return -1;
}

bool FeatureSelection::getAttrCol(string attrName, vector<double> &colVec)
{
    for(unsigned col=0; col<featureNumber(); col++){
        if(attrNameVec[col].find(attrName) != string::npos){ // match
            for(unsigned j=0; j<featureData.size(); j++){
                colVec.push_back(featureData[j][col]);
            }
            return true;
        }
    }
    return false;
}

void FeatureSelection::csvSplit(string s, const char delimiter, vector<string> &value)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    while (end <= std::string::npos){
        value.push_back(s.substr(start, end-start));
        if (end == std::string::npos)
	    	break;
    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
}