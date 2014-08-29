#ifndef CSV_FILTER_H
#define CSV_FILTER_H

#include <vector>
#include <string>

using std::vector;
using std::string;

class CSV_Filter
{
    public:
        CSV_Filter(string filename);
        void use(string attributeName);
        void use_all();
        void use_list_file(string filename);
        void exclude(string attributeName);
        void exclude_all();
        void exclude_list_file(string filename);
        void get_data(vector<vector<double> > &usedData);
        void get_title(vector<string> &usedTitle);

    private:
        string filename_;
        vector<string> allTitle_;
        vector<vector<double> > allData_;
        vector<int> useId_;
        void csvParser(string lineString, vector<string> &value);
        void csvParser(string lineString, vector<double> &value);
};

#endif // CSV_FILTER_H
