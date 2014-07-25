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

    cout << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features" << endl;
    resultFile << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features" << endl;

    // exclude some attributes
    fs.excludeAttr("dP_Filter");
    fs.excludeAttr("skewness");
    fs.excludeAttr("kurtosis");
    fs.excludeZeroColumn();

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
    cout << "Which method to discretize labels ? 1.NON  2.manual  3.EW_discrete  : 2" << endl << endl;
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
    cout << "only use " << fs.numOfUsedFeatures() << " features for calculation: " << endl;
    resultFile << "only use " << fs.numOfUsedFeatures() << " features for calculation: " << endl;
    for(unsigned i=0; i<fs.numOfUsedFeatures(); i++){
        cout << i+1 << ": " << fs.getAttrName(fs.useFeatureId(i)) << endl;
        resultFile << i+1 << ", " << fs.getAttrName(fs.useFeatureId(i)) << endl;
    }
    cout << endl;

    // change EW-discrete data to input matrix 1-D column array
    int matrixCounter=0;
    double *dataMatrix = new double[fs.numOfSamples() * fs.numOfUsedFeatures()];  // 966 * n
    for(unsigned col=0; col<fs.numOfUsedFeatures(); col++){
        for(unsigned row=0; row<fs.numOfSamples(); row++){
            dataMatrix[matrixCounter++] = static_cast<double>(discreteData[row][col]);
        }
    }

    // convert labels from int to double
    double *labelsD = new double[labels.size()];
    for(unsigned i=0; i<labels.size();  i++){
        labelsD[i] = static_cast<double>(labels[i]);
    }

    int top_k = 15;

    // JMI
    vector<int> outputFeaturesJMI;
    fs.JMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesJMI);
    cout  << "JMI select: " << endl;
    resultFile << endl << "JMI select: " << endl;
    for(unsigned i=0; i<outputFeaturesJMI.size(); i++){
        int useFtId = fs.useFeatureId(outputFeaturesJMI[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << "," << fs.getAttrName(useFtId) << endl;
    }

    // MRMR
    vector<int> outputFeaturesMRMR;
    fs.MRMR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesMRMR);
    cout << endl<< "MRMR select: " << endl;
    resultFile << endl << "MRMR select: " << endl;
    for(int i=0; i<top_k; i++){
        int useFtId = fs.useFeatureId(outputFeaturesMRMR[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }

    // CHI
    vector<int> outputFeaturesCHI;
    fs.CHI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesCHI);
    cout << endl << "CHI select: " << endl;
    resultFile << endl << "CHI select: " << endl;
    for(unsigned i=0; i<outputFeaturesCHI.size(); i++){
        int useFtId = fs.useFeatureId(outputFeaturesCHI[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }

    // FCBF
    vector<int> outputFeaturesFCBF;
    double threshold = 0.01;
    fs.FCBF(fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, threshold, outputFeaturesFCBF);
    cout << endl << "FCBF select: " << endl;
    resultFile << endl << "FCBF select: " << endl;
    for(unsigned i=0; i<outputFeaturesFCBF.size(); i++){
        int useFtId = fs.useFeatureId(outputFeaturesFCBF[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }

    // output labels data to csv file
    ofstream labelsFile(labelsFileName.c_str());
    for(unsigned i=0; i<labels.size(); i++){
        labelsFile << labels[i] << endl;
    }

    // output discrete data to csv file
    ofstream disctFile(disctFileName.c_str());
    for(unsigned i=0; i<discreteData.size(); i++){
        for(unsigned ftid=0; ftid<fs.numOfUsedFeatures(); ftid++){
            disctFile << discreteData[i][ftid];
            if(ftid!=fs.numOfUsedFeatures()-1) disctFile << ",";
        }
        disctFile << endl;
    }

    resultFile.close();
    labelsFile.close();
    disctFile.close();
    delete [] dataMatrix;
    delete [] labelsD;

    return 0;
}

