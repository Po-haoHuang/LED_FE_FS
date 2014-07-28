#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "../include/FeatureSelection.h"
#include "../include/Regression.h"

using namespace std;

const string featureDataSetFileName = "Output_noSeg.csv";
const string targetColName = "dP_Filter (X1)_max";
const string resultFileName = "FS_result.csv";
const string disctFileName = "FS_disct.csv";
const string labelsFileName = "FS_labels.csv";
const double manual_cut_points[] = {5,15,20};

struct ScoreElem{
    double score;
    int id;
    ScoreElem(): score(0), id(0){}
    ScoreElem(double sc, int i): score(sc), id(i){}
    bool operator< (const ScoreElem &y) const { return this->score < y.score;}
};


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

    // general setting
    int top_k = 15;
    vector<vector<int> > all_select_MI;
    vector<vector<int> > all_select_Regs;
    vector<vector<int> > all_select;

    // 1. MI - JMI
    vector<int> outputFeaturesJMI;
    fs.JMI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesJMI);
    cout  << "JMI select: " << endl;
    resultFile << endl << "JMI select: " << endl;
    for(unsigned i=0; i<outputFeaturesJMI.size(); i++){
        int useFtId = fs.useFeatureId(outputFeaturesJMI[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << "," << fs.getAttrName(useFtId) << endl;
    }
    all_select_MI.push_back(outputFeaturesJMI);
    all_select.push_back(outputFeaturesJMI);

    // 2. MI - MRMR
    vector<int> outputFeaturesMRMR;
    fs.MRMR(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesMRMR);
    cout << endl<< "MRMR select: " << endl;
    resultFile << endl << "MRMR select: " << endl;
    for(int i=0; i<top_k; i++){
        int useFtId = fs.useFeatureId(outputFeaturesMRMR[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }
    all_select_MI.push_back(outputFeaturesMRMR);
    all_select.push_back(outputFeaturesMRMR);

    // 3. MI - CHI
    vector<int> outputFeaturesCHI;
    fs.CHI(top_k, fs.numOfSamples(), fs.numOfUsedFeatures(), dataMatrix, labelsD, outputFeaturesCHI);
    cout << endl << "CHI select: " << endl;
    resultFile << endl << "CHI select: " << endl;
    for(unsigned i=0; i<outputFeaturesCHI.size(); i++){
        int useFtId = fs.useFeatureId(outputFeaturesCHI[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
    }
    all_select_MI.push_back(outputFeaturesCHI);
    all_select.push_back(outputFeaturesCHI);

    // 4. MI - FCBF
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
    all_select_MI.push_back(outputFeaturesFCBF);
    all_select.push_back(outputFeaturesFCBF);

    // Regression
    // usage of linear regression and ridge regression(set lambda = 1 or 2 or 3)
    vector<vector<double> > selectedDataMatrix;
    fs.allSelectedData(selectedDataMatrix);
    Regression regs;
	regs.init(selectedDataMatrix, targetColVec);

    // 5. Regs - Least Square
    vector<int> regs_result1;
	regs.doLinearRegression(0,regs_result1);
    cout << endl << "Least Square select: " << endl;
    resultFile << endl << "Least Square select: " << endl;
	for(int i = 0; i < top_k ;i++){
        int useFtId = fs.useFeatureId(regs_result1[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
	}
	all_select_Regs.push_back(regs_result1);
    all_select.push_back(regs_result1);

	// 6. Regs - Ridge
    vector<int> regs_result2;
    int lambda2 = 1; // (1,2,3)
	regs.doLinearRegression(lambda2,regs_result2);
    cout << endl << "Ridge select: " << endl;
    resultFile << endl << "Ridge select: " << endl;
	for(int i = 0; i < top_k ;i++){
        int useFtId = fs.useFeatureId(regs_result2[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
	}
	all_select_Regs.push_back(regs_result2);
    all_select.push_back(regs_result2);

    // 7. Regs - LASSO
    vector<int> regs_result3;
    int lambda3 = 1; // (1,2,3)
	regs.doLarsRegression(lambda3, 0, regs_result3);
    cout << endl << "LASSO select: " << endl;
    resultFile << endl << "LASSO select: " << endl;
	for(int i = 0; i < top_k ;i++){
        int useFtId = fs.useFeatureId(regs_result3[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
	}
	all_select_Regs.push_back(regs_result3);
    all_select.push_back(regs_result3);

    // 8. Regs - Elastic net
    vector<int> regs_result4;
    int lambda41 = 1; // (1,2,3)
    int lambda42 = 2; // (1,2,3)
	regs.doLarsRegression(lambda41, lambda42, regs_result4);
    cout << endl << "Elastic net select: " << endl;
    resultFile << endl << "Elastic net select: " << endl;
	for(int i = 0; i < top_k ;i++){
        int useFtId = fs.useFeatureId(regs_result4[i]);
        cout << i+1 << ": " << fs.getAttrName(useFtId) << endl;
        resultFile << i+1 << ","<< fs.getAttrName(useFtId) << endl;
	}
	all_select_Regs.push_back(regs_result4);
	all_select.push_back(regs_result4);

    // MI final score
    vector<ScoreElem> scoreVector_MI(fs.numOfFeatures(), ScoreElem());
    for(unsigned i=0; i<all_select_MI.size(); i++){
        for(unsigned curRank=0; curRank<all_select_MI[i].size(); curRank++){
            int id = fs.useFeatureId(all_select_MI[i][curRank]);
            scoreVector_MI[id].id = id;
            scoreVector_MI[id].score += fs.numOfUsedFeatures()-curRank;
        }
    }
    sort(scoreVector_MI.begin(), scoreVector_MI.end());
    reverse(scoreVector_MI.begin(), scoreVector_MI.end());  // descent order

    cout << fixed << setprecision(2);
    cout << endl << "Final score (MI): " << endl;
    resultFile << fixed << setprecision(2);
    resultFile << endl << "Final score (MI): " << endl;
    for(unsigned i=0; i<scoreVector_MI.size(); i++){
        double relative_score = scoreVector_MI[i].score/fs.numOfUsedFeatures()/all_select_MI.size();
        if(relative_score>0){
            cout << i+1 << ": " << relative_score*100 << "% ";
            cout << fs.getAttrName(scoreVector_MI[i].id) << endl;
            resultFile << i+1 << "," << relative_score*100 << "%,";
            resultFile << fs.getAttrName(scoreVector_MI[i].id) << endl;
        }
    }

    // Regression final score
    vector<ScoreElem> scoreVector_Regs(fs.numOfFeatures(), ScoreElem());
    for(unsigned i=0; i<all_select_Regs.size(); i++){
        for(unsigned curRank=0; curRank<all_select_Regs[i].size(); curRank++){
            int id = fs.useFeatureId(all_select_Regs[i][curRank]);
            scoreVector_Regs[id].id = id;
            scoreVector_Regs[id].score += fs.numOfUsedFeatures()-curRank;
        }
    }
    sort(scoreVector_Regs.begin(), scoreVector_Regs.end());
    reverse(scoreVector_Regs.begin(), scoreVector_Regs.end());  // descent order

    cout << fixed << setprecision(2);
    cout << endl << "Final score (Regression): " << endl;
    resultFile << fixed << setprecision(2);
    resultFile << endl << "Final score (Regression): " << endl;
    for(unsigned i=0; i<scoreVector_Regs.size(); i++){
        double relative_score = scoreVector_Regs[i].score/fs.numOfUsedFeatures()/all_select_Regs.size();
        if(relative_score>0){
            cout << i+1 << ": " << relative_score*100 << "% ";
            cout << fs.getAttrName(scoreVector_Regs[i].id) << endl;
            resultFile << i+1 << "," << relative_score*100 << "%,";
            resultFile << fs.getAttrName(scoreVector_Regs[i].id) << endl;
        }
    }

    // output selectedDataMatrix data to csv file
    ofstream selectedDataFile("FS_selectedDataMatrix.csv");
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

    // output targetColVec data to csv file
    ofstream targetColVecFile("FS_targetColVec.csv");
    for(unsigned row=0; row<targetColVec.size(); row++){
        targetColVecFile << targetColVec[row] << endl;
    }
    targetColVecFile.close();

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

