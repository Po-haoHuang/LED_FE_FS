#ifndef FeatureSelection_H
#define FeatureSelection_H

#include <string>
#include <vector>


using std::string;
using std::vector;

class FeatureSelection{
public:
    FeatureSelection();
    bool init(string FE_fileName);

    // data access
    vector<vector<double> > &allFeatureData(){return featureData;}
    bool getAttrCol(string attrName, vector<double> &colVec);
    int idOfAttr(string attrName);
    unsigned featureNumber(){return attrNameVec.size();}
    string attrTitle();
    string getAttrName(int attrId){return attrNameVec[attrId];}
    virtual ~FeatureSelection();

    // discretization method
    bool disct_manual(vector<double> &inFeatureData, vector<int> &discreteData, vector<double>& cutPoints);
    bool disct_ew(vector<vector<int> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr=NULL);
    bool disct_ew_cycle(vector<vector<int> >& discreteData, int partitionNum);

    // selection algorithm
    double* JMI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, double *outputFeatures);
    double* MRMR(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix,double *classColumn, double *outputFeatures);
    double* CHI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, double *outputFeatures, int *outputFeatureNum);

private:
    vector<vector<double> > featureData;  // main feature data container
    vector<double> featureDataCycle;
    vector<string> attrNameVec;
    void csvSplit(string s, const char delimiter, vector<string> &value);
    double chi2f(vector<double> &feature, vector<double> &label);
};

#endif // FeatureSelection_H
