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
void runFeatureExtraction(DataBase db,char* cycleBegin,char* cycleEnd);
double *FeatureExtraction(unsigned chunkSize,double* cleanData);



//Feature_name list
const char featureName[][30] ={"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};

//type for segmentation 1:no segmentation  2:segmentation
typedef enum {disable, enable}segmentPara;
segmentPara seg = disable;


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
	if(atoi(argv[3])!='\0')
	cycleBegin = atoi(argv[3]);
	else{
	cerr << "Cyclr brgin number error!";
	system("pause");
	return 1;
	}
	if(atoi(argv[4])!='\0')
	cycleEnd = atoi(argv[4]);
	else{
	cerr << "Cycle end number error!";
	system("pause");
	return 1;
	}
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
        cerr << "Database initializing failed." << endl;
        system("pause");
        return 1;
    }
    cout<<endl;
    runFeatureExtraction(db,argv[3],argv[4]);

    /*for(int id=db.beginOfCycle(cycleBegin); id<db.endOfCycle(cycleEnd); id++){
        FileData fd;
        if(db.extractById(id,fd)){
        	//start FE
			fileNum = fd.size();
			attrNum = fd.attrSize();
			//run FE
			runFeatureExtraction();
            cout << "read file id: " << fd.id << " lines: " << fd.dataVector.size() << endl;
        }
    }*/
	return 0;
}

