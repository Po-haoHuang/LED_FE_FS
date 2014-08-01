#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
//gsl library for statistic
#include"gsl/gsl_sort.h"
#include"gsl/gsl_statistics_double.h"
#include<iostream>
#include<vector>
#include<string>
#include <fstream>
#include <dirent.h>


//include file processing part
#include "../include/FileData.h"
#include "../include/DataBase.h"
#include "../include/CycleData.h"

//Variable for feature extraction
unsigned attrNum;
unsigned dataSize;
const unsigned featureNum = 10;
unsigned fileNum;
unsigned segNum;
vector<FileData*> fileDataVector;
vector<vector<double> > rowData;

using namespace std;

//Function declaration
//For FE
double *FeatureExtraction(double* cleanData);
void runFeatureExtraction();
void GenParaFeatureNameSet();
double *FeatureExtraction_seg(unsigned chunkSize,double* cleanData);
void GenParaFeatureNameSet_seg();


//Feature_name list
/*const char featureName[][30] ={"peak(not implement)","peak(not implement)",
"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};*/
const char featureName[][30] ={"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};

//type for segmentation 1:no segmentation  2:segmentation
typedef enum {disable, enable}segmentPara;
segmentPara seg = disable;

//ParaFeatureNameSet
vector<vector<vector<string> > > ParaFeatureNameSet;

int main(int argc, char *argv[]){
	string cycleListFileName,dataDir;
	int cycleBegin,cycleEnd;
	

	//checking argv input
	if(argc > 6){//too many arguments
		cerr<<"Too many arguments.\n";
		system("pause");
		return -1;
	}

	else if(argc < 6){//too few arguments
		cerr<<"Too few arguments.\n";
		system("pause");
		return -1;
	}
	else{
	//data DIR
	cycleListFileName = argv[1];
	dataDir = argv[2];
	//Set cycle range
	cycleBegin = atoi(argv[3]);
	cycleEnd = atoi(argv[4]);
	//set Segmentation number 
	segNum = atoi(argv[5]);
	if (segNum == 1)
		seg = disable;
	else 
		seg = enable;
	}

	//Start File IO	
 	// database
    DataBase db;

    db.init(dataDir, cycleListFileName);   // use filtered files directory as working directory

    if(!db.valid()){
        cout << "Database initializing failed." << endl;
        system("pause");
        return 1;
    }

    bool extractSuccess = db.extract(cycleBegin, cycleEnd); // start extracting file data

    if(!extractSuccess){
        cout << "Extracting failed." << endl;
        system("pause");
        return 1;
    }

    // get all file data

    bool getAllFileSuccessful = db.getAllFileDataPtr(fileDataVector);

    if(getAllFileSuccessful){
        cout << endl << "There are " << fileDataVector.size() << " files extracted:";

    }else{
        cout << "No file extracted." << endl;
        system("pause");
		return 1;
    }
    cout<<endl;
    
	//start FE
	fileNum = fileDataVector.size();
	attrNum = fileDataVector[0]->attrSize();
	
	//run FE
	runFeatureExtraction();
	return 0;
}

void runFeatureExtraction(){
	
	//Initialization
	vector<vector<vector<double> > > totalResult;//Result for all files selected
	vector<vector<double> > singleResult;//Result for a single file
	double* tempResult;//Result for a single attribute(variable)
	vector<double>temp;//All data in a single file for an attribute
	double* temp_array;//Turn temp into array version for computing(actually only pass memory address)


	//enable or disable
	switch (seg){
		case disable:{//no segmentation
			
			//Initialization
			totalResult.clear();
			singleResult.resize(featureNum);
			FILE* fout = fopen("Output_noSeg.csv","w+");

			
			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first file
			while first call FE.
			Result:File_1_Attr_1_firstfeature - File_1_Attr_1_lasttfeature*/
			for(unsigned k = 0;k < fileNum;k++){
				//Initialization for a new file
				rowData = fileDataVector[k]->dataVector;
				dataSize = fileDataVector[k]->dataVector.size();
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
				if( (k-1)!=0 && ((k-1)%100)==0 )
					cout<<"Computing "<<k<<" files."<<endl;
			}
			cout<<"Computing done."<<endl;

			cout<<"Output processing..."<<endl;
			//output
			fprintf(fout,"%s,%s,%s,","Id","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout,"%s_%s,",fileDataVector[0]->attrTypeVector[j+1].c_str(),featureName[i]);
				}
			}
			fprintf(fout,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				fprintf(fout,"%d,%d,%d,",fileDataVector[k]->id,
				fileDataVector[k]->fid,fileDataVector[k]->nCycle);
				for(unsigned j = 0;j < attrNum;j++){
					for(unsigned i = 0;i < featureNum;i++){
						fprintf(fout,"%lf,",totalResult[k][i][j]);
					}
				}
				fprintf(fout,"\n");
			}
			cout<<"Output done."<<endl;
			fclose(fout);
		}
			break;
			
		case enable:{//segmentation enabled

			//Initialization
			totalResult.clear();
			unsigned tempSize = 0;


			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first segment
			of file while first call FE.
			Result:File_1_Seg_1_Attr_1_firstfeature - File_1_Seg_1_Attr_1_lasttfeature*/

			for(unsigned k = 0;k < fileNum;k++){
				//Initialization for a new file
				rowData = fileDataVector[k]->dataVector;
				dataSize = fileDataVector[k]->dataVector.size();
				singleResult.clear();
				singleResult.resize(featureNum*segNum);


				for (unsigned l = 0;l < segNum;l++){
					for(unsigned j = 0;j < attrNum;j++){

						tempSize = 0;
						temp.clear();
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
								tempSize++;
								temp.push_back(rowData[i][j]);
							}
						}

						temp_array = &temp[0];
						tempResult = FeatureExtraction_seg(tempSize,temp_array);

						for(unsigned i = 0+l*featureNum;i < 0+l*featureNum+featureNum;i++){
							singleResult[i].push_back(tempResult[i-l*featureNum]);
						}
					}
				}
				totalResult.push_back(singleResult);
				if( (k-1)!=0 && ((k-1)%100)==0 )
					cout<<"Computing "<<k<<" files."<<endl;
			}
			
			//Computing non-segmented DP_filter
			singleResult.clear();
			singleResult.resize(featureNum);
			
			for(unsigned k = 0;k < fileNum;k++){
				//Initialization for a new file
				rowData = fileDataVector[k]->dataVector;
				dataSize = fileDataVector[k]->dataVector.size();
				temp.clear();
				temp.resize(dataSize);

				
				for(unsigned i = 0;i < dataSize;i++)
					temp[i] = rowData[i][0];
	 			temp_array = &temp[0];
				tempResult = FeatureExtraction(temp_array);
				for(unsigned i = 0;i < featureNum;i++){
					singleResult[i].push_back(tempResult[i]);
				}
			}

			
			cout<<"Computing done."<<endl;

			cout<<"Output processing..."<<endl;
			
			//output
			string f1,f2;
			char itoatemp[50] ;
			f1 = "Output_seg";
			itoa(segNum,itoatemp,10);
			f1 += itoatemp;
			f2 = f1;
			f2 += "_2.csv";
			f1 += "_1.csv";
			FILE* fout1 = fopen(f1.c_str(),"w+");
			FILE* fout2 = fopen(f2.c_str(),"w+");
			fprintf(fout1,"%s,%s,%s,","Id","Original_ID","Cycle");
			for(int i = 0;i < featureNum;i++)
				fprintf(fout1,"%s%s,",fileDataVector[0]->attrTypeVector[1].c_str(),featureName[i]);
			for(unsigned j = 1;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
                    for(unsigned k = 0;k < segNum;k++){
						fprintf(fout1,"%s%s%d_%s,",fileDataVector[0]->attrTypeVector[j+1].c_str(),"_Seg_",k,featureName[i]);
					}
				}
			}
			fprintf(fout1,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				fprintf(fout1,"%d,%d,%d,",fileDataVector[k]->id,
				fileDataVector[k]->fid,fileDataVector[k]->nCycle);
				
				for(unsigned j = 0;j < attrNum;j++){
					for(unsigned i = 0;i < featureNum;i++){
						if(j == 0){
							fprintf(fout1,"%lf,",singleResult[i][k]);
							continue;
						}
						for(unsigned l = 0;l < segNum;l++)
							fprintf(fout1,"%lf,",totalResult[k][l*featureNum+i][j]);
					}
				}
				fprintf(fout1,"\n");

			}
			fclose(fout1);
			fprintf(fout2,"%s,%s,%s,","Id","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout2,"%s_%s,",fileDataVector[0]->attrTypeVector[j+1].c_str(),featureName[i]);
				}
			}
			fprintf(fout2,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				for(unsigned l = 0;l < segNum;l++){
					fprintf(fout2,"%d,%d,%d,",fileDataVector[k]->id,
					fileDataVector[k]->fid,fileDataVector[k]->nCycle);
					for(unsigned j = 0;j < attrNum;j++){
						for(unsigned i = 0;i < featureNum;i++){
							fprintf(fout2,"%lf,",totalResult[k][l*featureNum+i][j]);
						}
					}
					fprintf(fout2,"\n");
				}
			}
			fclose(fout2);
			cout<<"Output done."<<endl;
		}
			break;
		
	}

}

