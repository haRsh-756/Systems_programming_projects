#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
/**
 * author: harsh_patel
 * program_name: tree.c
*/
//int level = 0;
int flag=0;
char* process(char* input){
    char* tmp = input;
    for(int i=0; i<strlen(tmp); i++){
        tmp[i] = tolower(tmp[i]);
    }
    return tmp;
}
char** sortArr(char** ArrToSort, int ArrSize){
    char** tempArr = ArrToSort;
    char* temp = NULL;
    char* t1 = NULL;
    char* t2 = NULL;
    for(int i=0; i< ArrSize; i++){                    //sorting string or integers
        for(int j= i+1; j<ArrSize; j++){
            t1 = malloc(strlen(tempArr[i])+1);
            t2 = malloc(strlen(tempArr[j])+1);
            strcpy(t1,tempArr[i]);
            strcpy(t2,tempArr[j]);
            if(strcmp(t1 = process(t1),t2 = process(t2)) > 0){
                temp = tempArr[i];
                tempArr[i] = tempArr[j];
                tempArr[j] = temp;
            }
            free(t1);
            free(t2);
        } 
    }
    return tempArr;
}
int check_regular_file(const char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void sortprint(char* basePath, int level){
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if(dir == NULL){
        printf("End of file\n");
        closedir(dir);
        //exit(EXIT_SUCCESS);
        return;
    }
    char ** Arr= NULL;
    int count = 0;
    //printf("Reading files:%s\n",basePath);
    while((dp=readdir(dir))!= NULL){
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0){
            char ** temp = realloc(Arr,(count+1)*sizeof(*temp));
                if(temp == NULL){
                    printf("Reallocation failed");
                    free(Arr);
                    exit(EXIT_FAILURE);
                }
            
                Arr = temp;
                Arr[count++] = malloc(strlen(dp->d_name)+1);
                strcpy(Arr[count-1],dp->d_name);
        }
    }
    if(count > 1){
        Arr = sortArr(Arr,count);
    }
    
    for(int i=0; i< count; i++){
        for (int j=0; j<level; j++) 
            {
                if (j%2 == 0 || j == 0){
                    continue;
                }
                else
                    printf("  ");
            }
            //if((root == 0||root%2==0) && ((is_regular_file(Arr[i]))==1)){
            //printf("file coming,%s\n",Arr[i]);
             char fullpath[300] ={};
            strcat(fullpath,basePath);
            strcat(fullpath,"/");
            strcat(fullpath,Arr[i]);
            if((level == 0 || level%2 ==0)&&(check_regular_file(fullpath))==1){
                printf("- %s\n",Arr[i]);
            }
            else if((check_regular_file(fullpath))==0){
                printf("- %s\n",Arr[i]);
            }
             if((check_regular_file(fullpath))==0){
                    //printf("recurse:\n");
                    char subDir[300] ={};
                    strcat(subDir,basePath);
                    strcat(subDir,"/");
                    strcat(subDir,Arr[i]);
                    sortprint(subDir,level+2);
             }          
    }
    for(int i=0; i<count; i++){
        free(Arr[i]);
    }
    free(Arr);
    closedir(dir);
}

void tree(char *basePath, const int root)
{
    int i;
    struct dirent *dp;
    //printf("Reading in %s\n",basePath);
    DIR *dir = opendir(basePath);
    if(dir == NULL){
        printf("error\n");
        return;
    }
    while ((dp = readdir(dir)) != NULL){
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            for (i=0; i<root; i++) 
            {
                if (i%2 == 0 || i == 0){
                    //printf("%c", '');
                    //printf("  ");
                    continue;
                }
                else
                    printf("  ");
            }
            if((root == 0||root%2==0) && dp->d_type == 8){
                printf("  d%s\n",dp->d_name);
                
                if(dp->d_type == 8){

                    //printf("check;%d\n",dp->d_reclen);
                }
            }
            else{
                printf("  x%s\n",dp->d_name);
            }
            if(dp->d_type == 4){
            char path[300];
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            tree(path, root + 2);
            }
        }
        //printf("endhere\n");
    }
    closedir(dir);
}
int main(int argc, char *argv[]){
    if(argc>1){
        printf("Too many Arguments\n");
        exit(EXIT_FAILURE);
    }
    printf(".\n");
    sortprint(".",0);
    //tree(".",0);
    return EXIT_SUCCESS;
}
