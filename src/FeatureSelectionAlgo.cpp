#include "FeatureSelection.h"

#include "../include/FEAST/FSAlgorithms.h"
#include "../include/FEAST/FSToolbox.h"

/* MIToolbox includes */
#include "../include/FEAST/MutualInformation.h"
#include "../include/FEAST/ArrayOperations.h"
#include "../include/FEAST/Entropy.h"

#include <vector>
#include <algorithm>
#include <iostream>

#include "../include/MatrixOp.h"

using std::cout;
using std::endl;
using std::vector;
using std::swap;

double* FeatureSelection::JMI(int k, int noOfSamples, int noOfFeatures,double *featureMatrix,
                              double *classColumn, double *outputFeatures)
{
    /*holds the class MI values*/
    double *classMI = (double *)calloc(noOfFeatures,sizeof(double));

    char *selectedFeatures = (char *)calloc(noOfFeatures,sizeof(char));

    /*holds the intra feature MI values*/
    int sizeOfMatrix = k*noOfFeatures;
    double *featureMIMatrix = (double *)calloc(sizeOfMatrix,sizeof(double));

    double maxMI = 0.0;
    int maxMICounter = -1;

    double **feature2D = (double**) calloc(noOfFeatures,sizeof(double*));

    double score, currentScore; //totalFeatureMI;
    int currentHighestFeature;

    double *mergedVector = (double *) calloc(noOfSamples,sizeof(double));

    int arrayPosition;
    double mi;

    int i,j,x;

    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    for (i = 0; i < sizeOfMatrix; i++) {
        featureMIMatrix[i] = -1;
    }/*for featureMIMatrix - blank to -1*/


    for (i = 0; i < noOfFeatures; i++) {
        /*calculate mutual info
        **double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);
        */
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);
        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    /*****************************************************************************
    ** We have populated the classMI array, and selected the highest
    ** MI feature as the first output feature
    ** Now we move into the JMI algorithm
    *****************************************************************************/

    for (i = 1; i < k; i++) {
        score = 0.0;
        currentHighestFeature = 0;
        currentScore = 0.0;
        //totalFeatureMI = 0.0;

        for (j = 0; j < noOfFeatures; j++) {

            /*if we haven't selected j*/
            if (selectedFeatures[j] == 0) {
                currentScore = 0.0;
                //totalFeatureMI = 0.0;

                for (x = 0; x < i; x++) {
                    arrayPosition = x*noOfFeatures + j;
                    if (featureMIMatrix[arrayPosition] == -1) {
                        mergeArrays(feature2D[(int) outputFeatures[x]], feature2D[j],mergedVector,noOfSamples);
                        /*double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);*/
                        mi = calculateMutualInformation(mergedVector, classColumn, noOfSamples);

                        featureMIMatrix[arrayPosition] = mi;
                    }/*if not already known*/
                    currentScore += featureMIMatrix[arrayPosition];
                }/*for the number of already selected features*/

                if (currentScore > score) {
                    score = currentScore;
                    currentHighestFeature = j;
                }
            }/*if j is unselected*/
        }/*for number of features*/

        selectedFeatures[currentHighestFeature] = 1;
        outputFeatures[i] = currentHighestFeature;

    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(feature2D);
    FREE_FUNC(featureMIMatrix);
    FREE_FUNC(mergedVector);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    feature2D = NULL;
    featureMIMatrix = NULL;
    mergedVector = NULL;
    selectedFeatures = NULL;

    return outputFeatures;
}

double* FeatureSelection::MRMR(int k, int noOfSamples, int noOfFeatures, double *featureMatrix,
                               double *classColumn, double *outputFeatures)
{
    double **feature2D = (double**) checkedCalloc(noOfFeatures,sizeof(double*));
    /*holds the class MI values*/
    double *classMI = (double *)checkedCalloc(noOfFeatures,sizeof(double));
    int *selectedFeatures = (int *)checkedCalloc(noOfFeatures,sizeof(int));
    /*holds the intra feature MI values*/
    int sizeOfMatrix = k*noOfFeatures;
    double *featureMIMatrix = (double *)checkedCalloc(sizeOfMatrix,sizeof(double));

    double maxMI = 0.0;
    int maxMICounter = -1;

    /*init variables*/

    double score, currentScore, totalFeatureMI;
    int currentHighestFeature;

    int arrayPosition, i, j, x;

    for(j = 0; j < noOfFeatures; j++) {
        feature2D[j] = featureMatrix + (int)j*noOfSamples;
    }

    for (i = 0; i < sizeOfMatrix; i++) {
        featureMIMatrix[i] = -1;
    }/*for featureMIMatrix - blank to -1*/


    for (i = 0; i < noOfFeatures; i++) {
        classMI[i] = calculateMutualInformation(feature2D[i], classColumn, noOfSamples);
        if (classMI[i] > maxMI) {
            maxMI = classMI[i];
            maxMICounter = i;
        }/*if bigger than current maximum*/
    }/*for noOfFeatures - filling classMI*/

    selectedFeatures[maxMICounter] = 1;
    outputFeatures[0] = maxMICounter;

    /*************
    ** Now we have populated the classMI array, and selected the highest
    ** MI feature as the first output feature
    ** Now we move into the mRMR-D algorithm
    *************/

    for (i = 1; i < k; i++) {
        /****************************************************
        ** to ensure it selects some features
        **if this is zero then it will not pick features where the redundancy is greater than the
        **relevance
        ****************************************************/
        score = -DBL_MAX;
        currentHighestFeature = 0;
        currentScore = 0.0;
        totalFeatureMI = 0.0;

        for (j = 0; j < noOfFeatures; j++) {
            /*if we haven't selected j*/
            if (selectedFeatures[j] == 0) {
                currentScore = classMI[j];
                totalFeatureMI = 0.0;

                for (x = 0; x < i; x++) {
                    arrayPosition = x*noOfFeatures + j;
                    if (featureMIMatrix[arrayPosition] == -1) {
                        /*work out intra MI*/

                        /*double calculateMutualInformation(double *firstVector, double *secondVector, int vectorLength);*/
                        featureMIMatrix[arrayPosition] = calculateMutualInformation(feature2D[(int) outputFeatures[x]], feature2D[j], noOfSamples);
                    }

                    totalFeatureMI += featureMIMatrix[arrayPosition];
                }/*for the number of already selected features*/

                currentScore -= (totalFeatureMI/i);
                if (currentScore > score) {
                    score = currentScore;
                    currentHighestFeature = j;
                }
            }/*if j is unselected*/
        }/*for number of features*/

        selectedFeatures[currentHighestFeature] = 1;
        outputFeatures[i] = currentHighestFeature;

    }/*for the number of features to select*/

    FREE_FUNC(classMI);
    FREE_FUNC(feature2D);
    FREE_FUNC(featureMIMatrix);
    FREE_FUNC(selectedFeatures);

    classMI = NULL;
    feature2D = NULL;
    featureMIMatrix = NULL;
    selectedFeatures = NULL;

    return outputFeatures;
}

void FeatureSelection::CHI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputFeatures)
{
    vector<double> label(classColumn, classColumn + noOfSamples);
    vector<double> classScore;
    vector<int> classScoreIndex;
    for(int i=0; i<noOfFeatures; i++){
        vector<double> feature(featureMatrix + i*noOfSamples, featureMatrix + (i+1)*noOfSamples);
        double score = chi2f(feature, label);
        classScore.push_back(score);
        classScoreIndex.push_back(i);
    }
    // bubble sort
    for(unsigned i=0; i<classScore.size(); i++){
        for(unsigned j=0; j<classScore.size()-i-1; j++){
            if(classScore[j] < classScore[j+1]){
                swap(classScore[j], classScore[j+1]);
                swap(classScoreIndex[j], classScoreIndex[j+1]);
            }
        }
    }

    // Compute mutual info. If MI(f1,f2) > MI(f1,class), exclude it.
    vector<int> select(classScore.size(), 1); // initializing with 1 means selected
    for(unsigned i=0; i<classScore.size(); i++){
        for(unsigned j=i+1; j<classScore.size(); j++){
            vector<double> f1(featureMatrix + i*noOfSamples, featureMatrix + (i+1)*noOfSamples);
            vector<double> f2(featureMatrix + j*noOfSamples, featureMatrix + (j+1)*noOfSamples);
            double mi = chi2f(f1,f2);
            if(mi > classScore[j]){
                select[j] = 0;  // excluded
            }
        }
    }
    for(unsigned i=0; i<classScore.size(); i++){
        if(select[i]==1){
            outputFeatures.push_back(classScoreIndex[i]);
        }
    }
}

