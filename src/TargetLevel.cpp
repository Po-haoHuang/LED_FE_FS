#include "TargetLevel.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cfloat>

using namespace std;

const unsigned LINE_BUFFER_SIZE = 4096;

TargetLevel::TargetLevel()
{
    //ctor
}

TargetLevel::~TargetLevel()
{
    //dtor
}

bool TargetLevel::init(string FE_fileName)
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

string TargetLevel::attrTitle()
{
    string title;
    for(unsigned i=0; i<attrNameVec.size(); i++){
        title += attrNameVec[i] + ",";
    }
    return title;
}

int TargetLevel::idOfAttr(string attrName)
{
    for(unsigned col=0; col<featureNumber(); col++){
        if(attrNameVec[col].find(attrName) != string::npos){ // match
            return col;
        }
    }
    return -1;
}

bool TargetLevel::getAttrCol(string attrName, vector<double> &colVec)
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



void TargetLevel::csvSplit(string s, const char delimiter, vector<string> &value)
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

bool TargetLevel::EW_discrete(vector<vector<int> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr)
{
    vector<vector<double> > *inFeatureDataPtr;
    if(inDataPtr==NULL){
        inFeatureDataPtr = &(this->featureData);
        cout << "Start EW_discrete ... ";
    }else{
        inFeatureDataPtr = inDataPtr;
    }

    vector<vector<double> > &inFeatureData = *inFeatureDataPtr;

    if(partitionNum<=1){
        vector<int> dataLine(featureNumber(),1);
        for(unsigned i=0; i<inFeatureData.size(); i++){
            discreteData.push_back(dataLine);
        }
        cout << "done." << endl;
        return true;
    }

    vector<double> attrMax(featureNumber(),-DBL_MAX);
    vector<double> attrMin(featureNumber(),DBL_MAX);
    vector<double> interval(featureNumber());
    for(unsigned i=0; i<inFeatureData.size(); i++){
        for(unsigned j=0; j<featureNumber(); j++){
            if(inFeatureData[i][j] > attrMax[j])
                attrMax[j] = inFeatureData[i][j];
            if(inFeatureData[i][j] < attrMin[j])
                attrMin[j] = inFeatureData[i][j];
        }
    }

    for(unsigned i=0; i<featureNumber(); i++){
        interval[i] = (attrMax[i] - attrMin[i])/partitionNum;
    }

    vector<int> discreteLineValue;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        discreteLineValue.clear();
        for(unsigned j=0; j<featureNumber(); j++){
            int level;
            if(interval[j]==0){
                level = 1;
            }else{
                level = (inFeatureData[i][j]-attrMin[j])/interval[j] + 1.0;
            }
            if(level>partitionNum) level = partitionNum;
            else if(level<1) level = 1;
            discreteLineValue.push_back(level);
        }
        discreteData.push_back(discreteLineValue);
    }
    if(inDataPtr==NULL)
        cout << "done." << endl;
    return true;
}

bool TargetLevel::cycle_EW_discrete(vector<vector<int> >& discreteData, int partitionNum)
{
    cout << "Start cycle_EW_discrete ... containing cycle: ";
    vector<vector<double> > cycleFeatureData;
    vector<vector<int> > cycleDiscreteData;
    for(unsigned i=0; i<featureData.size(); i++){
        cycleFeatureData.push_back(featureData[i]);
        // when reading last element or cycles not equal, it's the end of cycleDiscreteData
        if(i==featureData.size()-1 || featureDataCycle[i]!=featureDataCycle[i+1]){
            cout << featureDataCycle[i] << " ";
            EW_discrete(cycleDiscreteData, partitionNum, &cycleFeatureData);
            for(unsigned ci=0; ci<cycleDiscreteData.size(); ci++){
                discreteData.push_back(cycleDiscreteData[ci]);
            }
            cycleFeatureData.clear();
            cycleDiscreteData.clear();
        }

    }
    cout << " done." << endl;
    return true;
}


//Need to modify to cut target attribute only
// when value once grows over cut point's value, its label won't go lower.
bool TargetLevel::mamual_discrete(vector<double> &inFeatureData, vector<int> &discreteData, vector<double> &cutPoints)
{
    if(cutPoints.size()<=1){  // return n*1 matrix
        for(unsigned i=0; i<inFeatureData.size(); i++){
            discreteData.push_back(1);
        }
        return true;
    }
    sort(cutPoints.begin(), cutPoints.end());
    int thresholdLevel = 1;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        int level=-1;
        double currentValue = inFeatureData[i];
        // determine current value level
        for(unsigned j=0; j<cutPoints.size(); j++){
            if(currentValue < cutPoints[j]){
                level = j+1;
                break;
            }
        }
        if(level==-1){  // level undetermined, larger than last cut point
            level = cutPoints.size()+1;  // assign to max level
        }
        if(level > thresholdLevel){
            thresholdLevel = level;
        }
        discreteData.push_back(thresholdLevel);

        // when reading last element or cycles not equal, it's the end of cycleDiscreteData
        if(i==featureData.size()-1 || featureDataCycle[i]!=featureDataCycle[i+1]){
            thresholdLevel = 1;  // reset for next cycle
        }
    }
    return true;
}

