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
        bool init(string fileName);
        bool mamual_discrete(vector<vector<double> >&inFeatureData, vector<vector<int> >& discreteData, vector<double>& cutPoints);
        bool EW_discrete(vector<vector<int> >& discreteData, int partitionNum, vector<vector<double> >*inDataPtr=NULL);
        bool cycle_EW_discrete(vector<vector<int> >& discreteData, int partitionNum);
        vector<vector<double> > &allFeatureData(){ return featureData;}
        unsigned featureNumber(){ return attrName.size();}
        virtual ~TargetLevel();
    protected:
    private:
        vector<vector<double> > featureData;
        vector<double> featureDataCycle;
        vector<string> attrName;
        void csvSplit(string s, const char delimiter, vector<string> &value);
        bool EW_discrete2(vector<vector<int> >& discreteData, vector<vector<double> >& inFeatureData, int cut);
};

#endif // TARGETLEVEL_H