double *FeatureExtraction(double* cleanData){
	
	unsigned length = dataSize;

	static double f[featureNum];
	double tempData[dataSize];
	
	
	//tempData = cleanData^2;
	for(unsigned i = 0;i < length;i++)
		tempData[i] = pow(cleanData[i], 2);

	//three para for gsl func (data_array,element size(#double),#elements)	
	f[0] = gsl_stats_mean (cleanData, 1, length);
	f[1] = gsl_stats_variance (cleanData, 1, length);
	f[2] = gsl_stats_skew (cleanData, 1, length);
	f[3] = gsl_stats_kurtosis (cleanData, 1, length);
	f[4] = sqrt (gsl_stats_mean (tempData, 1, length));//rms
	f[5] = gsl_stats_max (cleanData, 1, length);
	f[6] = gsl_stats_min (cleanData, 1, length);
	f[7] = f[5] - f[6];//range
	gsl_sort (cleanData, 1, length);//sort before iqr
	//iqr
	f[8] = gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
			- gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25);
	f[9] = gsl_stats_sd (cleanData, 1, length);//std
	for(unsigned i = 0;i < featureNum;i++){//set 0 if nan
		if(isnan(f[i]))
			f[i] = 0;
	}
	return f;
	
	
}


double *FeatureExtraction_seg(unsigned chunkSize,double* cleanData){
	
	unsigned length = chunkSize;

	static double f[featureNum];
	double tempData[chunkSize];
	
	
	//tempData = cleanData^2;
	for(unsigned i = 0;i < length;i++)
		tempData[i] = pow(cleanData[i], 2);

	//three para for gsl func (data_array,element size(#double),#elements)	
	f[0] = gsl_stats_mean (cleanData, 1, length);
	f[1] = gsl_stats_variance (cleanData, 1, length);
	f[2] = gsl_stats_skew (cleanData, 1, length);
	f[3] = gsl_stats_kurtosis (cleanData, 1, length);
	f[4] = sqrt (gsl_stats_mean (tempData, 1, length));//rms
	f[5] = gsl_stats_max (cleanData, 1, length);
	f[6] = gsl_stats_min (cleanData, 1, length);
	f[7] = f[5] - f[6];//range
	gsl_sort (cleanData, 1, length);//sort before iqr
	//iqr
	f[8] = gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
			- gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25);
	f[9] = gsl_stats_sd (cleanData, 1, length);//std
	
	for(unsigned i = 0;i < featureNum;i++){//set 0 if nan
		if(isnan(f[i]))
			f[i] = 0;
	}

	return f;
	
}

