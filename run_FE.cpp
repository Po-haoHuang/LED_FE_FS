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

#define attrNum 6
#define dataSize 6
#define featureNum 12
#define fileNum 1
int segNum = 4;

using namespace std;

const string cycleListFileName = "use_file_list.csv";
const string dataDir = ".\\dp_variable_selection\\";
const string dataSelectionDir = ".\\dp_variable_selection\\selection\\";

//Function declaration
//For FE
double *FeatureExtraction(double* cleanData);
int runFeatureExtraction();
void GenParaFeatureNameSet();
double *FeatureExtraction_seg(int chunkSize,double* cleanData);
void GenParaFeatureNameSet_seg();




double rowData[6][6]={
{-5.701741013,100.016276,1199.951172,5000,5.502386154,0.009155553},
{-5.701741013,99.97558594,1199.951172,4998.779297,5.502386154,0.009155553},
{-5.671250419,100.7486979,1199.951172,4995.117188,5.502386154,0.006103702},
{-5.671250419,121.1344401,1197.387695,4732.666016,5.502386154,0.009155553},
{-5.671250419,97.57486979,748.4130859,4849.853516,5.502386154,0.009155553},
{-5.701741013,92.36653646,855.7128906,4805.908203,5.502386154,0.006103702},
};
//vector<vector<double> > rowData;
//int attrNum,dataSize;

