#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
/**
 * program_name: shell.c
 * author: harsh_patel
*/
static volatile sig_atomic_t pid;
static volatile sig_atomic_t inc = 0;
char tempCopy[256];
enum jobState{stopped, running, terminated, done};
struct job{
    pid_t pI;
    int jobID;
    enum jobState state;   
    char command[30];
    bool foreground;
    //struct job* next;
};
struct job jobs[40];
void trimLeadingSpace(char * input){
    int count = 0;
    while (input[count] == ' ' || input[count] == '\n' || input[count] == '\t') {
        count++;
    }
    if (count != 0){
    int i = 0;
    while (input[i + count] != '\0'){
      input[i] = input[i + count];
      i++;
    }
    input[i] = '\0';
  }
}
void sigint_handler(int sig){
    kill(pid,SIGINT);
}
void sig_tstp_handler(int sig){
    kill(pid,SIGTSTP);
}
void print_jobs(){
    for(int i = 0; i< inc; i++){
        //printf("cmd:%s\n",jobs[i].command);
        switch(jobs[i].state){
            case stopped:
                printf("[%d] %d Stopped %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                //printf("[%d] %d Stopped %s\n",jobs[i].jobID,jobs[i].pI,);
                break;
            case running:
                printf("[%d] %d Running %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                //printf("[%d] %d Stopped %s\n",jobs[i].jobID,jobs[i].pI,s);
                break;
            case terminated:
                printf("[%d] %d Terminated %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                break;
            case done:
                printf("[%d] %d Done %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                break;
            default:
                printf("[%d] %d Invalid %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                break;
        }
    }
    /*for(int i=0 ; i<inc; i++){
        jobs[i] = (struct job){0,0,0,NULL};
        printf("[%d] %d %d %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].state,jobs[i].command);
    }*/
   // printf("Done printing\n");
}
void launch(char *argv[], char *env[], pid_t *pgid, bool foreground,char cmd[]){
    if((pid=fork())== 0){
        setpgid(getpid(),*pgid);
        if(execve(argv[0],argv,env)==-1){
            perror("Something went wrong here");
            exit(1);
        }
        exit(0);
    }
    setpgid(pid,pid);
    jobs[inc].jobID = inc+1;
    jobs[inc].state = running;
    strcat(jobs[inc].command,cmd);
    if (foreground) {
        //printf("here\n");
        int status;
        pid_t wpid = waitpid(pid,&status,WUNTRACED);
        jobs[inc].pI = wpid;
        jobs[inc].foreground = true;
        if(WIFEXITED(status)){
            //printf("child %d exited with exit status%d\n",wpid,WEXITSTATUS(status)); 
            jobs[inc].state = done;     
        }
        else if(WIFSIGNALED(status)){
            printf("\n[%d] %d terminated by signal %d\n",inc+1,wpid,WTERMSIG(status));
            jobs[inc].state = terminated;
        }
        else if(WIFSTOPPED(status)){
            printf("\n[%d] %d Stopped %s\n",inc+1,wpid,jobs[inc].command);
            jobs[inc].state = stopped;
        }
        else{
            printf("Child %d terminated abnormally\n",wpid);
        }   
    }
    else{
        int cstatus;
        waitpid(pid,&cstatus,WNOHANG);
        jobs[inc].pI = pid;
        jobs[inc].foreground = false;
        printf("[%d] %d\n",jobs[inc].jobID,jobs[inc].pI);
    }
    inc++;
    if(inc >= 40){
        printf("out of jobs\n");
        exit(0);
    }
    return;
}
void updateJobStatus(pid_t cpid){
    int status;
    pid_t wpid =waitpid(cpid,&status,WUNTRACED);
    for(int i=0; i<inc; i++){
        if(jobs[i].pI == cpid){
            if(WIFEXITED(status)){
                jobs[i].state = done;
                //printf("child %d exited with exit status%d\n",wpid,WEXITSTATUS(status)); 
            }
            else if(WIFSIGNALED(status)){
                printf("[%d] %d terminated by signal %d\n",jobs[i].jobID,wpid,WTERMSIG(status));
                jobs[i].state = terminated;
            }
            else if(WIFSTOPPED(status)){
                printf("[%d] %d Stopped %s\n",jobs[i].jobID,wpid,jobs[inc].command);
                jobs[i].state = stopped;
            }
            else if(WIFCONTINUED(status)){
                jobs[i].state = running;
            }
            else{
                printf("Child %d terminated abnormally\n",wpid);
            }
            break;
        }
    }
    return;
}
void updateJobs(){
    pid_t wpid=0;
    for(int i=0; i<inc; i++){
        int status;
        wpid = waitpid(jobs[i].pI,&status,WNOHANG);
        //if(wpid > 0 && WIFEXITED(status)){
        if(wpid > 0){
            jobs[i].state = done;
        }
    }
    return;
}
int main(int argc, char * argv[]){
    char *line = NULL;
    char s[100];
    size_t len = 0;
    bool foreground = false;
    sigset_t mask, prev_mask, mask1, prev_mask1;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigemptyset(&mask1);
    sigaddset(&mask1,SIGTSTP);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGINT,SIG_IGN);    
    pid_t pgid = getpgid(getpid());
    int val = 0;

    while(!feof(stdin)){

        foreground = true;
        sigprocmask(SIG_BLOCK, &mask, &prev_mask);
        sigprocmask(SIG_BLOCK, &mask1, &prev_mask1);
        printf("> ");
        getline(&line, &len, stdin);
        trimLeadingSpace(line);
        sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
        sigprocmask(SIG_SETMASK, &prev_mask, NULL);

        if (feof(stdin)){
            for(int i=0; i<inc;i++){
                if(jobs[i].state == stopped){
                    kill(jobs[i].pI,SIGCONT);
                    kill(jobs[i].pI,SIGHUP);
                }
                else if(jobs[i].state == running){
                    kill(jobs[i].pI,SIGHUP);
                }
            }
            printf("\n");
            break;
        }
        if(sscanf(line,"%[^\n]s",s)==1){
            signal(SIGTSTP,sig_tstp_handler);
            signal(SIGINT,sigint_handler);
            sigprocmask(SIG_BLOCK, &mask, &prev_mask);
            sigprocmask(SIG_BLOCK, &mask1, &prev_mask1);
            if(strstr(s,"&")!= NULL){
                foreground = false;
            }
            char str[50];
            strcpy(str,s);
            //printf("str: %s\n",str);
            char * token = strtok(s," ");
            char * args[20];
            char *env[] = {tempCopy,NULL};
            int index = 0;
            while(1){
                args[index++] = token;
                if(token == NULL){
                    break;
                }
                token = strtok(NULL," \t&");
            }
            /*for(int i=0; i < index; i++){
                printf("  %s\n",args[i]);
            }*/
            sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
            sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            if(strcmp(args[0],"exit")==0 || strcmp(args[0],"/bin/exit")==0    
                || strcmp(args[0],"/usr/bin/exit")==0){          //exit command
                    for(int i=0; i<inc;i++){
                        if(jobs[i].state == stopped){
                            kill(jobs[i].pI,SIGCONT);
                            kill(jobs[i].pI,SIGHUP);
                        }
                        else if(jobs[i].state == running){
                            kill(jobs[i].pI,SIGHUP);
                        }
                    }
                break;
            }
            else if(strcmp(args[0],"cd")==0){       //changing working directory
                if(args[1] == NULL){
                    chdir(getenv("HOME"));
                }
                else{
                    chdir(args[1]);
                }
            }
            else if(strcmp(args[0],"jobs")==0){     //jobs
                updateJobs();                       //update and print jobs
                print_jobs();
            }
            else if(strcmp(args[0],"fg")==0 && sscanf(args[1],"%*[^0123456789]%d",&val)==1){ //fg command
                sigprocmask(SIG_BLOCK, &mask, &prev_mask);
                sigprocmask(SIG_BLOCK, &mask1, &prev_mask1);
                for(int i=0; i < inc; i++){
                    if(jobs[i].foreground == false && jobs[i].state == running && jobs[i].jobID == val){
                        printf("# ...and wait...\n");
                        break;
                    }
                    if(jobs[i].state == stopped && jobs[i].jobID == val){
                        //printf("[%d] %d Stopped %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                        //sigprocmask(SIG_BLOCK, &mask, &prev_mask);
                        //sigprocmask(SIG_BLOCK, &mask1, &prev_mask1);
                        // sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
                        //sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        kill(jobs[i].pI,SIGCONT);
                        //jobs[i].state = running;
                        pid = jobs[i].pI;
                        jobs[i].foreground = true;
                        updateJobStatus(jobs[i].pI);
                        //sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
                        //sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        //pid = jobs[i].pI;  //resetting to catch sigint or sigtstp signal
                        break;
                    }
                }
                sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            }
            else if(strcmp(args[0],"bg")==0 && sscanf(args[1],"%*[^0123456789]%d",&val)==1){ //bg command
                sigprocmask(SIG_BLOCK, &mask, &prev_mask);
                sigprocmask(SIG_BLOCK, &mask1, &prev_mask1);
                for(int i =0; i<inc; i++){
                    if(jobs[i].foreground == false && jobs[i].state == running && jobs[i].jobID == val){
                        printf("Already running in the background\n");
                        break;
                    }
                    if(jobs[i].state == stopped && jobs[i].jobID == val){
                        //printf("[%d] %d Stopped %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                        //sigprocmask(SIG_BLOCK, &mask, &prev_mask);
                        //sigprocmask(SIG_BLOCK, &mask1, &prev_mask1);
                        kill(jobs[i].pI,SIGCONT);
                        jobs[i].state = running;
                        if(strcmp(jobs[i].command,"&")!=0){
                            strcat(jobs[i].command," &");
                        }
                        jobs[i].foreground = false;
                        //int status;
                        //getJobStatus(jobs[i].pI);
                        waitpid(jobs[i].pI,NULL,WNOHANG);
                        //sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
                        //sigprocmask(SIG_SETMASK, &prev_mask, NULL);
                        break;
                    }
                }
                sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            }
            else if(strcmp(args[0],"kill")==0 && sscanf(args[1],"%*[^0123456789]%d",&val)==1){ //kill command
                sigprocmask(SIG_BLOCK, &mask, &prev_mask);
                sigprocmask(SIG_BLOCK, &mask1, &prev_mask1);
                for(int i=0; i<inc; i++){
                    if((jobs[i].state == running || jobs[i].state ==stopped) 
                        && jobs[i].jobID == val &&(jobs[i].foreground ==true || jobs[i].foreground ==false)){
                        if(jobs[i].state == stopped){
                            kill(jobs[i].pI,SIGCONT);
                        }
                        kill(jobs[i].pI,SIGTERM);
                        //jobs[i].state = terminated;
                        updateJobStatus(jobs[i].pI);
                        //printf("[%d] %d terminated by signal %d\n",jobs[i].jobID,jobs[i].pI,SIGTERM);
                        break;
                    }
                }
                sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
                sigprocmask(SIG_SETMASK, &prev_mask, NULL);
            }        
            else{
                char fullpath[100]={}, fullpath1[100]={};
                strcat(fullpath,"/usr/bin/");
                strcat(fullpath,args[0]);
                strcat(fullpath1,"/bin/");
                strcat(fullpath1,args[0]);
                if(access(args[0],F_OK)==0){
                    launch(args,env,&pgid,foreground,str);
                }
                else if(access(fullpath,F_OK)==0){
                    args[0] = fullpath;
                    launch(args,env,&pgid,foreground,str);
                }
                else if(access(fullpath1,F_OK)==0){
                    args[0] = fullpath1;
                    launch(args,env,&pgid,foreground,str);
                }
                else if(access(args[0],F_OK)!= 0 && strstr(args[0],"./")!=NULL){
                    printf("%s: No such file or directory\n",args[0]);
                }
                else if(((access(fullpath,F_OK) != 0) || (access(fullpath1,F_OK)!= 0)) && (access(args[0],F_OK)!=0)){
                    printf("%s: command not found\n",args[0]);
                }
                else{
                    printf("%s: No such file or directory\n",args[0]);
                }
            }
        }
    }
    free(line);
    return EXIT_SUCCESS;
}
