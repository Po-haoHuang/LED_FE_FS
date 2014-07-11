#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"gsl/gsl_sort.h"
#include"gsl/gsl_statistics_double.h"
#include<iostream>
#include<vector>
#include<string>
#include <fstream>
#include <dirent.h>


//include file processing part
#include "include/FileData.h"
#include "include/DataBase.h"
#include "include/CycleData.h"

//Variable for feature extraction
unsigned attrNum;
unsigned dataSize;
const unsigned featureNum = 12;
unsigned fileNum;
unsigned segNum;
vector<FileData> fileDataVector;
vector<vector<double> > rowData;

using namespace std;

//data DIR
const string cycleListFileName = "use_file_list.csv";
const string dataDir = ".\\dp_variable_selection\\";

//Function declaration
//For FE
double *FeatureExtraction(double* cleanData);
void runFeatureExtraction();
void GenParaFeatureNameSet();
double *FeatureExtraction_seg(unsigned chunkSize,double* cleanData);
void GenParaFeatureNameSet_seg();


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

    db.init(dataDir, cycleListFileName);   // use filtered files directory as working directory

    if(!db.valid()){
        cout << "Database initializing failed." << endl;
        return 1;
    }

    db.printCycleList();

    // Set cycle range
    double cycleBegin, cycleEnd;
    cout << endl << "Set cycle begin: ";
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
        cout << ", containing: " << endl;
        for(unsigned i = 0; i< fileDataVector.size(); i++){
            cout << fileDataVector[i].id << " \t";
        }

    }else{
        cout << "No file extracted." << endl;
    }
    cout<<endl;
    
	//start FE
	fileNum = fileDataVector.size();
	attrNum = fileDataVector[0].attrSize();

	
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
			fprintf(fout,"%s,%s,%s,","ID","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout,"%s%d_%s,","Attr_",j,featureName[i]);
				}
			}
			fprintf(fout,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				fprintf(fout,"%d,%d,%d,",fileDataVector[k].id,
				fileDataVector[k].fid,fileDataVector[k].nCycle);
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
			dataSize = fileDataVector[0].dataVector.size();

			for(unsigned k = 1;k < fileNum;k++)//Get minimum datasize of all selected files
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
				//Initialization for a new file
				rowData = fileDataVector[k].dataVector;
				dataSize = fileDataVector[k].dataVector.size();
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
				if( k!=0 && (k%20)==0 )
					cout<<"Computing "<<k<<" files."<<endl;
			}
			cout<<"Computing done."<<endl;
			//Generate the correspnding nametags for FE output
			GenParaFeatureNameSet_seg();
			cout<<"Generating nametags done."<<endl;

			cout<<"Output processing..."<<endl;
			
			//output
			FILE* fout1 = fopen("Output_seg1.csv","w+");
			FILE* fout2 = fopen("Output_seg2.csv","w+");
			fprintf(fout1,"%s,%s,%s,","ID","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
                    for(unsigned k = 0;k < segNum;k++){
						fprintf(fout1,"%s%d%s%d_%s,","Attr_",j,"_Seg_",k,featureName[i]);
					}
				}
			}
			fprintf(fout1,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				fprintf(fout1,"%d,%d,%d,",fileDataVector[k].id,
				fileDataVector[k].fid,fileDataVector[k].nCycle);
				for(unsigned j = 0;j < attrNum;j++){
					for(unsigned i = 0;i < featureNum;i++){
						for(unsigned l = 0;l < segNum;l++)
							fprintf(fout1,"%lf,",totalResult[k][l*featureNum+i][j]);
					}
				}
				fprintf(fout1,"\n");

			}
			fclose(fout1);
			fprintf(fout2,"%s,%s,%s,","ID","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout2,"%s%d_%s,","Attr_",j,featureName[i]);
				}
			}
			fprintf(fout2,"\n");
			for(unsigned k = 0;k < fileNum;k++){
				for(unsigned l = 0;l < segNum;l++){
					fprintf(fout2,"%d,%d,%d,",fileDataVector[k].id,
					fileDataVector[k].fid,fileDataVector[k].nCycle);
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
