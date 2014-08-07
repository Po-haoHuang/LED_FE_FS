#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstdlib>

#include "../include/FeatureSelection.h"
#include "../include/Regression.h"

using namespace std;

void output_select_result(FeatureSelection &fs, string typeName, vector<int> &resultVec, unsigned print_n, ofstream &fout)
{
    cout << typeName << " select: " << endl;
    fout << typeName << " select: " << endl;
    for(unsigned i=0; i<resultVec.size() && i<print_n; i++){
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

string gen_filename(string prefix, int argc, char *argv[])
{
    string finalName = prefix + "_";
    string inFileName = argv[1];
    if(inFileName.find_last_of("/\\")==string::npos) inFileName = inFileName.substr(0, inFileName.find("."));
    else inFileName = inFileName.substr(inFileName.find_last_of("/\\")+1,
                                        inFileName.find(".") - inFileName.find_last_of("/\\")-1);
    finalName += inFileName;
    for(int i=2; i<argc; i++){
        finalName += string("_") + string(argv[i]);
    }
    finalName += ".csv";
    return finalName;
}

int main(int argc, char *argv[])
{
    if(argc!=12){
        cout << "Argument setting error." << endl;
        cout << "usage: FS_no_GUI.exe input_file target_feature "
             << "disct_method fcbf_thrd ridge_lambda lasso_lambda els_lambda1 els_lambda2 "
             << "print_n score_method" << endl;
        return 1;
    }

    // argument setting
    const string featureDataSetFileName = argv[1];
    const string targetColName = argv[2];
    const unsigned print_n = atoi(argv[9]);
    const bool disctByCycle = atoi(argv[10]);
    const int top_k = atoi(argv[11]);

    // set cut points (defined in the head of file)
    int partitionNum;
    int disctMethod;
    vector<double> manual_cut_points;
    process_argv(argv[3], partitionNum, disctMethod, manual_cut_points);

    // output filename
    const string resultFileName = gen_filename("FS_Result",argc, argv);
    const string disctDataFileName = gen_filename("FS_DiscretizedData",argc, argv);
    const string normDataFileName = gen_filename("FS_NormalizedData",argc, argv);
    const string detailFileName = gen_filename("FS_Details",argc, argv);

    // output result to file
    ofstream resultFile(resultFileName.c_str());

    // log the parameters
    resultFile << "Feature Selection Result - ver. 2014.08.07" << endl;
    resultFile << "Use file:," << featureDataSetFileName << endl;
    resultFile << "target:," << targetColName << endl;
    resultFile << "target discretize method:," << argv[3] << endl;
    resultFile << "FCBF threshold:," << argv[4] << endl;
    resultFile << "RIDGE lambda:," << argv[5] << endl;
    resultFile << "LASSO lambda:," << argv[6] << endl;
    resultFile << "ELS lambda 1:," << argv[7] << endl;
    resultFile << "ELS lambda 2:," << argv[8] << endl;
    resultFile << "print_n:," << argv[9] << endl;
    resultFile << "discretize by cycle:," << argv[10] << endl;
    resultFile << "top_k:," << argv[11] << endl;
    resultFile << endl;

    // main data class
    FeatureSelection fs(featureDataSetFileName);
    if(!fs.valid()){
        return 1;
    }

    // exclude some attributes
    fs.excludeAttr("dP_Filter");
    fs.excludeAttr("C5");
    fs.excludeAttr("C6");
    //fs.excludeAttr("std");
    //fs.excludeAttr("skewness");
    //fs.excludeAttr("kurtosis");
    fs.excludeZeroColumn();

    cout << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features. ";
    cout << "Use " << fs.numOfUsedFeatures() << " features for calculation." << endl << endl;
    resultFile << "Read " << fs.numOfSamples() << " samples and " << fs.numOfFeatures() << " features. ";
    resultFile << "Use " << fs.numOfUsedFeatures() << " features for calculation." << endl << endl;

    /// Discretize

    // EW_discrete for discreteData
    vector<vector<double> > discreteData;

    if(disctByCycle)
        fs.disct_ew(discreteData,partitionNum);
    else
        fs.disct_ew_cycle(discreteData,partitionNum);

    // choose target column (dp_filter_max)
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

    // change EW-discrete data to input matrix 1-D column array
    int matrixCounter=0;
    double *dataMatrix = new double[fs.numOfSamples() * fs.numOfUsedFeatures()];  // 966 * n
    for(unsigned col=0; col<fs.numOfUsedFeatures(); col++){
        for(unsigned row=0; row<fs.numOfSamples(); row++){
            dataMatrix[matrixCounter++] = discreteData[row][col];
        }
    }

    /// Algorithm
    vector<vector<int> > mi_rank(9,vector<int>());

    // 1. MI - JMI
    fs.JMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[0]);

    // 2. MI - MRMR
    fs.MRMR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[1]);

    // 3. MI - CMIM
    fs.CMIM(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[2]);

    // 4. MI - DISR
    fs.DISR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[3]);

    // 5. MI - CondMI
    fs.CondMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[4]);

    // 6. MI - ICAP
    fs.ICAP(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[5]);

    // 7. MI - MIM
    fs.MIM(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[6]);

    // 8. MI - CHI
    fs.CHI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], mi_rank[7]);

    // 9. MI - FCBF
    double fcbf_threshold = atof(argv[4]);
    fs.FCBF(fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, &labels[0], fcbf_threshold, mi_rank[8]);

    // MI result
    fs.score_and_rank_mi(mi_rank, print_n, resultFile);

    // Regression
    // usage of linear regression and ridge regression(set lambda = 1 or 2 or 3)
    vector<vector<double> > selectedDataMatrix;
    vector<vector<double> > normalizedColVec;
    fs.allSelectedData(selectedDataMatrix);
    Regression regs;
	regs.init(selectedDataMatrix, targetColVec, normalizedColVec);
    vector<vector<int> > regs_rank(4, vector<int>());
    vector<vector<double> > regs_coeff(4, vector<double>());

    // 1. Regs - Least Square
	regs.doLinearRegression(0,regs_rank[0],regs_coeff[0]);

	// 2. Regs - Ridge
    int lambda2 = atoi(argv[5]); // (1,2,3)
	regs.doLinearRegression(lambda2,regs_rank[1], regs_coeff[1]);

    // 3. Regs - LASSO
    int lambda3 = atoi(argv[6]); // (1,2,3)
	regs.doLarsRegression(lambda3, 0, regs_rank[2], regs_coeff[2]);

    // 4. Regs - Elastic net
    int lambda41 = atoi(argv[7]); // (1,2,3)
    int lambda42 = atoi(argv[8]); // (1,2,3)
	regs.doLarsRegression(lambda41, lambda42, regs_rank[3], regs_coeff[3]);

    // Regs result
    vector<vector<int> > regs_rank_exclude_LSQ(regs_rank.begin()+1, regs_rank.end());
    vector<vector<double> > regs_coeff_exclude_LSQ(regs_coeff.begin()+1, regs_coeff.end());
    double regs_threshold = 0.1;
    fs.score_and_rank_regs(regs_rank_exclude_LSQ, regs_coeff_exclude_LSQ, regs_threshold, print_n, resultFile);

    // output the attributes in use
    ofstream detailFile(detailFileName);
    detailFile << "Use " << fs.numOfUsedFeatures() << " features: " << endl;
    for(unsigned i=0; i<fs.numOfUsedFeatures(); i++){
        detailFile << i+1 << ", " << fs.getAttrName(fs.useFeatureId(i)) << endl;
    }
    cout << endl;
    detailFile << endl;

    output_select_result(fs, "JMI", mi_rank[0], print_n, resultFile);
    output_select_result(fs, "MRMR", mi_rank[1], print_n, resultFile);
    output_select_result(fs, "CMIM", mi_rank[2], print_n, resultFile);
    output_select_result(fs, "DISR", mi_rank[3], print_n, resultFile);
    output_select_result(fs, "CondMI", mi_rank[4], print_n, resultFile);
    output_select_result(fs, "ICAP", mi_rank[5], print_n, resultFile);
    output_select_result(fs, "MIM", mi_rank[6], print_n, resultFile);
    output_select_result(fs, "CHI", mi_rank[7], print_n, resultFile);
    output_select_result(fs, "FCBF", mi_rank[8], print_n, resultFile);

    output_select_result(fs, "Least Square", regs_rank[0], print_n, resultFile);
    output_select_result(fs, "Ridge", regs_rank[1], print_n, resultFile);
    output_select_result(fs, "LASSO", regs_rank[2], print_n, resultFile);
    output_select_result(fs, "Elastic net", regs_rank[3], print_n, resultFile);

    // output discretized data to csv file
    ofstream disctFile(disctDataFileName.c_str());
    disctFile << "id,";
    disctFile << targetColName << ",";  // feature title
    for(unsigned ftid=0; ftid<fs.numOfUsedFeatures(); ftid++){
        disctFile << fs.getAttrName(fs.useFeatureId(ftid)) << ",";
    }
    disctFile << endl;
    for(unsigned i=0; i<discreteData.size(); i++){  // value
        disctFile << i+1 << ",";
        disctFile << labels[i] << ",";
        for(unsigned ftid=0; ftid<fs.numOfUsedFeatures(); ftid++){
            disctFile << discreteData[i][ftid];
            if(ftid!=fs.numOfUsedFeatures()-1) disctFile << ",";
        }
        disctFile << endl;
    }
    disctFile.close();

    // output normalized data to csv file
    ofstream normFile(normDataFileName.c_str());
    normFile << "id,";
    normFile << targetColName << ",";  // print feature title
    for(unsigned ftid=0; ftid<fs.numOfUsedFeatures(); ftid++){
        normFile << fs.getAttrName(fs.useFeatureId(ftid)) << ",";
    }
    normFile << endl;
    for(unsigned i=0; i<normalizedColVec.front().size(); i++){  // print value
        normFile << i+1 << ",";
        for(unsigned j=0; j<normalizedColVec.size(); j++){
            normFile << normalizedColVec[j][i] << ",";
        }
        normFile << endl;
    }
    normFile.close();

    // output details info to csv file
    detailFile << fs.stringOut(); // cut points

    detailFile << endl << "Least Square rank:,";
    for(unsigned i=0; i<regs_rank[0].size(); i++) detailFile << fs.getAttrName(fs.useFeatureId(regs_rank[0][i])) << ",";
    detailFile << endl << "Least Square coefficients:,";
    for(unsigned i=0; i<regs_coeff[0].size(); i++) detailFile << regs_coeff[0][i] << ",";

    detailFile << endl << "Ridge rank:,";
    for(unsigned i=0; i<regs_rank[0].size(); i++) detailFile << fs.getAttrName(fs.useFeatureId(regs_rank[1][i])) << ",";
    detailFile << endl << "Ridge coefficients:,";
    for(unsigned i=0; i<regs_coeff[1].size(); i++) detailFile << regs_coeff[1][i] << ",";

    detailFile << endl << "LASSO rank:,";
    for(unsigned i=0; i<regs_rank[0].size(); i++) detailFile << fs.getAttrName(fs.useFeatureId(regs_rank[2][i])) << ",";
    detailFile << endl << "LASSO coefficients:,";
    for(unsigned i=0; i<regs_coeff[2].size(); i++) detailFile << regs_coeff[2][i] << ",";

    detailFile << endl << "Elastic net rank:,";
    for(unsigned i=0; i<regs_rank[0].size(); i++) detailFile << fs.getAttrName(fs.useFeatureId(regs_rank[3][i])) << ",";
    detailFile << endl << "Elastic net coefficients:,";
    for(unsigned i=0; i<regs_coeff[3].size(); i++) detailFile << regs_coeff[3][i] << ",";
    detailFile << endl;

    detailFile.close();

    delete [] dataMatrix;
    resultFile.close();
    return 0;
}

