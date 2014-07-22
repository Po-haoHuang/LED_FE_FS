#include <iostream>
#include <vector>
#include <cfloat>
#include <algorithm>

#include "../include/FeatureSelection.h"

using std::cout;
using std::endl;
using std::vector;

bool FeatureSelection::disct_ew(vector<vector<int> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr)
{
    vector<vector<double> > *inFeatureDataPtr;
    if(inDataPtr==NULL){
        inFeatureDataPtr = &(this->featureData);
        cout << "Start disct_ew ... ";
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

bool FeatureSelection::disct_ew_cycle(vector<vector<int> >& discreteData, int partitionNum)
{
    cout << "Start cycle_disct_ew ... containing cycle: ";
    vector<vector<double> > cycleFeatureData;
    vector<vector<int> > cycleDiscreteData;
    for(unsigned i=0; i<featureData.size(); i++){
        cycleFeatureData.push_back(featureData[i]);
        // when reading last element or cycles not equal, it's the end of cycleDiscreteData
        if(i==featureData.size()-1 || featureDataCycle[i]!=featureDataCycle[i+1]){
            cout << featureDataCycle[i] << " ";
            disct_ew(cycleDiscreteData, partitionNum, &cycleFeatureData);
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
bool FeatureSelection::disct_manual(vector<double> &inFeatureData, vector<int> &discreteData, vector<double> &cutPoints)
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
