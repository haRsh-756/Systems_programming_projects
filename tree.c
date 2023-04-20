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
                    //printf("%c", '');
                    //printf("  ");
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
                //printf("%d\n",level);
                //level = level -2;
                //root=0;
                //printf("d%s\n",Arr[i]);
                //if(dp->d_type == 8){

                    //printf("check;%d\n",dp->d_reclen);
                //}
            }
            //printf("%s%c%s\n", "|>", '-', dp->d_name);
            else if((check_regular_file(fullpath))==0){
            //else{
                 //printf("%d\n",level);
                 //if (level ==0) 
                 //printf("  ");

                printf("- %s\n",Arr[i]);
                //level += 2;
            }
             if((check_regular_file(fullpath))==0){
                    //printf("recurse:\n");
                    char subDir[300] ={};
                    strcat(subDir,basePath);
                    strcat(subDir,"/");
                    strcat(subDir,Arr[i]);
                    //level +=2;
                    //level + 2;
                    sortprint(subDir,level+2);
                    
                    //printf("  ");
                    //level = level -2;
                    //level =4;
                }
               // if()
                
             //   root=0;
            
            //root= 0;
            //root +=2;
        //printf("file name:%s\n",Arr[i]);
        //root=0;
        //level=2;
    }
    //level=0;
    
   
   /* for(int i=0; i<count; i++){
         char fullpath[300] ={};
            strcat(fullpath,basePath);
            strcat(fullpath,"/");
            strcat(fullpath,Arr[i]);
         if((check_regular_file(fullpath))==0){
                    char subDir[300] ={};
                    strcat(subDir,basePath);
                    strcat(subDir,"/");
                    strcat(subDir,Arr[i]);
                    //level +=2;
                    //sortprint(subDir);
                }
    }*/
    /*for(int i=0; i<count; i++){
        //printf("%s\n",Arr[i]);
        int res = is_regular_file(Arr[i]);
       // printf("%d\n",res);
        if(res == 0){
            char subDir[300] ={};
            strcat(subDir,basePath);
            strcat(subDir,"/");
            strcat(subDir,Arr[i]);
            root +=2;
            sortprint(subDir);
        }
        root = 0;
    }*/
    for(int i=0; i<count; i++){
        free(Arr[i]);
    }
    free(Arr);
    closedir(dir);
}

void tree(char *basePath, const int root)
{
    int i;
    //printf("dd\n");
   // char path[1000];
    struct dirent *dp;
    //printf("Reading in %s\n",basePath);
    DIR *dir = opendir(basePath);
    if(dir == NULL){
        printf("error\n");
        return;
    }
    //printf("%s\n",basePath);
    //if (!dir)
       //printf("error\n");
      //  return;
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
            //printf("%s%c%s\n", "|>", '-', dp->d_name);
            else{
                printf("  x%s\n",dp->d_name);
            }
            if(dp->d_type == 4){
            char path[300];
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            tree(path, root + 2);
            //printf("root:%d\n",root);
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
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

int inc= 0;
int inc1 = 1;
void indent(int i,int level){
    if(i >= level){
        return;
    }
    printf("\t");
    indent(i+1,level);
}
void recursePrint(char *currDir){
    DIR * inDir = opendir(currDir);
    if(inDir == NULL){
        printf("Error in opening directory\n");
        exit(EXIT_FAILURE);
    }
    //printf("Reading files in: %s\n", currDir);
    struct dirent *directory;
    
    while( (directory = readdir(inDir)) != NULL){
        
        if((directory->d_type == 4) && (strcmp(directory->d_name,".")!=0) 
        && (strcmp(directory->d_name,"..")!=0)){
            //inc++;
            //printf("index:%d %s\n",inc1,directory->d_name);
           //printf("%d %s %s\n",directory->d_type,currDir,directory->d_name);
            char subDir[300] ={};
            strcat(subDir,currDir);
            strcat(subDir,"/");
            strcat(subDir,directory->d_name);
            //printf("\t%s\n",directory->d_name);
            int i =0;
            inc++;
            indent(i,inc);
            printf("%s\n",directory->d_name);
            recursePrint(subDir);
            //printf("\t");
            //inc = 0;
            //inc1++;
            //inc++;
        }
        else{
            if((strcmp(directory->d_name,".")!=0) && (strcmp(directory->d_name,"..")!=0)){
            //printf("%s/%s\n",currDir,directory->d_name);
            inc++;
            int i =0;
            indent(i,inc);
            printf("%s\n",directory->d_name);
            }
        }
    }
    closedir(inDir);
}

int main(int argc, char *argv[]){
    if(argc > 1){
        printf("Too many Arguments\n");
        exit(EXIT_FAILURE);
    }
    char * d = "tree.dSYM";
    printf("%s\n",d);
    recursePrint("tree.dSYM");
    return EXIT_SUCCESS;
}*/
/**
 * void recursePrint(char *currDir){
    DIR * inDir = opendir(currDir);
    if(inDir == NULL){
        printf("Error in opening directory\n");
        exit(EXIT_FAILURE);
    }
    printf("Reading files in: %s\n", currDir);
    struct dirent *directory;
    
    
        if(directory->d_type == 8){
            inc++;
            printf("indexh:%d %s\n",inc,directory->d_name);
        }
        if((directory->d_type == 4) && (strcmp(directory->d_name,".")!=0) 
        && (strcmp(directory->d_name,"..")!=0)){
            //inc++;
            printf("index:%d %s\n",inc1,directory->d_name);
           //printf("%d %s %s\n",directory->d_type,currDir,directory->d_name);
            char subDir[300] ={};
            strcat(subDir,currDir);
            strcat(subDir,"/");
            strcat(subDir,directory->d_name);
            //printf(" ");
            recursePrint(subDir);
            inc1++;
        }
    }
    closedir(inDir);
}
*/