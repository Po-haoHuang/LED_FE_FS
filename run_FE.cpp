#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<gsl/gsl_sort.h>
#include<gsl/gsl_statistics_double.h>
#include<iostream>
#include<vector>
#include<string>
#include <fstream>
#include <dirent.h>
//include file processing part
#include "include/FileData.h"
#include "include/DataBase.h"
#include "include/CycleData.h"

unsigned attrNum;
unsigned dataSize;
const unsigned featureNum = 12;
unsigned fileNum;
unsigned segNum;
vector<FileData> fileDataVector;
using namespace std;

const string cycleListFileName = "use_file_list.csv";
const string dataDir = ".\\dp_variable_selection\\";
const string dataSelectionDir = ".\\dp_variable_selection\\selection\\";

//Function declaration
//For FE
double *FeatureExtraction(double* cleanData);
int runFeatureExtraction();
void GenParaFeatureNameSet();
double *FeatureExtraction_seg(unsigned chunkSize,double* cleanData);
void GenParaFeatureNameSet_seg();

vector<vector<double> > rowData;

//Feature_name list
const char featureName[][30] ={"peak(not implement)","peak(not implement)",
"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};

//type for segmentation 1:no segmentation  2:segmentation
typedef enum {disable, enable}segmentPara;
segmentPara seg = disable;

//ParaFeatureNameSet
vector<vector<vector<string> > > ParaFeatureNameSet;

int main(int argc, char *argv[]){

	//Start File IO
	
    // database
    DataBase db;

    if(argc==2 && strcmp(argv[1],"init")==0){
        db.init(dataDir, dataSelectionDir,cycleListFileName);
		db.init(dataSelectionDir,cycleListFileName);   // copy listed files to run directory
    }else{
        db.init(dataSelectionDir,cycleListFileName);   // use filtered files directory as working directory
    }

    if(!db.valid()){
        cout << "Database initializing failed." << endl;
        return 1;
    }

    db.printList();

    // Set cycle range
    double cycleBegin, cycleEnd;
    cout << "Set cycle begin: ";
    cin >> cycleBegin;
    cout << "Set cycle end: ";
    cin >> cycleEnd;
    cout << endl;
    bool extractSuccess = db.extract(cycleBegin, cycleEnd); // start extracting file data

    if(!extractSuccess){
        cout << "Extracting failed." << endl;
        return 1;
    }
    // get all file data

    bool getAllFileSuccessful = db.getAllFileData(fileDataVector);
    if(getAllFileSuccessful){
        cout << endl << "There are " << fileDataVector.size() << " files extracted:";
        cout << ", containing: ";
        for(unsigned i = 0; i< fileDataVector.size(); i++){
            cout << fileDataVector[i].id << " \t";
        }
    }else{
        cout << "No file extracted." << endl;
    }
    cout<<endl;
	//start FE
	fileNum = fileDataVector.size();
	attrNum = fileDataVector[0].dataVector[0].size();


	
	//checking argv input
	if(argc > 3){//too many arguments
		cerr<<"Too many arguments.\n";
		return -1;
	}

	else if(argc < 2){//too few arguments
		cerr<<"Too few arguments.\n";
		return -1;
	}
	else{

		if(atoi(argv[1]) == 0){
			if(argc == 3){//if disable need 2 arguments
				cerr<<"No need to enter segmentation number when disabled.\n";
				return -1;
			}
			else
			seg = (segmentPara)atoi(argv[1]);
		}
		else if(atoi(argv[1]) == 1 ){
			if(argc == 2){//if enable need 3 arguments
				cerr<<"Need to enter segmentation number when enabled.\n";
				return -1;
			}
			else{
			seg = (segmentPara)atoi(argv[1]);
			segNum = atoi(argv[2]);
			}
		}
		else{//Other error of segmentation option
			cerr<<"Segmentation option value error."<<endl
			<<"0 for disable."<<endl<<"1 for enable.";
			return -1;
		}
	}
	//run FE
	runFeatureExtraction();
	system("pause");
	return 0;
}

int runFeatureExtraction(){
	
	//Initialization
	double* tempResult;
	vector<double>temp;
	vector<vector<vector<double> > > totalResult;
	vector<vector<double> > singleResult;
	double* temp_array;
	FILE* fout = fopen("Output.csv","w+");

	//enable or disable
	switch (seg){
		case disable://no segmentation
		
			//Initialization

			totalResult.clear();
			singleResult.resize(featureNum);
			
			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first file
			while first call FE.
			Result:File_1_Attr_1_firstfeature - File_1_Attr_1_lasttfeature*/
			for(unsigned k = 0;k < fileNum;k++){
				rowData = fileDataVector[k].dataVector;
				dataSize = fileDataVector[k].dataVector.size();
				temp.clear();
				temp.resize(dataSize);
				singleResult.clear();
				singleResult.resize(featureNum);
				for(unsigned j = 0;j < attrNum;j++){
							
					for(unsigned i = 0;i < dataSize;i++)
						temp[i] = rowData[i][j];
	 				temp_array = &temp[0];
					tempResult = FeatureExtraction(temp_array);
					for(unsigned i = 0;i < featureNum;i++){
						singleResult[i].push_back(tempResult[i]);
					}
				}
				totalResult.push_back(singleResult);
				if( k!=0 && (k%20)==0 )
					cout<<"Computing "<<k<<" files."<<endl;
			}
			cout<<"Computing done."<<endl;
			//Generate the correspnding nametags for FE output
			GenParaFeatureNameSet();
			cout<<"Generating nametags done."<<endl;
			
			cout<<"Output processing..."<<endl;
			//output
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout,"%s%d%s,","Attr_",j,featureName[i]);
				}
			}
			fprintf(fout,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				for(unsigned j = 0;j < attrNum;j++){
					
					/*for(unsigned j = 0;j < attrNum;j++){

						fprintf(fout,"%s",ParaFeatureNameSet[k][i][j].c_str());
						if(j != (attrNum-1))
						fprintf(fout,",");
						
					}*/
					//fprintf(fout,"\n");
					for(unsigned i = 0;i < featureNum;i++){

						fprintf(fout,"%lf,",totalResult[k][i][j]);
						/*if(j != (attrNum-1))
						fprintf(fout,",");*/

					}
					
				}
				fprintf(fout,"\n");
			}
			cout<<"Output done."<<endl;
			fclose(fout);
			break;
			
		case enable://segmentation enabled
			dataSize = fileDataVector[0].dataVector.size();

			for(unsigned k = 1;k < fileNum;k++)
				dataSize = fileDataVector[k].dataVector.size() > dataSize?
				dataSize : fileDataVector[k].dataVector.size();

			//check if segNum is valid
			while(segNum > dataSize || segNum == 0){
            	cerr<<"Segmentation number error."<<endl<<
				"Please enter a new Segmentation number:(1 ~ datasize)";
				scanf("%d",&segNum);

			}
			
			//Initialization
			totalResult.clear();
			unsigned tempSize = 0;


			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first segment
			of file while first call FE.
			Result:File_1_Seg_1_Attr_1_firstfeature - File_1_Seg_1_Attr_1_lasttfeature*/

			for(unsigned k = 0;k < fileNum;k++){
				rowData = fileDataVector[k].dataVector;
				dataSize = fileDataVector[k].dataVector.size();
				singleResult.clear();
				singleResult.resize(featureNum*segNum);
				//cout<<"Computing "<<k<<" files."<<endl;


				for (unsigned l = 0;l < segNum;l++){
					//cout<<"seg"<<l<<endl;
					for(unsigned j = 0;j < attrNum;j++){
						//cout<<"attr"<<j<<endl;
						tempSize = 0;
						temp.clear();
						//temp.resize(dataSize);
						if(l != segNum - 1){
							for(unsigned i = round((float)dataSize/(float)segNum*(float)l);
							i <= round((float)dataSize/(float)segNum*(float)(l+1)-1.0);i++){
								tempSize++;
								temp.push_back(rowData[i][j]);
							}
						}
						
						else{
							for(unsigned i =  round((float)dataSize/(float)segNum*(float)l);
							i < dataSize;i++){
								//cout<<i<<endl;
								tempSize++;
								temp.push_back(rowData[i][j]);
							}
							//cout<<"f";
						}

						temp_array = &temp[0];
						tempResult = FeatureExtraction_seg(tempSize,temp_array);

						/*#ifdef printTempresult
						for(unsigned i = 0;i < featureNum;i++)
						cout<<tempResult[i]<<endl;
						#endif*/
						for(unsigned i = 0+l*featureNum;i < 0+l*featureNum+featureNum;i++){
							singleResult[i].push_back(tempResult[i-l*featureNum]);
						}
					}
				}
				//cout<<"222";
				totalResult.push_back(singleResult);
				if( k!=0 && (k%20)==0 )
					cout<<"Computing "<<k<<" files."<<endl;
			}
			cout<<"Computing done."<<endl;
			//Generate the correspnding nametags for FE output
			GenParaFeatureNameSet_seg();
			cout<<"Generating nametags done."<<endl;

			cout<<"Output processing..."<<endl;
			
			//output
			FILE* fout2 = fopen("Output2.csv","w+");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					for(int k = 0;k < segNum;k++){
						fprintf(fout,"%s%d%s%d_%s,","Attr_",j,"_Seg_",k,featureName[i]);
					}
				}
			}
			fprintf(fout,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				for(unsigned j = 0;j < attrNum;j++){
					/*for(unsigned j = 0;j < attrNum;j++){
						fprintf(fout,"%s",ParaFeatureNameSet[k][i][j].c_str());
						if(j != (attrNum-1))
						fprintf(fout,",");
					}
					fprintf(fout,"\n");*/
					for(unsigned i = 0;i < featureNum;i++){
						for(unsigned l = 0;l < segNum;l++)
							fprintf(fout,"%lf,",totalResult[k][l*featureNum+i][j]);
						/*if(j != (attrNum-1))
						fprintf(fout,",");*/
					}
				}
				fprintf(fout,"\n");

			}
			fclose(fout);
			
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout2,"%s%d%s,","Attr_",j,featureName[i]);
				}
			}
			fprintf(fout,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				for(unsigned l = 0;l < segNum;l++){
					for(unsigned j = 0;j < attrNum;j++){
					/*for(unsigned j = 0;j < attrNum;j++){
						fprintf(fout,"%s",ParaFeatureNameSet[k][i][j].c_str());
						if(j != (attrNum-1))
						fprintf(fout,",");
					}
					fprintf(fout,"\n");*/
						for(unsigned i = 0;i < featureNum;i++){
							fprintf(fout2,"%lf,",totalResult[k][l*featureNum+i][j]);
						/*if(j != (attrNum-1))
						fprintf(fout,",");*/
						}
					}
					fprintf(fout2,"\n");
				}
			}
			fclose(fout2);


			
			cout<<"Output done."<<endl;
			break;
		
	}

}