// return the score of column data ( 966 x 1)
double FeatureSelection::chi2f(vector<double> &feature, vector<double> &label)
{
    vector<double> classL; // label's level
    for(unsigned i=0; i<feature.size(); i++){
        if(find(classL.begin(), classL.end(), label[i])==classL.end()){ // not found, so add it
            classL.push_back(label[i]);
        }
    }
    sort(classL.begin(), classL.end());

    vector<double> member; // feature's level
    for(unsigned i=0; i<feature.size(); i++){
        if(find(member.begin(), member.end(), feature[i])==member.end()){ // not found, so add it
            member.push_back(feature[i]);
        }
    }
    sort(member.begin(), member.end());

    vector<double> n_fstar;  // the number of member
    for(unsigned i=0; i<member.size(); i++){
        n_fstar.push_back(count(feature.begin(), feature.end(), member[i]));
    }
    matrixColRepeat(n_fstar, classL.size());

    vector<double> ct(classL.size()*member.size(), 0); // the number of corresponding member of same index
    for(unsigned i=0; i<member.size(); i++){
        for(unsigned j=0; j<feature.size(); j++){
            if(feature[j]==member[i]){  // index matched with member
                for(unsigned k=0; k<classL.size(); k++){  // search for corresponding class
                    if(label[j]==classL[k]){
                        ct[i*classL.size()+k]++;
                        break;
                    }
                }
            }
        }
    }

    vector<double> n_star;
    matrixColSum(n_star, ct, classL.size());
    matrixRowRepeat(n_star, member.size());

    vector<double> mu_i, mu_i_tmp;
    elemMul(mu_i_tmp, n_star, n_fstar);
    elemDividedByConst(mu_i, mu_i_tmp, feature.size());

    vector<double> part, part_tmp;
    elemSub(part,ct,mu_i);
    elemSquare(part_tmp,part);
    elemDiv(part, part_tmp, mu_i);

    vector<double> negPos;
    matrixRowSum(negPos,  part, member.size());

    vector<double> resultSum;
    matrixRowSum(resultSum, negPos, 1);
    double chi = resultSum[0];
    return chi;
}



