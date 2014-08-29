#include "..\include\CSV_Filter.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

CSV_Filter::CSV_Filter(string filename)
{
    ifstream inFile(filename.c_str());
    if(!inFile){
        cout << "Input File " << filename << " error!" << endl;
        return;
    }

    // read attribute name
    string lineString;
    getline(inFile, lineString);
    csvParser(lineString, allTitle_);
    if(allTitle_.back().size()==0)
        allTitle_.pop_back(); // remove redundant value
    cout << allTitle_.size() << endl;

    // read data by line
    vector<double> lineValue;
    while(getline(inFile, lineString)){
        lineValue.clear();
        csvParser(lineString, lineValue);
        allData_.push_back(lineValue);
    }
    cout << allData_.size() << endl;
    inFile.close();
}

void CSV_Filter::get_data(vector<vector<double> > &usedData)
{
    for(unsigned i=0; i<allData_.size(); i++){
        vector<double> oneLineData;
        for(unsigned j=0; j<useId_.size(); j++){
            oneLineData.push_back(allData_[i][useId_[j]]);
        }
        usedData.push_back(oneLineData);
    }
}

void CSV_Filter::get_title(vector<string> &usedTitle)
{
    for(unsigned i=0; i<useId_.size(); i++){
        usedTitle.push_back(allTitle_[useId_[i]]);
    }
}

void CSV_Filter::use_list_file(string filename)
{
    ifstream inFile(filename);
    if(!inFile) return;
    string lineString;
    while(getline(inFile, lineString)){
        use(lineString);
    }
}

void CSV_Filter::exclude_list_file(string filename)
{
    ifstream inFile(filename);
    if(!inFile) return;
    string lineString;
    while(getline(inFile, lineString)){
        exclude(lineString);
    }
}

void CSV_Filter::use_all()
{
    useId_.clear();
    for(unsigned i=0; i<allTitle_.size(); i++){
        useId_.push_back(i);
    }
}

void CSV_Filter::csvParser(string lineString, vector<double> &value)
{
    size_t start=0;
    size_t end=lineString.find_first_of(",");
    while (end <= string::npos){
        value.push_back(atof((lineString.substr(start, end-start)).c_str()));
	    if (end == string::npos)
	    	break;
    	start=end+1;
    	end = lineString.find_first_of(",", start);
    }
}

void CSV_Filter::csvParser(string lineString, vector<string> &value)
{
    size_t start=0;
    size_t end=lineString.find_first_of(",");
    while (end <= string::npos){
        string str;
        if(lineString[0]=='"'){
            str = lineString.substr(start+1, end-start-2);
        }else{
            str = lineString.substr(start, end-start);
        }
        value.push_back(str);
        if (end == string::npos)
	    	break;
    	start=end+1;
    	end = lineString.find_first_of(",", start);
    }
}

void CSV_Filter::exclude(string attributeName)
{
    if(attributeName.size()==0) return;
    for(unsigned i=0; i<useId_.size(); i++){
        if(allTitle_[useId_[i]].find(attributeName)!=string::npos){  // if attribute name match
            useId_[i]=-1;  // exclude
        }
    }
    // remove unused feature id
    vector<int> newUseId;  // temporary use only
    for(unsigned i=0; i<useId_.size(); i++){
        if(useId_[i]!=-1) newUseId.push_back(useId_[i]);
    }
    useId_.swap(newUseId);
}

void CSV_Filter::use(string attributeName)
{
    if(attributeName.size()==0) return;
    if(attributeName=="ALL"){
       use_all();
       return;
    }
    for(unsigned i=0; i<allTitle_.size(); i++){
        if(allTitle_[i].find(attributeName)!=string::npos){  // if attribute name match
            bool repeated = false;
            for(unsigned j=0; j<useId_.size(); j++){
                if(allTitle_[i]==allTitle_[useId_[j]]){ // already contained
                    repeated = true;
                    break;
                }
            }
            if(!repeated) useId_.push_back(i); // add the feature
        }
    }
}
