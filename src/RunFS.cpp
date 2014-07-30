#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstdlib>

#include "../include/FeatureSelection.h"
#ifdef FS_Regs
#include "../include/Regression.h"
#endif

using namespace std;

void output_select_result(FeatureSelection &fs, string typeName, vector<int> &resultVec, ofstream &fout)
{
    cout << typeName << " select: " << endl;
    fout << typeName << " select: " << endl;
    for(unsigned i=0; i<resultVec.size(); i++){
        int useFtId = fs.useFeatureId(resultVec[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        fout << i+1 << "," << fs.getAttrName(useFtId) << endl;
    }
    cout << endl;
    fout << endl;
}

void process_argv(string argvStr, int &partitionNum, int &disctMethod, vector<double> &manual_cut_points)
{
    if(isdigit(argvStr[0])){  // manual, CSV parse
        size_t mStart=0;
        size_t mEnd = argvStr.find_first_of(",");
        while (mEnd <= string::npos){
            double val = atof(argvStr.substr(mStart, mEnd-mStart).c_str());
            manual_cut_points.push_back(val);
            if (mEnd == string::npos)
                break;
            mStart = mEnd+1;
            mEnd = argvStr.find_first_of(",", mStart);
        }
        disctMethod = 1;
        partitionNum = manual_cut_points.size()+1;
    }else if(argvStr.substr(0,9)=="ew_cycle="){
        disctMethod = 2;
        partitionNum = atoi(argvStr.substr(9).c_str());
    }
}

string get_result_file_name(int argc, char *argv[])
{
    string finalName;
    string inFileName = argv[1];
    if(inFileName.find_last_of("/\\")==string::npos) inFileName = inFileName.substr(0, inFileName.find("."));
    else inFileName = inFileName.substr(inFileName.find_last_of("/\\")+1,
                                        inFileName.find(".") - inFileName.find_last_of("/\\")-1);
    finalName += string("FS_result_") + inFileName;
    for(int i=2; i<argc; i++){
        finalName += string("_") + string(argv[i]);
    }
    finalName += ".csv";
    return finalName;
}

int main(int argc, char *argv[])
{
    if(argc!=10){
        cout << "Argument setting error." << endl;
        cout << "usage: FS_no_GUI.exe input_file target_feature top_k "
             << "disct_method fcbf_thrd ridge_lambda lasso_lambda els_lambda1 els_lambda2" << endl;
        return 1;
    }

    // argument setting
    const string featureDataSetFileName = argv[1];
    const string targetColName = argv[2];
    const int top_k = atoi(argv[3]);

    // set cut points (defined in the head of file)
    int partitionNum;
    int disctMethod;
    vector<double> manual_cut_points;
    process_argv(argv[4], partitionNum, disctMethod, manual_cut_points);

    // output filename
    const string resultFileName = get_result_file_name(argc, argv);
    const string disctFileName = "FS_disct.csv";
    const string labelsFileName = "FS_labels.csv";
    const string selectedDataMatrixFileName = "FS_selectedDataMatrix.csv";
    const string targetColVecFileName = "FS_targetColVec.csv";

    // output result to file
    ofstream resultFile(resultFileName.c_str());
    if(!resultFile){
        cout << resultFileName << endl;
        cout << "Error! resultFile failed. Please turn off the opened file." << endl;
        return 1;
    }

    // main data class
    FeatureSelection fs;
    if(!fs.init(featureDataSetFileName)){
        return 1;
    }
    cout << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features" << endl << endl;
    resultFile << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features" << endl << endl;

    // exclude some attributes
    fs.excludeAttr("dP_Filter");
    //fs.excludeAttr("skewness");
    //fs.excludeAttr("kurtosis");
    fs.excludeZeroColumn();

    // EW_discrete for discreteData
    vector<vector<double> > discreteData;
    //fs.disct_ew(discreteData,partitionNum);
    fs.disct_ew_cycle(discreteData,partitionNum);

    // choose target column
    vector<double> targetColVec;
    if(!fs.getAttrCol(targetColName, targetColVec)){
        cout << "Not found attrName: " << targetColName << endl;
        return 1;
    }

    // discretized input labels
    vector<double> labels;

    // choose the method
    switch(disctMethod){
    case 1:
        fs.disct_col_manual(targetColVec, labels, manual_cut_points);
        break;
    case 2:
    default:
        fs.disct_col_ew_cycle(targetColVec, labels, partitionNum);
        break;
    }

    // print the attributes in use
    cout << "use " << fs.numOfUsedFeatures() << " features for calculation: " << endl;
    resultFile << "use " << fs.numOfUsedFeatures() << " features for calculation: " << endl;
    for(unsigned i=0; i<fs.numOfUsedFeatures(); i++){
        cout << i+1 << ": " << fs.getAttrName(fs.useFeatureId(i)) << endl;
        resultFile << i+1 << ", " << fs.getAttrName(fs.useFeatureId(i)) << endl;
    }
    cout << endl;
    resultFile << endl;

    // change EW-discrete data to input matrix 1-D column array
    int matrixCounter=0;
    double *dataMatrix = new double[fs.numOfSamples() * fs.numOfUsedFeatures()];  // 966 * n
    for(unsigned col=0; col<fs.numOfUsedFeatures(); col++){
        for(unsigned row=0; row<fs.numOfSamples(); row++){
            dataMatrix[matrixCounter++] = discreteData[row][col];
        }
    }

    // 1. MI - JMI
    vector<int> outputFeaturesJMI;
    fs.JMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesJMI);
    output_select_result(fs, "JMI", outputFeaturesJMI, resultFile);

    // 2. MI - MRMR
    vector<int> outputFeaturesMRMR;
    fs.MRMR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesMRMR);
    output_select_result(fs, "MRMR", outputFeaturesMRMR, resultFile);

    // 3. MI - CMIM
    vector<int> outputFeaturesCMIM;
    fs.CMIM(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesCMIM);
    output_select_result(fs, "CMIM", outputFeaturesCMIM, resultFile);

    // 4. MI - DISR
    vector<int> outputFeaturesDISR;
    fs.DISR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesDISR);
    output_select_result(fs, "DISR", outputFeaturesDISR, resultFile);

    // 5. MI - CondMI
    vector<int> outputFeaturesCondMI;
    fs.CondMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesCondMI);
    output_select_result(fs, "CondMI", outputFeaturesCondMI, resultFile);

    // 6. MI - ICAP
    vector<int> outputFeaturesICAP;
    fs.ICAP(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesICAP);
    output_select_result(fs, "ICAP", outputFeaturesICAP, resultFile);

    // 7. MI - MIM
    vector<int> outputFeaturesMIM;
    fs.MIM(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesMIM);
    output_select_result(fs, "MIM", outputFeaturesMIM, resultFile);

    // 8. MI - CHI
    vector<int> outputFeaturesCHI;
    fs.CHI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], outputFeaturesCHI);
    output_select_result(fs, "CHI", outputFeaturesCHI, resultFile);

    // 9. MI - FCBF
    vector<int> outputFeaturesFCBF;
    double threshold = atof(argv[5]);
    fs.FCBF(fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], threshold, outputFeaturesFCBF);
    output_select_result(fs, "FCBF", outputFeaturesFCBF, resultFile);

    #ifdef FS_Regs
    // Regression
    // usage of linear regression and ridge regression(set lambda = 1 or 2 or 3)
    vector<vector<double> > selectedDataMatrix;
    fs.allSelectedData(selectedDataMatrix);
    Regression regs;
	regs.init(selectedDataMatrix, targetColVec);

    // 1. Regs - Least Square
    vector<int> regs_result1;
	regs.doLinearRegression(0,regs_result1);
	regs_result1.erase(regs_result1.begin()+top_k, regs_result1.end());
    output_select_result(fs, "Least Square", regs_result1, resultFile);

	// 2. Regs - Ridge
    vector<int> regs_result2;
    int lambda2 = atoi(argv[6]); // (1,2,3)
	regs.doLinearRegression(lambda2,regs_result2);
	regs_result2.erase(regs_result2.begin()+top_k, regs_result2.end());
    output_select_result(fs, "Ridge", regs_result2, resultFile);

    // 3. Regs - LASSO
    vector<int> regs_result3;
    int lambda3 = atoi(argv[7]); // (1,2,3)
	regs.doLarsRegression(lambda3, 0, regs_result3);
	regs_result3.erase(regs_result3.begin()+top_k, regs_result3.end());
    output_select_result(fs, "LASSO", regs_result3, resultFile);

    // 4. Regs - Elastic net
    vector<int> regs_result4;
    int lambda41 = atoi(argv[8]); // (1,2,3)
    int lambda42 = atoi(argv[9]); // (1,2,3)
	regs.doLarsRegression(lambda41, lambda42, regs_result4);
	regs_result4.erase(regs_result4.begin()+top_k, regs_result4.end());
    output_select_result(fs, "Elastic net", regs_result4, resultFile);
	#endif

    // final score ranking
    vector<vector<int> > all_select_MI;
    vector<vector<int> > all_select_Regs;
    vector<vector<int> > all_select;

    all_select_MI.push_back(outputFeaturesJMI);
    all_select_MI.push_back(outputFeaturesMRMR);
    all_select_MI.push_back(outputFeaturesCMIM);
    all_select_MI.push_back(outputFeaturesDISR);
    all_select_MI.push_back(outputFeaturesCondMI);
    all_select_MI.push_back(outputFeaturesICAP);
    all_select_MI.push_back(outputFeaturesMIM);
    all_select_MI.push_back(outputFeaturesCHI);
    all_select_MI.push_back(outputFeaturesFCBF);

    fs.score_and_rank(all_select_MI, resultFile, "MI");

    #ifdef FS_Regs
    all_select_Regs.push_back(regs_result1);
    all_select_Regs.push_back(regs_result2);
	all_select_Regs.push_back(regs_result3);
    all_select_Regs.push_back(regs_result4);

    all_select.push_back(outputFeaturesJMI);
    all_select.push_back(outputFeaturesMRMR);
    all_select.push_back(outputFeaturesCMIM);
    all_select.push_back(outputFeaturesDISR);
    all_select.push_back(outputFeaturesCondMI);
    all_select.push_back(outputFeaturesICAP);
    all_select.push_back(outputFeaturesMIM);
    all_select.push_back(outputFeaturesCHI);
    all_select.push_back(outputFeaturesFCBF);
    all_select.push_back(regs_result1);
    all_select.push_back(regs_result2);
    all_select.push_back(regs_result3);
    all_select.push_back(regs_result4);

    fs.score_and_rank(all_select_Regs, resultFile, "Regression");
    fs.score_and_rank(all_select, resultFile, "All");

    #ifdef FS_DEBUG
    // output selectedDataMatrix data to csv file
    ofstream selectedDataFile(selectedDataMatrixFileName.c_str());
    for(unsigned row=0; row<fs.numOfUsedFeatures(); row++){
        selectedDataFile << fs.getAttrName(fs.useFeatureId(row)) << ",";
    }
    selectedDataFile << endl;
    for(unsigned row=0; row<selectedDataMatrix.size(); row++){
        for(unsigned col=0; col<selectedDataMatrix[row].size(); col++){
            selectedDataFile << selectedDataMatrix[row][col] << ",";
        }
        selectedDataFile << endl;
    }
    selectedDataFile.close();
    #endif
    #endif

    #ifdef FS_DEBUG
    // output targetColVec data to csv file
    ofstream targetColVecFile(targetColVecFileName.c_str());
    for(unsigned row=0; row<targetColVec.size(); row++){
        targetColVecFile << targetColVec[row] << endl;
    }
    targetColVecFile.close();

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
    #endif // FS_DEBUG

    delete [] dataMatrix;
    resultFile.close();
    return 0;
}

