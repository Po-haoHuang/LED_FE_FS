#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER 1024

const char fileName[] = "run0001_CE_ON__1-current.csv";
int main()
{
    // open file
    FILE *fPtr = fopen(fileName, "r");
    if(!fPtr){
        printf("Cannot open file: %s\n",fileName);
        exit(1);
    }

    char line[LINE_BUFFER];
    char *pch;
    int counter = 0;
    int colNum = 0;
    // read one line
    while(fgets(line, LINE_BUFFER, fPtr)!=NULL){

        printf("%d: %s", ++counter, line);

        // split the line by comma
        pch = strtok (line,",");
        while (pch != NULL){
            if(counter==1) colNum++;
            printf ("%s\n",pch);
            pch = strtok (NULL, ",");
        }

        // store data, except for time stamp
        if(counter>1){
            double attributeArray*;

        }
    }
    printf("colNum = %d\n", colNum);
    return 0;
}