//Feature_name list
const char featureName[][30] ={"peak(not implement)","peak(not implement)",
"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};

//type for segmentation 1:no segmentation  2:segmentation
typedef enum {disable, enable}segmentPara;
segmentPara seg = disable;

//ParaFeatureNameSet
vector<vector<vector<string> > > ParaFeatureNameSet;

int main(int argc, char *argv[]){
	/*//Start File IO
	
    // database
    DataBase db;

    if(argc==2 && strcmp(argv[1],"init")==0){
        db.copyToSelection(dataDir, dataSelectionDir, cycleListFileName);  // copy listed files to run directory
    }

    db.init(dataSelectionDir, cycleListFileName);   // use filtered files directory as working directory

    db.printList();

    // Set cycle range
    double cycleBegin, cycleEnd;
    cout << "set cycle begin: ";
    cin >> cycleBegin;
    cout << "set cycle end: ";
    cin >> cycleEnd;
    bool extractSuccess = db.extract(cycleBegin, cycleEnd); // start extracting file data

    if(!extractSuccess){
        cout << "Extracting failed." << endl;
        return 1;
    }
    
	//start FE
	/*
	rowData = fd.dataVector;
	dataSize = fd.dataVector.size();
	attrNum = fd.dataVector[0].size();
	*/
	if(argc > 4){
		cerr<<"Too many arguments.\n";
		return -1;
	}
	else if(argc < 3){
		cerr<<"Too few arguments.\n";
		return -1;
	}
	else{

		if(atoi(argv[1]) == 0){
			if(argc == 4){
				cerr<<"No need to enter segmentation number when disabled.\n";
				return -1;
			}
			else
			seg = (segmentPara)atoi(argv[1]);
		}
		else if(atoi(argv[1]) == 1 ){
			if(argc == 3){
				cerr<<"Need to enter segmentation number when enabled.\n";
				return -1;
			}
			else{
			seg = (segmentPara)atoi(argv[1]);
			segNum = atoi(argv[2]);
			}
		}
		else{
			cerr<<"Segmentation option value error."<<endl
			<<"0 for disable."<<endl<<"1 for enable.";
			return -1;
		}
	}
	runFeatureExtraction();
	system("pause");
	return 0;
}

int runFeatureExtraction(){
	
	double* tempResult;
	vector<double>temp;
	vector<vector<vector<double> > > totalResult;
	vector<vector<double> > singleResult;
	double* temp_array;
	FILE* fout = fopen("Output.csv","w+");
	//Option of segmentation
	/*cout<<"Segmentation ?"<<endl<<"Enter 0 to disable,1 to enable."<<endl;
	scanf("%1d",&seg);*/
	


	switch (seg){
		case disable://no segmentation
			//Initialization
			singleResult.clear();
			totalResult.clear();
			temp.clear();
			temp.resize(dataSize);
			singleResult.resize(featureNum);
			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first file
			while first call FE.
			Result:File_1_Attr_1_firstfeature - File_1_Attr_1_lasttfeature*/
			for(int k = 0;k < fileNum;k++){
			
				for(int j = 0;j < attrNum;j++){
							
					for(int i = 0;i < dataSize;i++)
						temp[i] = rowData[i][j];
	 				temp_array = &temp[0];
					tempResult = FeatureExtraction(temp_array);
					for(int i = 0;i < featureNum;i++){
						singleResult[i].push_back(tempResult[i]);
					}
				}
				totalResult.push_back(singleResult);
			}
			//Generate the correspnding nametags for FE output
			GenParaFeatureNameSet();
			//output
			for(int k = 0;k < fileNum;k++){
			
				for(int i = 0;i < featureNum;i++){
					
					for(int j = 0;j < attrNum;j++){
						fprintf(fout,"%s",ParaFeatureNameSet[k][i][j].c_str());
						if(j != (attrNum-1))
						fprintf(fout,",");
						
					}
					fprintf(fout,"\n");
					for(int j = 0;j < attrNum;j++){
						fprintf(fout,"%lf",totalResult[k][i][j]);
						if(j != (attrNum-1))
						fprintf(fout,",");

					}
					fprintf(fout,"\n");
				}
			}
			fclose(fout);
			break;
			
		case enable://segmentation enabled
			//check if segNum is valid
			while(segNum > dataSize || segNum <= 0){
            	cerr<<"Segmentation number error."<<endl<<
				"Please enter a new Segmentation number:(1 ~ datasize)";
				scanf("%d",&segNum);

			}
			//Initialization
			temp.clear();
			singleResult.clear();
			totalResult.clear();
			singleResult.resize(featureNum*segNum);
			int tempSize = 0;


			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first segment
			of file while first call FE.
			Result:File_1_Seg_1_Attr_1_firstfeature - File_1_Seg_1_Attr_1_lasttfeature*/

			for(int k = 0;k < fileNum;k++){
				
				for (int l = 0;l < segNum;l++){
				
					for(int j = 0;j < attrNum;j++){
						tempSize = 0;
						temp.clear();
						if(l != segNum - 1){
							for(int i = round((float)dataSize/(float)segNum*(float)l);
							i <= round((float)dataSize/(float)segNum*(float)(l+1)-1.0);i++){
								tempSize++;
								temp.push_back(rowData[i][j]);
								//cout<<i<<"\t";
							}
						}
						
						else{
							for(int i =  round((float)dataSize/(float)segNum*(float)l);
							i < dataSize;i++){
								tempSize++;
								temp.push_back(rowData[i][j]);
							}
						}

						temp_array = &temp[0];
						tempResult = FeatureExtraction_seg(tempSize,temp_array);
						#ifdef printTempresult
						for(int i = 0;i < featureNum;i++)
						cout<<tempResult[i]<<endl;
						#endif
						for(int i = 0+l*featureNum;i < 0+l*featureNum+featureNum;i++){
							singleResult[i].push_back(tempResult[i-l*featureNum]);
						}
					}
				}
				totalResult.push_back(singleResult);
			}
			//Generate the correspnding nametags for FE output
			GenParaFeatureNameSet_seg();
			//output
			for(int k = 0;k < fileNum;k++){

				for(int i = 0;i < featureNum*segNum;i++){
					for(int j = 0;j < attrNum;j++){
						fprintf(fout,"%s",ParaFeatureNameSet[k][i][j].c_str());
						if(j != (attrNum-1))
						fprintf(fout,",");

					}
					fprintf(fout,"\n");
					for(int j = 0;j < attrNum;j++){
						fprintf(fout,"%lf",totalResult[k][i][j]);
						if(j != (attrNum-1))
						fprintf(fout,",");

					}
					fprintf(fout,"\n");
				}
			}

			fclose(fout);
			break;
		
	}

}

double *FeatureExtraction(double* cleanData){
	//check if it is empty not implement yet

	
	int length = dataSize;//not implement yet

	static double f[featureNum];
	double tempData[dataSize];
	
	
	//tempData = cleanData^2;
	for(int i = 0;i < length;i++)
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
	for(int i = 0;i < featureNum;i++){//set 0 if nan
		if(isnan(f[i]))
			f[i] = 0;
	}
	return f;
	
	
}

void GenParaFeatureNameSet(){
	ParaFeatureNameSet.resize(fileNum);
	for(int k = 0;k < fileNum;k++)
		ParaFeatureNameSet[k].resize(featureNum);
	for(int k = 0;k < fileNum;k++)
		for(int i = 0;i < featureNum;i++)
			ParaFeatureNameSet[k][i].resize(attrNum);
	string tempStr;
	char buffer[50];
	for(int k = 0;k < fileNum;k++){
		for(int i = 0;i < featureNum;i++){
			for(int j = 0;j < attrNum;j++){
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


double *FeatureExtraction_seg(int chunkSize,double* cleanData){
//check if it is empty not implement yet

	
	int length = chunkSize;//not implement yet

	static double f[featureNum];
	double tempData[chunkSize];
	
	
	//tempData = cleanData^2;
	for(int i = 0;i < length;i++)
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
	for(int i = 0;i < featureNum;i++){//set 0 if nan
		if(isnan(f[i]))
			f[i] = 0;
		//cout<<f[i]<<endl;
	}

	return f;
	
}

void GenParaFeatureNameSet_seg(){
	ParaFeatureNameSet.resize(fileNum);
	for(int k = 0;k < fileNum;k++)
		ParaFeatureNameSet[k].resize(featureNum*segNum);
	for(int k = 0;k < fileNum;k++)
		for(int i = 0;i < featureNum*segNum;i++)
			ParaFeatureNameSet[k][i].resize(attrNum);
	string tempStr;
	char buffer[50];
	for(int k = 0;k < fileNum;k++){
		for(int m = 0;m < segNum;m++ ){

			for(int i = 0;i < featureNum;i++){

				for(int j = 0;j < attrNum;j++){
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