double *FeatureExtraction(double* cleanData){
	//check if it is empty not implement yet

	
	unsigned length = dataSize;//not implement yet

	static double f[featureNum];
	double tempData[dataSize];
	
	
	//tempData = cleanData^2;
	for(unsigned i = 0;i < length;i++)
		tempData[i] = pow(cleanData[i], 2);		
		
	f[0] = 0;//peak in matlab not implement
	f[1] = 0;	
	//three para for gsl func (data_array,element size(#double),#elements)	
	f[2] = gsl_stats_mean (cleanData, 1, length);
	f[3] = gsl_stats_variance (cleanData, 1, length);
	f[4] = gsl_stats_skew (cleanData, 1, length);
	f[5] = gsl_stats_kurtosis (cleanData, 1, length);
	f[6] = sqrt (gsl_stats_mean (tempData, 1, length));//rms
	f[7] = gsl_stats_max (cleanData, 1, length);
	f[8] = gsl_stats_min (cleanData, 1, length);
	f[9] = f[7] - f[8];//range
	gsl_sort (cleanData, 1, length);//sort before iqr
	//iqr
	f[10] = gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
			- gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25);
	f[11] = gsl_stats_sd (cleanData, 1, length);//std
	for(unsigned i = 0;i < featureNum;i++){//set 0 if nan
		if(isnan(f[i]))
			f[i] = 0;
	}
	return f;
	
	
}

