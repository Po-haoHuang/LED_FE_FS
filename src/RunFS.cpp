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

    // log the parameters
    resultFile << "Feature Selection Result" << endl;
    resultFile << "Use file:," << featureDataSetFileName << endl;
    resultFile << "target:," << targetColName << endl;
    resultFile << "top_k:," << top_k << endl;
    resultFile << "target discretize method:," << argv[4] << endl;
    resultFile << "FCBF threshold:," << argv[5] << endl;
    resultFile << "RIDGE lambda:," << argv[6] << endl;
    resultFile << "LASSO lambda:," << argv[7] << endl;
    resultFile << "ELS lambda 1:," << argv[8] << endl;
    resultFile << "ELS lambda 2:," << argv[9] << endl;
    resultFile << endl;

    // main data class
    FeatureSelection fs(featureDataSetFileName);
    if(!fs.valid()){
        return 1;
    }

    // exclude some attributes
    fs.excludeAttr("dP_Filter");
    //fs.excludeAttr("skewness");
    //fs.excludeAttr("kurtosis");
    fs.excludeAttr("Cp2Mg_1.source (F120)");
    fs.excludeZeroColumn();

    cout << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features. ";
    cout << "Use " << fs.numOfUsedFeatures() << " features for calculation." << endl;
    resultFile << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features. ";
    resultFile << "Use " << fs.numOfUsedFeatures() << " features for calculation." << endl;

    /// Discretize

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
    resultFile << fs.sout.str() << endl;

    // change EW-discrete data to input matrix 1-D column array
    int matrixCounter=0;
    double *dataMatrix = new double[fs.numOfSamples() * fs.numOfUsedFeatures()];  // 966 * n
    for(unsigned col=0; col<fs.numOfUsedFeatures(); col++){
        for(unsigned row=0; row<fs.numOfSamples(); row++){
            dataMatrix[matrixCounter++] = discreteData[row][col];
        }
    }

    /// Algorithm
    vector<vector<int> > mi_result(9,vector<int>());

    // 1. MI - JMI
    fs.JMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[0]);

    // 2. MI - MRMR
    fs.MRMR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[1]);

    // 3. MI - CMIM
    fs.CMIM(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[2]);

    // 4. MI - DISR
    fs.DISR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[3]);

    // 5. MI - CondMI
    fs.CondMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[4]);

    // 6. MI - ICAP
    fs.ICAP(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[5]);

    // 7. MI - MIM
    fs.MIM(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[6]);

    // 8. MI - CHI
    fs.CHI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_result[7]);

    // 9. MI - FCBF
    double threshold = atof(argv[5]);
    fs.FCBF(fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], threshold, mi_result[8]);

    #ifdef FS_Regs
    // Regression
    // usage of linear regression and ridge regression(set lambda = 1 or 2 or 3)
    vector<vector<double> > selectedDataMatrix;
    fs.allSelectedData(selectedDataMatrix);
    Regression regs;
	regs.init(selectedDataMatrix, targetColVec);
    vector<vector<int> > regs_result(4, vector<int>());
    vector<vector<double> > regs_score(4, vector<double>());

    // 1. Regs - Least Square
	regs.doLinearRegression(0,regs_result[0],regs_score[0]);
	regs_result[0].erase(regs_result[0].begin()+top_k, regs_result[0].end());

	// 2. Regs - Ridge
    int lambda2 = atoi(argv[6]); // (1,2,3)
	regs.doLinearRegression(lambda2,regs_result[1], regs_score[1]);
	regs_result[1].erase(regs_result[1].begin()+top_k, regs_result[1].end());

    // 3. Regs - LASSO
    int lambda3 = atoi(argv[7]); // (1,2,3)
	regs.doLarsRegression(lambda3, 0, regs_result[2], regs_score[2]);
	regs_result[2].erase(regs_result[2].begin()+top_k, regs_result[2].end());

    // 4. Regs - Elastic net
    int lambda41 = atoi(argv[8]); // (1,2,3)
    int lambda42 = atoi(argv[9]); // (1,2,3)
	regs.doLarsRegression(lambda41, lambda42, regs_result[3], regs_score[3]);
	regs_result[3].erase(regs_result[3].begin()+top_k, regs_result[3].end());
    #endif

	/// Result

    fs.score_and_rank(mi_result, resultFile, "MI");

    #ifdef FS_Regs
    fs.score_and_rank(regs_result, resultFile, "Regression");
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

    // print the attributes in use
    cout << "Use " << fs.numOfUsedFeatures() << " Features: " << endl;
    resultFile << "Use " << fs.numOfUsedFeatures() << " Features: " << endl;
    for(unsigned i=0; i<fs.numOfUsedFeatures(); i++){
        cout << i+1 << ": " << fs.getAttrName(fs.useFeatureId(i)) << endl;
        resultFile << i+1 << ", " << fs.getAttrName(fs.useFeatureId(i)) << endl;
    }
    cout << endl;
    resultFile << endl;

    output_select_result(fs, "JMI", mi_result[0], resultFile);
    output_select_result(fs, "MRMR", mi_result[1], resultFile);
    output_select_result(fs, "CMIM", mi_result[2], resultFile);
    output_select_result(fs, "DISR", mi_result[3], resultFile);
    output_select_result(fs, "CondMI", mi_result[4], resultFile);
    output_select_result(fs, "ICAP", mi_result[5], resultFile);
    output_select_result(fs, "MIM", mi_result[6], resultFile);
    output_select_result(fs, "CHI", mi_result[7], resultFile);
    output_select_result(fs, "FCBF", mi_result[8], resultFile);

    output_select_result(fs, "Least Square", regs_result[0], resultFile);
    for(unsigned i=0; i<regs_score[0].size(); i++)
        resultFile << regs_score[0][i] << endl;
    output_select_result(fs, "Ridge", regs_result[1], resultFile);
    for(unsigned i=0; i<regs_score[1].size(); i++)
        resultFile << regs_score[1][i] << endl;
    output_select_result(fs, "LASSO", regs_result[2], resultFile);
    for(unsigned i=0; i<regs_score[2].size(); i++)
        resultFile << regs_score[2][i] << endl;
    output_select_result(fs, "Elastic net", regs_result[3], resultFile);
    for(unsigned i=0; i<regs_score[3].size(); i++)
        resultFile << regs_score[3][i] << endl;

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
    for(unsigned ftid=0; ftid<fs.numOfUsedFeatures(); ftid++){
        disctFile << fs.getAttrName(fs.useFeatureId(ftid)) << ",";
    }
    disctFile << endl;

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

