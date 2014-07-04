#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define LINE_BUFFER_SIZE 1024
#define MAX_FILE_LIST_SIZE 512
#define MAX_CYCLE_INFO_ARRAY_LENGTH 256

const char cycleListFileName[] = "use_file_list.csv";
const char dataDirName[] = "dp_variable_selection";

struct CycleInfo {
    double cycle;
    int *fileNoList;
    char *fileNameList;
    int length;
};

void parseList(const char *listFileName, struct CycleInfo **cycleInfoArray, int *cycleInfoArrayLength)
{
    // open file
    FILE *fPtr = fopen(listFileName, "r");
    if(!fPtr) {
        printf("Cannot open file: %s\n",listFileName);
        exit(1);
    }

    // initialize
    char line[LINE_BUFFER_SIZE];
    char *pch;
    *cycleInfoArrayLength = 0;
    *cycleInfoArray = (struct CycleInfo *)calloc(MAX_CYCLE_INFO_ARRAY_LENGTH, sizeof(struct CycleInfo));

    // discard first line
    fgets(line, LINE_BUFFER_SIZE, fPtr);

    // read file by line
    while(fgets(line, LINE_BUFFER_SIZE, fPtr)!=NULL) {

        double cycle;
        int *fileNoList = calloc(MAX_FILE_LIST_SIZE, sizeof(int));
        (*cycleInfoArray)[*cycleInfoArrayLength].fileNoList = fileNoList;

        // split the line by comma
        pch = strtok (line,",");
        cycle = atof(pch);
        pch = strtok (NULL, ",");
        int colNum=0;
        while(pch != NULL) {
            fileNoList[colNum] = atof(pch);
            pch = strtok (NULL, ",");
            colNum++;
        }
        (*cycleInfoArray)[*cycleInfoArrayLength].length = colNum-1;
        int i;
        printf("cycle %.1f  total %d data :\n", cycle, colNum);
        for(i=0; i<colNum-1; i++) {
            printf("%d \t", (*cycleInfoArray)[*cycleInfoArrayLength].fileNoList[i]);
        }
        printf("\n");

        (*cycleInfoArrayLength)++;
    }

}

void singleFileExtract(const char *fileName)
{
    // open file
    FILE *fPtr = fopen(fileName, "r");
    if(!fPtr) {
        printf("Cannot open file: %s\n",fileName);
        exit(1);
    }
    printf("%s:\n",fileName);

    char line[LINE_BUFFER_SIZE];
    char *pch;
    int counter = 0;
    int colNum = 0;
    int attributeNum;

    // read first line, split the line by comma
    fgets(line, LINE_BUFFER_SIZE, fPtr);
    pch = strtok (line,",");
    while (pch != NULL) {
        colNum++;
        //printf ("%s\n",pch);
        pch = strtok (NULL, ",");
    }
    attributeNum = colNum-1;  // exclude first column (time stamp)
    //printf("attributeNum = %d\n\n", attributeNum);

    // read file by line
    while(fgets(line, LINE_BUFFER_SIZE, fPtr)!=NULL) {
        counter++;
        //printf("%d: ",counter);
        char timeStamp[128];
        double *attributeArray = calloc(attributeNum, sizeof(double));
        if(attributeArray==NULL){
            printf("\nMemory run out !!!\n");
            exit(100);
        }

        // split the line by comma
        pch = strtok (line,",");
        int i;
        for(i=-1; i<attributeNum; i++) {
            if(i==-1) {
                strcpy(timeStamp,pch);
                //printf("%s \t", timeStamp);
            } else {
                attributeArray[i] = atof(pch);
                //printf("%lf \t", attributeArray[i]);
            }
            pch = strtok (NULL, ",");
        }
        //printf("\n");
    }
    printf("\nTotal: %d data read\n", counter);
    fclose(fPtr);
}

int getNextFileNo(struct CycleInfo *cycleInfoArray, int arrayLength)
{
    static int listNo = 0;
    static int fileNo = 0;
    int nextFileNo = -1;
    if(listNo < arrayLength) {
        if(fileNo < cycleInfoArray[listNo].length) {
            nextFileNo = cycleInfoArray[listNo].fileNoList[fileNo];
            fileNo++;
        } else {  // change to next list
            listNo++;
            fileNo=0;
            if(listNo < arrayLength) {
                nextFileNo = cycleInfoArray[listNo].fileNoList[fileNo];
                fileNo=1;
            }
        }
    }
    return nextFileNo;
}

void cycleExtract()
{
    struct CycleInfo *cycleInfoArray=0;
    int cycleInfoArrayLength;
    char fileNameList[2048][256];
    int fileCounter=0;

    // choose file on list
    parseList(cycleListFileName, &cycleInfoArray, &cycleInfoArrayLength);
    printf("cycleInfoArrayLength = %d\n", cycleInfoArrayLength);

    DIR *dir;
    struct dirent *ent;
    int nextFileNo = getNextFileNo(cycleInfoArray, cycleInfoArrayLength);
    int fileNameIndex = -1;
    char fileNameIndexStr[5];

    // search data directory
    if ((dir = opendir (dataDirName)) != NULL) {
        while ((ent = readdir (dir)) != NULL){
            if(strstr(ent->d_name, "run")==NULL) continue;  // exclude other file
            strncpy(fileNameIndexStr, ent->d_name+3,4); // get file serial number
            fileNameIndexStr[4]=0;
            fileNameIndex = atoi(fileNameIndexStr);
            if(fileNameIndex==nextFileNo){  // file id matched
                nextFileNo = getNextFileNo(cycleInfoArray, cycleInfoArrayLength);
                char relativePath[256] = ".\\dp_variable_selection\\selection\\";
                strcpy(fileNameList[fileCounter++], strcat(relativePath, ent->d_name));
                printf("%d \t", fileNameIndex);
                char cmd[512] = "copy /y .\\dp_variable_selection\\\"";  // copy to selection directory
                strcat(cmd, ent->d_name);
                strcat(cmd, "\" .\\dp_variable_selection\\selection");
                //system(cmd);

            }
        }
        closedir (dir);
    } else {
        // could not open directory
        perror ("");
        return;
    }

    // start file extraction
    int i;
    for(i=0; i<fileCounter; i++){
        singleFileExtract(fileNameList[i]);
    }
    printf("Finished !\n");

}

int main()
{
    cycleExtract();
    return 0;
}
