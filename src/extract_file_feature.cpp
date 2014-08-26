#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "../include/csv.h"

using namespace std;

void csvValueSplit(string s, const char delimiter, vector<double> &lineValue)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    while (end <= string::npos){
        lineValue.push_back(atof((s.substr(start, end-start)).c_str()));
	    if (end == string::npos)
	    	break;
    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
}

void csvStrSplit(string s, const char delimiter, vector<string> &value)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);

    while (end <= std::string::npos){
        value.push_back(s.substr(start, end-start));
        if (end == std::string::npos)
	    	break;
    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
}

bool extract_file_feature(string fileName, vector<string> usedFeatureName, vector<vector<double> > &matrix)
{
    // open file
    ifstream inFile(fileName.c_str(), ios::in);
    if(!inFile) {
        cout << "Error! Cannot open file: " << fileName << endl;
        return false;
    }
    string lineBuffer;
    vector<string> title;

    // read the first line (title)
    getline(inFile, lineBuffer);
    csvStrSplit(lineBuffer, ',', title);

    // locate feature index
    vector<int> featureIndex;
    for(unsigned i=0; i<usedFeatureName.size(); i++){
        for(unsigned j=0; j<title.size(); j++){
            if(usedFeatureName[i] == title[j]){
                featureIndex.push_back(j);
                break;
            }
        }
    }

    // read data by line, 2 method implement
    #ifndef USE_FAST_CSV
    while(getline(inFile, lineBuffer)){
        vector<double> lineValue, lineValueP;
        csvValueSplit(lineBuffer, ',', lineValue);
        for(unsigned i=0; i<featureIndex.size(); i++){
            lineValueP.push_back(lineValue[featureIndex[i]]);
        }
        matrix.push_back(lineValueP);
    }
    #endif
    inFile.close();

    #ifdef USE_FAST_CSV
    csvIO::LineReader in(fileName);
    in.next_line(); // discard attribute title
    char *linePtr;
    while( linePtr = in.next_line()){
        vector<double> lineValue, lineValueP;
        csvValueSplit(string(linePtr), ',', lineValue);
        for(unsigned i=0; i<featureIndex.size(); i++){
            lineValueP.push_back(lineValue[featureIndex[i]]);
        }
        matrix.push_back(lineValueP);
    }
    #endif
    return true;
}

int main()
{
    vector<string> usedFeatureName;
    usedFeatureName.push_back("Position_max");
    usedFeatureName.push_back("Filter.press_mean");

    vector<vector<double> > matrix;

    extract_file_feature("FSo_SelectedData.csv", usedFeatureName, matrix);

    cout << "First 10 samples:" << endl;
    for(unsigned i=0; i<usedFeatureName.size(); i++){
        cout << usedFeatureName[i] << " \t";
    }
    cout << endl;
    for(unsigned i=0; i<matrix.size() && i<10; i++){
        for(unsigned j=0; j<matrix[i].size(); j++){
            cout << matrix[i][j] << " \t";
        }
        cout << endl;
    }
    return 0;
}