void runFeatureExtraction(DataBase db,char* cycleBegin,char* cycleEnd){
	
	//Initialization
	vector<vector<double> > singleResult;//Result for a single file
	double* tempResult;//Result for a single attribute(variable)
	vector<double>temp;//All data in a single file for an attribute
	double* temp_array;//Turn temp into array version for computing(actually only pass memory address)
	FileData tempfd;
	db.extractById(db.beginOfCycle(atoi(cycleBegin)),tempfd);
   	attrNum = tempfd.attrSize();


	//enable or disable
	switch (seg){
		case disable:{//no segmentation
			
			//Initialization

			string tempFile;
			tempFile += "Output_noSeg_";
			tempFile += cycleBegin;
			tempFile +="-";
			tempFile += cycleEnd;
			tempFile += ".csv";

			FILE* fout = fopen(tempFile.c_str(),"w+");

			fprintf(fout,"%s,%s,%s,","Id","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout,"%s_%s,",tempfd.attrTypeVector[j+1].c_str(),featureName[i]);
				}
			}

			
			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first file
			while first call FE.
			Result:File_1_Attr_1_firstfeature - File_1_Attr_1_lasttfeature*/
    		for(int id=db.beginOfCycle(atoi(cycleBegin));
				id<db.endOfCycle(atoi(cycleEnd)); id++){
				//Initialization for a new file
				FileData fd;
        		if(db.extractById(id,fd)){
        			//start FE
					rowData = fd.dataVector;
					dataSize = fd.dataVector.size();
					temp.clear();
					temp.resize(dataSize);
					singleResult.clear();
					singleResult.resize(featureNum);
					for(unsigned j = 0;j < attrNum;j++){
						for(unsigned i = 0;i < dataSize;i++)
							temp[i] = rowData[i][j];
	 					temp_array = &temp[0];
						tempResult = FeatureExtraction(dataSize,temp_array);
						for(unsigned i = 0;i < featureNum;i++){
							singleResult[i].push_back(tempResult[i]);
						}
					}
					//output

					fprintf(fout,"\n");
					

					fprintf(fout,"%d,%d,%d,",fd.id,fd.fid,fd.nCycle);
					for(unsigned j = 0;j < attrNum;j++){
						for(unsigned i = 0;i < featureNum;i++){
							fprintf(fout,"%lf,",singleResult[i][j]);
						}
					}


					if( (id)!=0 && ((id)%100)==0 )
						cout<<"Computing "<<id<<" files."<<endl;
				}
			}
			cout<<"Computing done."<<endl;
			cout<<"Output processing..."<<endl;
			cout<<"Output done."<<endl;
			fclose(fout);
		}
			break;
			
		case enable:{//segmentation enabled

			//Initialization
			vector<double > originalDP;
			unsigned tempSize = 0;
			string f1,f2;
			char itoatemp[50] ;
			f1 = "Output_seg";
			itoa(segNum,itoatemp,10);
			f1 += itoatemp;
			f1 += "_";
			f2 = f1;
			f2 += cycleBegin;
			f2 +="-";
			f2 += cycleEnd;
			f1 += cycleBegin;
			f1 +="-";
			f1 += cycleEnd;
			f2 += "_2.csv";
			f1 += "_1.csv";
			FILE* fout1 = fopen(f1.c_str(),"w+");
			FILE* fout2 = fopen(f2.c_str(),"w+");
			
			//Pre-Output
			fprintf(fout1,"%s,%s,%s,","Id","Original_ID","Cycle");
			for(int i = 0;i < featureNum;i++)
				fprintf(fout1,"%s_%s,",tempfd.attrTypeVector[1].c_str(),featureName[i]);
			for(unsigned j = 1;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
                    for(unsigned k = 0;k < segNum;k++){
						fprintf(fout1,"%s%s%d_%s,",tempfd.attrTypeVector[j+1].c_str(),"_Seg_",k,featureName[i]);
					}
				}
			}
			fprintf(fout1,"\n");

			fprintf(fout2,"%s,%s,%s,","Id","Original_ID","Cycle");
			for(unsigned j = 0;j < attrNum;j++){
				for(unsigned i = 0;i < featureNum;i++){
					fprintf(fout2,"%s_%s,",tempfd.attrTypeVector[j+1].c_str(),featureName[i]);
				}
			}
			fprintf(fout2,"\n");
			

			/*Call FE by file,attribute and data size
			EX: calculate 12 features of first attribute of first segment
			of file while first call FE.
			Result:File_1_Seg_1_Attr_1_firstfeature - File_1_Seg_1_Attr_1_lasttfeature*/

    		for(int id=db.beginOfCycle(atoi(cycleBegin));
			id<db.endOfCycle(atoi(cycleEnd)); id++){
				FileData fd;
        		if(db.extractById(id,fd)){
					//Initialization for a new file
					rowData = fd.dataVector;
					dataSize = fd.dataVector.size();
					singleResult.clear();
					singleResult.resize(featureNum*segNum);

					temp.clear();
					temp.resize(dataSize);
				
					//Computing non-segmented DP_filter
					originalDP.clear();
					originalDP.resize(featureNum);


					for(unsigned i = 0;i < dataSize;i++)
						temp[i] = rowData[i][0];
	 				temp_array = &temp[0];
					tempResult = FeatureExtraction(dataSize,temp_array);
					for(unsigned i = 0;i < featureNum;i++){
						originalDP[i] = tempResult[i];
					}


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
							tempResult = FeatureExtraction(tempSize,temp_array);

							for(unsigned i = 0+l*featureNum;i < 0+l*featureNum+featureNum;i++){
								singleResult[i].push_back(tempResult[i-l*featureNum]);
							}
						}
					}

					fprintf(fout1,"%d,%d,%d,",fd.id,
					fd.fid,fd.nCycle);

					for(unsigned j = 0;j < attrNum;j++){
						for(unsigned i = 0;i < featureNum;i++){
							if(j == 0){
								fprintf(fout1,"%lf,",originalDP[i]);
								continue;
							}
							for(unsigned l = 0;l < segNum;l++)
								fprintf(fout1,"%lf,",singleResult[l*featureNum+i][j]);
						}
					}
					fprintf(fout1,"\n");
				
				
					for(unsigned l = 0;l < segNum;l++){
						fprintf(fout2,"%d,%d,%d,",fd.id,
						fd.fid,fd.nCycle);
						for(unsigned j = 0;j < attrNum;j++){
							for(unsigned i = 0;i < featureNum;i++){
								fprintf(fout2,"%lf,",singleResult[l*featureNum+i][j]);
							}
						}
						fprintf(fout2,"\n");
					}






					if( (id)!=0 && (id%100)==0 )
						cout<<"Computing "<<id<<" files."<<endl;
				}
			}
			

			
			cout<<"Computing done."<<endl;
			cout<<"Output processing..."<<endl;

			fclose(fout1);
			fclose(fout2);
			cout<<"Output done."<<endl;

		}
			break;
	}
}

double *FeatureExtraction(unsigned chunkSize,double* cleanData){
	
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

