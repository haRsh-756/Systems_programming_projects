#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
/**
 * author: harsh_patel
 * program_name: ls.c
*/
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
void printLongFormat(char* fileName, struct stat Status){
    if(Status.st_mode & S_IRUSR){
        printf("r");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IWUSR){
        printf("w");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IXUSR){
        printf("x");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IRGRP){
        printf("r");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IWGRP){
        printf("w");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IXGRP){
        printf("x");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IROTH){
        printf("r");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IWOTH){
        printf("w");
    }
    else{
        printf("-");
    }
    if(Status.st_mode & S_IXOTH){
        printf("x");
    }
    else{
        printf("-");
    }
    struct group *grp;
    struct passwd *pwd;
    char time[30];
    pwd = getpwuid(Status.st_uid);
    if(pwd != NULL){
        printf(" %s",pwd->pw_name);
    }
    else{
        printf(" userID");
    }
    grp = getgrgid(Status.st_uid);
    if(grp != NULL){
        printf(" %s", grp->gr_name);
    }
    else{
        printf("/group ID");
    }
    printf(" %d",Status.st_size);
    strftime(time,30,"%b %d %H:%M",localtime(&Status.st_mtime));
    printf(" %s",time);
    printf(" %s\n",fileName);
}
int main(int argc, char* argv[]){
    if(argc > 3){
        printf("Too many Arguments\n");
        exit(EXIT_FAILURE);
    }
    struct dirent *dir; 

    DIR *inDr = opendir(".");
  
    if (inDr == NULL){
        printf("Could not open current directory" );
        return 0;
    }
    char** Arr = NULL;
    int index = 0;
    struct stat statStatus;
    while ((dir = readdir(inDr)) != NULL){
        if((strcmp(dir->d_name,".")==0) || (strcmp(dir->d_name,"..")==0)){
            continue;
        }
        else{
            char** temp  = realloc(Arr,(index+1) * sizeof(*temp));
            if(temp == NULL){
                printf("Reallocation failed");
                free(Arr);
                exit(EXIT_FAILURE);
            }
            Arr = temp;
            Arr[index++] = malloc(strlen(dir->d_name)+1);
            strcpy(Arr[index-1],dir->d_name);
            //printf("%s\n", de->d_name);
        }
    }
    Arr = sortArr(Arr,index);
    
    for(int i = 0; i<index; i++){
        if(argc == 2){
            if(strcmp(argv[1],"-l")==0){
                
                if(stat(Arr[i],&statStatus)==0){
                    printLongFormat(Arr[i],statStatus);
                }
            }
    
            else{
                perror("Error Fectcing Info");
                break;
            }
        }
        else{
            printf("%s\n",Arr[i]);
        }
    }
    for(int i = 0; i < index; i++){
        free(Arr[i]);
    }
    free(Arr);
    closedir(inDr);
    return EXIT_SUCCESS;
}