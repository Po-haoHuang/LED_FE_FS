#ifndef FeatureSelection_H
#define FeatureSelection_H

#include <string>
#include <vector>
#include <ostream>


using std::string;
using std::vector;
using std::ostream;

struct ScoreElem{
    double score;
    int id;
    ScoreElem(): score(0), id(0){}
    ScoreElem(double sc, int i): score(sc), id(i){}
    bool operator< (const ScoreElem &y) const { return this->score < y.score;}
};

class FeatureSelection{
public:
    // constructor
    FeatureSelection();
    virtual ~FeatureSelection();
    bool init(string FE_fileName);

    // data access
    vector<vector<double> > &allFeatureData(){return featureData;}
    void allSelectedData(vector<vector<double> > &storage);
    bool getAttrCol(string attrName, vector<double> &colVec);
    bool getAttrCol(unsigned attrId, vector<double> &colVec);

    // data info
    unsigned numOfFeatures(){return attrNameVec.size();}
    unsigned numOfUsedFeatures(){return useFeatureId_.size();}
    unsigned numOfSamples(){return featureData.size();}
    int attrId(string attrName);
    int useFeatureId(int i){return useFeatureId_[i];}
    string getAttrName(int attrId){return attrNameVec[attrId];}

    // feature excluding
    void excludeAttr(string attrName);
    void excludeZeroColumn();

    // discretization method
    bool disct_manual(vector<double> &inFeatureData, vector<int> &discreteData, vector<double>& cutPoints);
    bool disct_ew(vector<vector<int> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr=NULL);
    bool disct_ew_cycle(vector<vector<int> >& discreteData, int partitionNum);

    // selection algorithm
    void JMI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void MRMR(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix,double *classColumn, vector<int> &outputId);
    void CMIM(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void DISR(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void CHI(int top_k, int noOfSamples, int noOfFeatures,double *featureMatrix, double *classColumn, vector<int> &outputId);
    void CondMI(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void ICAP(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);
    void MIM(int top_k, int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, vector<int> &outputId);

    void FCBF(int noOfSamples, int noOfFeatures, double *featureMatrix, double *classColumn, double threshold, vector<int> &outputId);

private:
    vector<vector<double> > featureData;  // contains all feature data
    vector<int> useFeatureId_; // used
    vector<double> featureDataCycle;
    vector<string> attrNameVec;
    void csvSplit(string s, const char delimiter, vector<string> &value);
    double chi2f(vector<double> &feature, vector<double> &label);  // for CHI
    double SU(double *dataVector1, double *dataVector2, int vectorLength);  // for FCBF
};

#endif // FeatureSelection_H
