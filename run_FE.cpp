#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/* 1:no segmentation  2-3:segmentation, �@�k���P,
case 2�����FeatureExtraction��k, case 3�A�Ω�RUN�Ƥ֡A���W�[feature�Ʈɨϥ�
*/
enum segmentPara{disable,enable,enableless};
const char* fileName="cycle";
int main(){
	segmentPara seg=disable;

	//read from result of dataEtract
	for(int count=1;count<=cycleNum;count++){	
		FILE *fPtr=fopen(strcat(fileName+itoa(count)+".txt","r");
		
		if(!fPtr){
    	    printf("Cannot open file: %s\n",fileName);
        	exit(1);
    	}
	}
	switch (seg){
		case disable:
			
		case enable:
		
		case enableless:
			
		
		
		
	}
	
	
}
