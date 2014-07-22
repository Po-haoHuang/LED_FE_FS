#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

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

    // set cut points (defined in the head of file)
    vector<double> cutPoints(manual_cut_points,manual_cut_points
                             + sizeof(manual_cut_points)/sizeof(double));

    // EW_discrete for discreteData (JMI/MRMR function's input matrix)
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
    //cin >> disctMethod;
    switch(disctMethod){
    default:
    case 1:

    case 2:
        fs.disct_manual(targetColVec, labels, cutPoints);
        break;
    case 3:
        for(unsigned i=0; i<discreteData.size(); i++){
            labels.push_back(discreteData[i][fs.idOfAttr(targetColName)]);
        }
        break;
    }

    // record using features' id (exclude undesired attributes)
    vector<int> useFeatureId(fs.featureNumber());
    for(unsigned i=0; i<useFeatureId.size(); i++){
        useFeatureId[i] = i;  // initialize: use all feature id
    }
    for(unsigned i=0; i<fs.featureNumber(); i++){
        if(fs.getAttrName(i).find("dP_Filter")!=string::npos){  // exclude dp_filter attribute
            useFeatureId[i]=-1;
        }else if(fs.getAttrName(i).find("skewness")!=string::npos ||
                fs.getAttrName(i).find("kurtosis")!=string::npos){ // exclude skewness and kurtosis attribute
            useFeatureId[i]=-1;
        }else{
            vector<double> colVec;
            fs.getAttrCol(fs.getAttrName(i), colVec);  // exclude zero columns
            if(*max_element(colVec.begin(), colVec.end()) == 0){
                useFeatureId[i]=-1;
            }
        }
    }

    // remove unused feature id
    vector<int> useFeatureIdReplace;  // temporary use only
    for(unsigned i=0; i<useFeatureId.size(); i++){
        if(useFeatureId[i]!=-1) useFeatureIdReplace.push_back(i);
    }
    useFeatureId.swap(useFeatureIdReplace);

    // print the attributes in use
    cout << "use " << useFeatureId.size() << " features: " << endl;
    resultFile << "use " << useFeatureId.size() << " features: " << endl;
    for(unsigned i=0; i<useFeatureId.size(); i++){
        cout << i+1 << ": " << fs.getAttrName(useFeatureId[i]) << endl;
        resultFile << i+1 << ", " << fs.getAttrName(useFeatureId[i]) << endl;
    }
    cout << endl;

    // use EW-cycle discrete data as input matrix
    int matrixCounter=0;
    double *dataMatrix = new double[discreteData.size()*(useFeatureId.size())];  // 966 * n
    for(unsigned i=0; i<useFeatureId.size(); i++){
        for(unsigned j=0; j<discreteData.size(); j++){
            dataMatrix[matrixCounter++] = discreteData[j][useFeatureId[i]];
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
    fs.JMI(top_k, targetColVec.size(), useFeatureId.size(), dataMatrix, &labelsD[0], outputFeaturesJMI);
    cout  << "JMI select from " << targetColVec.size() << " samples: " << endl;
    resultFile << endl << "JMI select from " << targetColVec.size() << " samples: " << endl;
    for(int i=0; i<top_k; i++){
        int memId = useFeatureId[outputFeaturesJMI[i]];
        cout << i+1 << ": " << fs.getAttrName(memId) << endl;
        resultFile << i+1 << "," << fs.getAttrName(memId) << endl;
    }

    // MRMR
    double *outputFeaturesMRMR = new double[top_k];
    fs.MRMR(top_k, targetColVec.size(), useFeatureId.size(), dataMatrix, &labelsD[0], outputFeaturesMRMR);
    cout << endl<< "MRMR select from " << targetColVec.size() << " samples: " << endl;
    resultFile << endl << "MRMR select from " << targetColVec.size() << " samples: " << endl;
    for(int i=0; i<top_k; i++){
        int memId = useFeatureId[outputFeaturesMRMR[i]];
        cout << i+1 << ": " << fs.getAttrName(memId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(memId) << endl;
    }

    // CHI
    double *outputFeaturesCHI = new double[top_k];
    int outputFeatureNum;
    fs.CHI(top_k, targetColVec.size(), useFeatureId.size(), dataMatrix, &labelsD[0], outputFeaturesCHI, &outputFeatureNum);
    cout << endl << "CHI select from " << targetColVec.size() << " samples: " << endl;
    resultFile << endl << "CHI select from " << targetColVec.size() << " samples: " << endl;
    for(int i=0; i<outputFeatureNum; i++){
        int memId = useFeatureId[outputFeaturesMRMR[i]];
        cout << i+1 << ": " << fs.getAttrName(memId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(memId) << endl;
    }
    resultFile.close();

    // output labels data
    ofstream labelsFile(labelsFileName.c_str());
    for(unsigned i=0; i<labels.size(); i++){
        labelsFile << labels[i] << endl;
    }
    labelsFile.close();

    // output discrete data
    ofstream disctFile(disctFileName.c_str());
    for(unsigned i=0; i<discreteData.size(); i++){
        for(unsigned ftid=0; ftid<useFeatureId.size(); ftid++){
            disctFile << discreteData[i][useFeatureId[ftid]];
            if(ftid!=useFeatureId.size()-1) disctFile << ",";
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