void FeatureSelection::FCBF(int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, double threshold, vector<int> &outputId)
{
    vector<double> classScore;
    vector<int> classScoreIndex;
    for(unsigned i=0; i<noOfFeatures; i++) {
        double score = SU(featureMatrix + i*noOfSamples, classColumn, noOfSamples);
        classScore.push_back(score);
        classScoreIndex.push_back(i);
    }
    // bubble sort (descent)
    for(unsigned i=0; i<classScore.size(); i++) {
        for(unsigned j=0; j<classScore.size()-i-1; j++) {
            if(classScore[j] < classScore[j+1]) {
                swap(classScore[j], classScore[j+1]);
                swap(classScoreIndex[j], classScoreIndex[j+1]);
            }
        }
    }
    // Compute mutual info. If MI(f1,f2) > MI(f1,class), exclude it.
    vector<int> select(classScore.size(), 1); // initializing with 1 means selected
    for(unsigned i=0; i<classScore.size(); i++) {
        for(unsigned j=i+1; j<classScore.size(); j++) {
            double score = SU(featureMatrix + i*noOfSamples, featureMatrix + j*noOfSamples, noOfSamples);
            if(score > classScore[j]) {
                select[j] = 0;  // excluded
            }
        }
    }

    for(unsigned i=0; i<classScore.size(); i++) {
        if(select[i]==1) {
            outputId.push_back(classScoreIndex[i]);
        }
    }
}

double FeatureSelection::SU(double *dataVector1, double *dataVector2, int vectorLength)
{
    double hX = calculateEntropy(dataVector1, vectorLength);
    double hY = calculateEntropy(dataVector2, vectorLength);
    double iXY = calculateMutualInformation(dataVector1, dataVector2, vectorLength);
    double score = (2 * iXY) / (hX + hY);
    return score;
}
