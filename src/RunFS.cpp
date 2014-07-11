#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../include/TargetLevel.h"

using namespace std;

const string featureDataSetFileName = "Output_noSeg.csv";

int main(){
    TargetLevel targetLevel;
    targetLevel.init(featureDataSetFileName);

    /****** discrete all ******/
    int ewPt;
    cout << "set EW_discrete cut points number: ";
    cin >> ewPt;
    vector<vector<int> > discreteData;
    targetLevel.EW_discrete(discreteData, ewPt);

    ofstream outFile("Output_noSeg_discrete_EW.csv");
    if(!outFile){
        cout << "Error! outFile failed." << endl;
        return 1;
    }
    outFile << endl;
    for(unsigned i=0; i<discreteData.size(); i++){
        for(unsigned j=0; j<discreteData[i].size(); j++){
            outFile << discreteData[i][j] << ",";
        }
        outFile << endl;
    }

    /***** discrete cycle *****/
    vector<vector<int> > discreteDataC;
    targetLevel.cycle_EW_discrete(discreteDataC, ewPt);
    ofstream outFileC("Output_noSeg_discrete_EW_cycle.csv");
    if(!outFileC){
        cout << "Error! outFile failed." << endl;
        return 1;
    }
    outFileC << endl;
    for(unsigned i=0; i<discreteDataC.size(); i++){
        for(unsigned j=0; j<discreteDataC[i].size(); j++){
            outFileC << discreteDataC[i][j] << ",";
        }
        outFileC << endl;
    }

    /***** discrete manual *****/
    ofstream outFileManual("Output_noSeg_discrete_Manual.csv");
    if(!outFileManual){
        cout << "Error! outFileManual failed." << endl;
        return 1;
    }
    outFileManual << endl;
    int cutPortion;
    vector<vector<int> > discreteDataManual;
    vector<double> cutPoints;
    cout << "cut points number: ";
    cin >> cutPortion;
    for(int i=0; i<cutPortion; i++){
        double pt;
        cout << i+1 << ": ";
        cin >> pt;
        cutPoints.push_back(pt);
    }
    targetLevel.mamual_discrete(targetLevel.allFeatureData(), discreteDataManual, cutPoints);

    for(unsigned i=0; i<discreteDataManual.size(); i++){
        for(unsigned j=0; j<discreteDataManual[i].size(); j++){
            outFileManual << discreteDataManual[i][j] << ",";
        }
        outFileManual << endl;
    }

    return 0;
}

