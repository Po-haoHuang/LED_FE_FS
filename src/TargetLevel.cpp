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

bool TargetLevel::init(string fileName)
{
    cout << "Loading file \"" << fileName << "\": ";

    // open file
    ifstream inFile((fileName).c_str(), ios::in);
    if(!inFile) {
        cout << "Error! Cannot open file!" << endl;
        return false;
    }
    cout << "Successful" << endl;

    // read attribute name
    char lineBuffer[LINE_BUFFER_SIZE];
    inFile.getline(lineBuffer, LINE_BUFFER_SIZE);
    csvSplit(string(lineBuffer), ',', attrName);
    attrName.erase(attrName.begin(), attrName.begin()+3);
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

    return true;
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

bool TargetLevel::cycle_EW_discrete(vector<vector<int> >& discreteData, int partitionNum)
{
    cout << "Start cycle_EW_discrete ... containing cycle: ";
    vector<vector<double> > cycleFeatureData;
    vector<vector<int> > cycleDiscreteData;
    for(unsigned i=0; i<featureData.size(); i++){
        if(featureDataCycle[i]==featureDataCycle[i+1]){
            cycleFeatureData.push_back(featureData[i]);
        }

        if(i==featureData.size()-1 || featureDataCycle[i]!=featureDataCycle[i+1]){
            cycleFeatureData.push_back(featureData[i]);
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

/*bool TargetLevel::EW_discrete2(vector<vector<int> >& discreteData, vector<vector<double> >& inFeatureData, int partitionNum)
{
    if(partitionNum<=1){
        vector<int> dataLine(featureNumber(),1);
        for(unsigned i=0; i<inFeatureData.size(); i++){
            discreteData.push_back(dataLine);
        }
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
        interval[i] = (attrMax[i] - attrMin[i])/(partitionNum-1);
    }

    vector<int> discreteLineValue;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        discreteLineValue.clear();
        for(unsigned j=0; j<featureNumber(); j++){
            int level;
            if(interval[j]==0){
                level = 1;
            }else{
                level = (inFeatureData[i][j]-attrMin[j])/interval[j] +1.001;
            }
            discreteLineValue.push_back(level);
        }
        discreteData.push_back(discreteLineValue);
    }
    return true;
}*/


//Need to modify to cut target attribute only
bool TargetLevel::mamual_discrete(vector<vector<double> >&inFeatureData, vector<vector<int> >& discreteData, vector<double>& cutPoints)
{
    if(cutPoints.size()<=1){
        vector<int> dataLine(featureNumber(),1);
        for(unsigned i=0; i<inFeatureData.size(); i++){
            discreteData.push_back(dataLine);
        }
        return true;
    }

    sort(cutPoints.begin(), cutPoints.end());

    vector<int> lineData;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        lineData.clear();
        for(unsigned j=0; j<inFeatureData[i].size(); j++){
            int level=-1;
            double currentValue = inFeatureData[i][j];
            for(unsigned i=0; i<cutPoints.size(); i++){
                if(currentValue < cutPoints[i]){
                    level = i+1;
                    break;
                }
            }
            if(level==-1){  // larger than last cut point
                level = cutPoints.size()+1;
            }
            lineData.push_back(level);
        }
        discreteData.push_back(lineData);
    }
    return true;
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
        interval[i] = (attrMax[i] - attrMin[i])/(partitionNum-1);
    }

    vector<int> discreteLineValue;
    for(unsigned i=0; i<inFeatureData.size(); i++){
        discreteLineValue.clear();
        for(unsigned j=0; j<featureNumber(); j++){
            int level;
            if(interval[j]==0){
                level = 1;
            }else{
                level = (inFeatureData[i][j]-attrMin[j])/interval[j] +1.001;
            }
            discreteLineValue.push_back(level);
        }
        discreteData.push_back(discreteLineValue);
    }
    if(inDataPtr==NULL)
        cout << "done." << endl;
    return true;
}
