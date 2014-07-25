#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "../include/FeatureSelection.h"

using namespace std;

const string featureDataSetFileName = "Output_noSeg.csv";
const string targetColName = "dP_Filter (X1)_max";
const string resultFileName = "FS_result.csv";
const string disctFileName = "FS_disct.csv";
const string labelsFileName = "FS_labels.csv";
const double manual_cut_points[] = {5,15,20};

int main(){

    // output result to file
    ofstream resultFile(resultFileName.c_str());
    if(!resultFile){
        cout << "Error! resultFile failed. Please turn off the opened file." << endl;
        return 1;
    }

    // main data class
    FeatureSelection fs;
    fs.init(featureDataSetFileName);

    // exclude some attributes
    fs.excludeAttr("dP_Filter");
    fs.excludeAttr("skewness");
    fs.excludeAttr("kurtosis");
    fs.excludeZeros();

    // set cut points (defined in the head of file)
    vector<double> cutPoints(manual_cut_points,manual_cut_points
                             + sizeof(manual_cut_points)/sizeof(double));

    // EW_discrete for discreteData
    vector<vector<int> > discreteData;
    fs.disct_ew(discreteData, cutPoints.size()+1);

    // choose target column
    vector<double> targetColVec;
    if(!fs.getAttrCol(targetColName, targetColVec)){
        cout << "Not found attrName: " << targetColName << endl;
        return 1;
    }

    // discretized input labels
    vector<int> labels;

    // choose the method
    int disctMethod = 2;
    cout << "Which method to discretize labels ? 1.NON  2.manual  3.EW_discrete  : 2" << endl;
    switch(disctMethod){
    default:
    case 1:

    case 2:
        fs.disct_manual(targetColVec, labels, cutPoints);
        break;
    case 3:
        break;
    }

    // print the attributes in use
    cout << "use " << fs.numOfUsedFeatures() << " features: " << endl;
    resultFile << "use " << fs.numOfUsedFeatures() << " features: " << endl;
    for(unsigned i=0; i<fs.numOfUsedFeatures(); i++){
        cout << i+1 << ": " << fs.getAttrName(fs.useFeatureId(i)) << endl;
        resultFile << i+1 << ", " << fs.getAttrName(fs.useFeatureId(i)) << endl;
    }
    cout << endl;

    // change EW-discrete data to input matrix 1-D column array
    int matrixCounter=0;
    double *dataMatrix = new double[discreteData.size()*(fs.numOfUsedFeatures())];  // 966 * n
    for(unsigned col=0; col<fs.numOfUsedFeatures(); col++){
        for(unsigned row=0; row<fs.numOfSamples(); row++){
            dataMatrix[matrixCounter++] = discreteData[row][col];
        }
    }

    // convert labels from int to double
    double *labelsD = new double[labels.size()];
    for(unsigned i=0; i<labels.size();  i++){
        labelsD[i] = labels[i];
    }
    int top_k = 15;

    // JMI
    double *outputFeaturesJMI = new double[top_k];
    fs.JMI(top_k, targetColVec.size(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesJMI);
    cout  << "JMI select from " << targetColVec.size() << " samples: " << endl;
    resultFile << endl << "JMI select from " << targetColVec.size() << " samples: " << endl;
    for(int i=0; i<top_k; i++){
        int useFtId = fs.useFeatureId(outputFeaturesJMI[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << "," << fs.getAttrName(useFtId) << endl;
    }

    // MRMR
    double *outputFeaturesMRMR = new double[top_k];
    fs.MRMR(top_k, targetColVec.size(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesMRMR);
    cout << endl<< "MRMR select from " << targetColVec.size() << " samples: " << endl;
    resultFile << endl << "MRMR select from " << targetColVec.size() << " samples: " << endl;
    for(int i=0; i<top_k; i++){
        int useFtId = fs.useFeatureId(outputFeaturesMRMR[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }

    // CHI
    vector<int> outputFeaturesCHI;
    fs.CHI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesCHI);
    cout << endl << "CHI select from " << targetColVec.size() << " samples: " << endl;
    resultFile << endl << "CHI select from " << targetColVec.size() << " samples: " << endl;
    for(unsigned i=0; i<outputFeaturesCHI.size(); i++){
        int useFtId = fs.useFeatureId(outputFeaturesCHI[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }

    // FCBF
    vector<int> outputFeaturesFCBF;
    double threshold = 0.01;
    fs.FCBF(fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, threshold, outputFeaturesFCBF);
    cout << endl << "FCBF select from " << targetColVec.size() << " samples: " << endl;
    resultFile << endl << "FCBF select from " << targetColVec.size() << " samples: " << endl;
    for(unsigned i=0; i<outputFeaturesFCBF.size(); i++){
        int useFtId = fs.useFeatureId(outputFeaturesFCBF[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }

    resultFile.close();

    // output labels data to csv file
    ofstream labelsFile(labelsFileName.c_str());
    for(unsigned i=0; i<labels.size(); i++){
        labelsFile << labels[i] << endl;
    }
    labelsFile.close();

    // output discrete data to csv file
    ofstream disctFile(disctFileName.c_str());
    for(unsigned i=0; i<discreteData.size(); i++){
        for(unsigned ftid=0; ftid<fs.numOfUsedFeatures(); ftid++){
            disctFile << discreteData[i][ftid];
            if(ftid!=fs.numOfUsedFeatures()-1) disctFile << ",";
        }
        disctFile << endl;
    }
    disctFile.close();

    free(dataMatrix);
    free(labelsD);
    free(outputFeaturesJMI);
    free(outputFeaturesMRMR);

    return 0;
}

