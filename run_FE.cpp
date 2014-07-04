#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include <gsl/gsl_sort.h>
#include<gsl/gsl_statistics_double.h>
#include<iostream>
#include<vector>
#define attrNum 6
#define dataSize 6
#define featureNum 12
#define fileNum 2

using namespace std;

double *FeatureExtraction(double* cleanData);
void runFeatureExtraction();
void GenParaFeatureNameSet();

double rowData[6][6]={
{-5.701741013,100.016276,1199.951172,5000,5.502386154,0.009155553},
{-5.701741013,99.97558594,1199.951172,4998.779297,5.502386154,0.009155553},
{-5.671250419,100.7486979,1199.951172,4995.117188,5.502386154,0.006103702},
{-5.671250419,121.1344401,1197.387695,4732.666016,5.502386154,0.009155553},
{-5.671250419,97.57486979,748.4130859,4849.853516,5.502386154,0.009155553},
{-5.701741013,92.36653646,855.7128906,4805.908203,5.502386154,0.006103702},
};
const char featureName[][30] ={"peak(not implement)","peak(not implement)",
"mean","variance","skewness","kurtosis","RMS","max","min","range","iqr","std"};

// 1:no segmentation  2:segmentation
typedef enum {disable, enable, enableless}segmentPara;
segmentPara seg = disable;
vector<vector<vector<string> > > ParaFeatureNameSet;


int main(){

	runFeatureExtraction();





	
	return 0;
}

void runFeatureExtraction(){
	
		double* tempResult;
	tempResult = (double*)calloc(attrNum,sizeof(double));
	double temp[dataSize];

	switch (seg){
		case disable://no segmentation
			vector<vector<double>*> totalResult;
			vector<double> *singleResult = new vector<double> [featureNum];
			for(int k = 0;k < fileNum;k++){
			
				for(int j = 0;j < attrNum;j++){
							
					for(int i = 0;i < dataSize;i++)
						temp[i] = rowData[i][j];			
					tempResult = FeatureExtraction(temp);
			
					for(int i = 0;i < featureNum;i++){
						singleResult[i].push_back(tempResult[i]);
					}
				}
				totalResult.push_back(singleResult);
			}
			GenParaFeatureNameSet();
			for(int k = 0;k < fileNum;k++){
			
				for(int i = 0;i < featureNum;i++){
					for(int j = 0;j < attrNum;j++){
					
						printf("%lf\t",totalResult[k][i][j]);
						cout<<ParaFeatureNameSet[k][i][j];
						cout<<endl;
					}
					cout<<endl;
				}
			}

			
			break;
			
		/*case enable://segmentation
			
			//DataSegmentation();
			//FeatureExtraction_m();
			//GenParaFeatureNameSet_m();
			break;*/
		
	}
}

double *FeatureExtraction(double* cleanData){
	//check if it is empty not implement yet

	
	int length = dataSize;//not implement yet

	static double f[12];
	double tempData[6];
	
	
	//tempData = cleanData^2;
	for(int i = 0; i++; i<length)
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
	for(int i = 0;i < 12;i++){//set 0 if nan
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
	for(int k = 0;k < fileNum;k++){
		for(int i = 0;i < featureNum;i++){
			for(int j = 0;j < attrNum;j++){
				tempStr.clear();
				tempStr = "File_";
				tempStr += k+48;
				tempStr += "_Attr_";
				tempStr += j+48;
				tempStr += "_";
				tempStr += featureName[i];
				ParaFeatureNameSet[k][i][j] = tempStr;

			}
		}
	}
}

/*
FeatureExtraction_m(){
	//check if it is empty not implement yet

	
	int length = datasize;//not implement yet

	int f[12];
	double *tempData;
	

	tempData = calloc(length, sizeof(double));
	for(int i = 0; i++; i<length)
		tempData[i] = pow(cleanData[i], 2);

	
	
	//three para data_array,element size(#double),#elements	
	f[2] = gsl_stats_mean (cleanData, 1, length);
	f[3] = gsl_stats_variance (cleanData, 1, length);
	f[4] = gsl_stats_skew (cleanData, 1, length);
	f[5] = gsl_stats_kurtosis (cleanData, 1, length);
	f[6] = sqrt (gsl_stats_mean (tempData, 1, length));
	f[7] = gsl_stats_max (cleanData, 1, length);
	f[8] = gsl_stats_min (cleanData, 1, length);
	f[9] = f[7] - f[8];
	gsl_sort (cleanData, 1, length);//sort before iqr
	f[10] = gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
			- gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25);
	f[11] = gsl_stats_sd (cleanData, 1, length);
	for(int i = 0;i < 12;i++){//set 0 if nan
		if(isnan(f[i]))
			f[i] = 0;
	}
	
	
}*/