void GenParaFeatureNameSet(){
	ParaFeatureNameSet.resize(fileNum);
	for(unsigned k = 0;k < fileNum;k++)
		ParaFeatureNameSet[k].resize(featureNum);
	for(unsigned k = 0;k < fileNum;k++)
		for(unsigned i = 0;i < featureNum;i++)
			ParaFeatureNameSet[k][i].resize(attrNum);
	string tempStr;
	char buffer[50];
	for(unsigned k = 0;k < fileNum;k++){
		for(unsigned i = 0;i < featureNum;i++){
			for(unsigned j = 0;j < attrNum;j++){
				tempStr.clear();
				tempStr = "File_";
				sprintf(buffer,"%d",k);
				tempStr += buffer;
				tempStr += "_Attr_";
				sprintf(buffer,"%d",j);
				tempStr += buffer;
				tempStr += "_";
				tempStr += featureName[i];
				ParaFeatureNameSet[k][i][j] = tempStr;

			}
		}
	}
}


double *FeatureExtraction_seg(unsigned chunkSize,double* cleanData){
//check if it is empty not implement yet

	
	unsigned length = chunkSize;//not implement yet

	static double f[featureNum];
	double tempData[chunkSize];
	
	
	//tempData = cleanData^2;
	for(unsigned i = 0;i < length;i++)
		tempData[i] = pow(cleanData[i], 2);		
		
	f[0] = 0;//peak in matlab not implement
	f[1] = 0;	
	//three para for gsl func (data_array,element size(#double),#elements)	
	f[2] = gsl_stats_mean (cleanData, 1, length);
	f[3] = gsl_stats_variance (cleanData, 1, length);
	f[4] = gsl_stats_skew (cleanData, 1, length);
	f[5] = gsl_stats_kurtosis (cleanData, 1, length);
	f[6] = sqrt (gsl_stats_mean (tempData, 1, length));//rms
	f[7] = gsl_stats_max (cleanData, 1, length);
	f[8] = gsl_stats_min (cleanData, 1, length);
	f[9] = f[7] - f[8];//range
	gsl_sort (cleanData, 1, length);//sort before iqr
	//iqr
	f[10] = gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
			- gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25);
	f[11] = gsl_stats_sd (cleanData, 1, length);//std
	for(unsigned i = 0;i < featureNum;i++){//set 0 if nan
		if(isnan(f[i]))
			f[i] = 0;
	}

	return f;
	
}

void GenParaFeatureNameSet_seg(){
	ParaFeatureNameSet.resize(fileNum);
	for(unsigned k = 0;k < fileNum;k++)
		ParaFeatureNameSet[k].resize(featureNum*segNum);
	for(unsigned k = 0;k < fileNum;k++)
		for(unsigned i = 0;i < featureNum*segNum;i++)
			ParaFeatureNameSet[k][i].resize(attrNum);
	string tempStr;
	char buffer[50];
	for(unsigned k = 0;k < fileNum;k++){
		for(unsigned m = 0;m < segNum;m++ ){

			for(unsigned i = 0;i < featureNum;i++){

				for(unsigned j = 0;j < attrNum;j++){
					tempStr.clear();
					tempStr = "File_";
					sprintf(buffer,"%d",k);
					tempStr += buffer;
					tempStr += "_Seg_";
					sprintf(buffer,"%d",m+1);
					tempStr += buffer;
					tempStr += "_Attr_";
					sprintf(buffer,"%d",j);
					tempStr += buffer;
					tempStr += "_";
					tempStr += featureName[i];
					ParaFeatureNameSet[k][i+m*featureNum][j] = tempStr;
				}

			}
		}
	}
}
