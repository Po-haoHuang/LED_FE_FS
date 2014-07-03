#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include <gsl/gsl_sort.h>
#include<gsl/gsl_statistics_double.h>


// 1:no segmentation  2:segmentation
enum segmentPara {disable, enable, enableless};

const char* fileName = "cleandata";

int main(){

	segmentPara seg = disable;

	//read from result of dataEtract
	for(int count = 1;count<=cycleNum;count++){	
		FILE *fPtr = fopen(strcat(fileName+itoa(count)+".txt","r");
		/*not implement yet
		depend on file input
		read in cleanData
		*/
		if(!fPtr){
    	    printf("Cannot open file: %s\n",fileName);
        	exit(1);
    	}
	}
	switch (seg){
		case disable://no segmentation
			
			FeatureExtraction();
			GenParaFeatureNameSet();			
			break;
			
		case enable://segmentation
			
			DataSegmentation();
			FeatureExtraction_m();
			GenParaFeatureNameSet_m();
			break;
		
	}
	
	
}

FeatureExtraction(){
	//check if it is empty not implement yet

	
	int length = datasize;//not implement yet
	int i;
	int f[12];
	double *tempData;
	
	//tempData = cleanData^2;
	tempData = calloc(length, sizeof(double));
	for(i = 0; i++; i<length)
		tempData[i] = pow(cleanData[i], 2);
	f[0] = 
	f[1] = 
	
	
	//three para data_array,element size(#double),#elements	
	f[2] = gsl_stats_mean (cleanData, 1, length);
	f[3] = gsl_stats_variance (cleanData, 1, length);
	f[4] = gsl_stats_skew (cleanData, 1, length);
	f[5] = gsl_stats_kurtosis (cleanData, 1, length);
	f[6] = sqrt (gsl_stats_mean (tempData, 1, length);)
	f[7] = gsl_stats_max (cleanData, 1, length);
	f[8] = gsl_stats_min (cleanData, 1, length);
	f[9] = f7 - f8;
	gsl_sort (cleandData, 1, length);//sort before iqr
	f[10] = gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.75)
			- gsl_stats_quantile_from_sorted_data (cleanData,1, length, 0.25);
	f[11] = gsl_stats_sd (cleanData, 1, length);
	
	
}
