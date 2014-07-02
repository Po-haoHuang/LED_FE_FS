#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 1024

const char fileName[] = "run0001_CE_ON__1-current.csv";
int main()
{
    // open file
    FILE *fPtr = fopen(fileName, "r");
    if(!fPtr){
        printf("Cannot open file: %s\n",fileName);
        exit(1);
    }

    char line[LINE_BUFFER_SIZE];
    char *pch;
    int counter = 0;
    int colNum = 0;
    int attributeNum;

    // read first line, split the line by comma
    fgets(line, LINE_BUFFER_SIZE, fPtr);
    pch = strtok (line,",");
    while (pch != NULL){
        colNum++;
        printf ("%s\n",pch);
        pch = strtok (NULL, ",");
    }
    attributeNum = colNum-1;  // exclude first column (time stamp)
    printf("attributeNum = %d\n\n", attributeNum);

    // read file by line
    while(fgets(line, LINE_BUFFER_SIZE, fPtr)!=NULL){
        counter++;
        printf("%d: ",counter);
        char timeStamp[128];
        double *attributeArray = calloc(attributeNum, sizeof(double));

        // split the line by comma
        pch = strtok (line,",");
        int i;
        for(i=-1; i<attributeNum; i++){
            if(i==-1){
                strcpy(timeStamp,pch);
                printf("%s \t", timeStamp);
            }
            else{
                attributeArray[i] = atof(pch);
                printf("%lf \t", attributeArray[i]);
            }
            pch = strtok (NULL, ",");
        }
        printf("\n");
    }
    printf("\nTotal: %d data read\n", counter);
    return 0;
}
