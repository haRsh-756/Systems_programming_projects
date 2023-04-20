#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
/**
 * author: harsh_patel
 * program_name: find.c
*/
int findStatus(char * currDir,char * targetFile){
    DIR * inDir = opendir(currDir);
    if(inDir == NULL){
        printf("Something went wrong\n");
        exit(EXIT_FAILURE);
    }
    struct dirent* dir;
    int status = 0;
    while((dir = readdir(inDir))!= NULL){
        if((strcmp(dir->d_name,".")==0) || (strcmp(dir->d_name,"..")==0)){
            continue;
        }
        else{
            if(strcmp(dir->d_name,targetFile)==0){
                status = 1;
            }
        }
    }
    closedir(inDir);
    return status;
}
void SearchFiles(char * currDir, char * targetFileName){
    //printf("check:%s\n",currDir);
    DIR * inDir = opendir(currDir);
    if(inDir == NULL){
        printf("Error in opening directory\n");
        exit(EXIT_FAILURE);
    }
    struct dirent *directory;
    while( (directory = readdir(inDir)) != NULL){
        if((strcmp(directory->d_name,".")==0) || (strcmp(directory->d_name,"..")==0)){
            continue;
        }
        /*if((strstr(directory->d_name,targetFileName)!= NULL)){
            printf("./%s\n",directory->d_name);
        }*/
        if((strcmp(directory->d_name,targetFileName)==0)){
            
            printf("%s/%s\n",currDir, directory->d_name);
            closedir(inDir);
            return;
        }
        else if(strstr(directory->d_name,targetFileName)!= NULL){
            printf("./%s\n",directory->d_name);
        }
        if(directory->d_type == 4){
            //printf("%s\n",directory->d_name);
            char subDir[300] ={};
            strcat(subDir,currDir);
            strcat(subDir,"/");
            strcat(subDir,directory->d_name);
            SearchFiles(subDir,targetFileName); 
        }
    }
    closedir(inDir);
}
int main(int argc, char *argv[])
{
    //if file found in the present directory print that else branch
    if(argc > 2){
        printf("Too many Arguments\n");
        exit(EXIT_FAILURE);
    }
    if(argc == 1){
        printf("More arguments required\n");
        exit(EXIT_FAILURE);
    }
    int result = findStatus(".",argv[1]);
    if(result != 1){
        SearchFiles(".",argv[1]);
    }
    else{
        printf("./%s\n",argv[1]);
    }
    return EXIT_SUCCESS;
}
