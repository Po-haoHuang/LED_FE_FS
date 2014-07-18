#ifndef TARGETLEVEL_H
#define TARGETLEVEL_H

#include <string>
#include <vector>

using std::string;
using std::vector;

class TargetLevel
{
    public:
        TargetLevel();
        bool init(string FE_fileName);
        bool mamual_discrete(vector<double> &inFeatureData, vector<int> &discreteData, vector<double>& cutPoints);
        bool EW_discrete(vector<vector<int> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr=NULL);
        bool cycle_EW_discrete(vector<vector<int> >& discreteData, int partitionNum);
        vector<vector<double> > &allFeatureData(){ return featureData;}
        bool getAttrCol(string attrName, vector<double> &colVec);
        int idOfAttr(string attrName);
        unsigned featureNumber(){ return attrNameVec.size();}
        string attrTitle();
        string getAttrName(int attrId){return attrNameVec[attrId];}
        virtual ~TargetLevel();
    protected:
    private:
        vector<vector<double> > featureData;
        vector<double> featureDataCycle;
        vector<string> attrNameVec;
        void csvSplit(string s, const char delimiter, vector<string> &value);
        bool EW_discrete2(vector<vector<int> >& discreteData, vector<vector<double> >& inFeatureData, int cut);
};

#endif // TARGETLEVEL_H
