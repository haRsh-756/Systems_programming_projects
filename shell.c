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
/*void sig_tstp(int sig){
    //signal(SIGTSTP,sig_tstp);
    signal(SIGTSTP,SIG_IGN);
    //write(STDOUT_FILENO,"\n",1);
    //write(STDOUT_FILENO,"Ctrl-z detected\n",17);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTSTP,sig_tstp);
    kill(pid,SIGTSTP);
    return;
}*/
/*void sigint_handler(int sig){
    //signal(SIGINT,sigint_handler);
    signal(SIGINT,SIG_IGN);
    //write(STDOUT_FILENO,"Ctrl-c detected\n",17);
    //write(STDOUT_FILENO,"\n",1);
    signal(SIGTTOU,SIG_IGN);
    signal(SIGINT,sigint_handler);
    //signal(SIGTTOU,SIG_IGN);
    kill(pid,SIGINT);
    return;
}*/
//char pwd[50]={};
                //if(getcwd(pwd,sizeof(pwd))!=NULL){
                //    printf("Current working dir: %s\n",pwd);
                //}
/*void getJobStatus(){
    pid_t wpid = 0;
    for(int i=0; i< inc; i++){
        int status;
        wpid = waitpid(jobs[i].pI,&status,WNOHANG);
        if(wpid > 0){
            jobs[i].state = done;
            printf("jobs state:%d\n",jobs[i].state);
            printf("[%d] %d Done %s\n",jobs[i].jobID,wpid,jobs[i].command);
        }
    }
}*/
/*void getJobStatus(int cpid){
    int status;
    pid_t wpid = waitpid(cpid,&status,WUNTRACED);
    if(WIFSIGNALED(status)){
        printf("[%d] %d terminated by signal %d\n",inc+1,wpid,WTERMSIG(status));
    }
}*/
 //int status;
                    /*pid_t wpid = waitpid(jobs[i].pI,&status,WUNTRACED);
                    if(WIFSTOPPED(status)){
                        printf("[%d] %d Stopped %s\n",jobs[i].jobID,wpid,jobs[inc].command);
                        jobs[inc].state = stopped;
                    }
                    else if(WIFSIGNALED(status)){
                        printf("[%d] %d terminated by signal %d\n",jobs[i].jobID,wpid,WTERMSIG(status));
                        jobs[inc].state = terminated;
                    }
                    else{
                        printf("Child %d terminated abnormally\n",wpid);
                    }*/
/*void getJobStatus(pid_t cpid){
    int status;
    pid_t wpid =waitpid(cpid,&status,WNOHANG);
    for(int i=0; i<inc; i++){
        if(jobs[i].pI == cpid){
            if(WIFEXITED(status)){
                //printf("child %d exited with exit status%d\n",wpid,WEXITSTATUS(status)); 
                //jobs[inc].state = done;     
            }
            else if(WIFSIGNALED(status)){
                printf("[%d] %d terminated by signal %d\n",jobs[i].jobID,wpid,WTERMSIG(status));
                //jobs[inc].state = terminated;
            }
            else if(WIFSTOPPED(status)){
                printf("[%d] %d Stopped %s\n",jobs[i].jobID,wpid,jobs[inc].command);
                //jobs[inc].state = stopped;
            }
            else{
                printf("Child %d terminated abnormally\n",wpid);
            }
            break;
        }
    }
    return;
}*/
/*void changeWorkingDirectory(char * args[], int *homeFlag){
    if(args[1] == NULL){
        char * homeVar =  getenv("HOME");
        printf("home:%s\n",homeVar);
        char p[100]={};
        strcat(p,"PWD=");
        strcat(p,homeVar);
        putenv(p);
        setenv("PWD",homeVar,1);
        //updatedPath=  getenv("PWD");
        *homeFlag = 1;
        //printf("df:%s\n",updatedPath);
    }
    else{
    char * path = getenv("PWD");
    printf("path is %s\n",path);
    int fs = 2;
    if(*homeFlag == 1){
        char temp[256] = {};
        strcat(temp,path);
        strcat(temp,"/");
        strcat(temp,args[1]);
        fs= chdir(temp);
        *homeFlag = 0;
    }
    else{            
        fs = chdir(args[1]);
    }
    printf("fs:%d\n",fs);
    char pwd[50]={};
    if(getcwd(pwd,sizeof(pwd))!=NULL){
        printf("Current working dir: %s\n",pwd);
    }
    char p[100]={};
    strcat(p,"PWD=");
    strcat(p,pwd);
    //printf("p:%s\n",p);
    putenv(p);
    char * pWd = getenv("PWD");
    printf("now path is %s\n",pWd);
        setenv("PWD",pwd,1);
       // tempCopy = pWd;
    }
}*/
/*void noAbsRelPath(char * args[], char * env[]){
    printf("dsd\n");
    char fullpath[100];
    strcat(fullpath,"/usr/bin");
    strcat(fullpath,"/");
    strcat(fullpath,args[0]);
    if(access(fullpath,F_OK)==0){
        //builtInCmdFlag = 1;
        if(execve(fullpath,args,env)==-1){
            char fullpath1[100];
            strcat(fullpath1,"/bin");
            strcat(fullpath1,"/");
            strcat(fullpath1,args[0]);
            if(access(fullpath1,F_OK)==0){
                //builtInCmdFlag = 1;
                if(execve(fullpath1,args,env)==-1){
                    printf("%s: command not found\n",args[0]);
                    exit(0);
                }
            }
            exit(0);
        }
    }
    return;
}
void noAbsRelPath(char*args[],char * env[]){
    //if(execve(args[0],args,env)== -1){
        //printf("level-1\n");
        char fullpath[100] ={};
        strcat(fullpath,"/usr/bin");
        strcat(fullpath,"/");
        strcat(fullpath,args[0]);
        if(execve(fullpath,args,env)==-1){
           // printf("level-2\n");
            char fullpath_[100] ={};
            strcat(fullpath_,"/bin");
            strcat(fullpath_,"/");
            strcat(fullpath_,args[0]);
            if(execve(fullpath_,args,env)==-1){
                //printf("inner\n");
                printf("%s: command not found\n",args[0]);
                //exit(0);
            }
            //exit(0);
        }
    //exit(0);
    //}
    return;
}*/
 /** continuing process;
                 * printf("starting process\n");
                kill(pid,SIGCONT);
                wpid = waitpid(pid,&status,WUNTRACED);
                if(WIFEXITED(status)){
                    printf("exit\n");
                }*/
                //if(inc < 30){
                //     jobs[inc].pI = wpid;
                //     jobs[inc].jobID = 1;
                //     inc++;
                //}
                //kill(wpid,SIGCONT);
                 //int status1;
                //waitpid(wpid,&status1,WUNTRACED);
/*if(foreground == false){
           printf("pgid: %d\n",pgid);
           printf("jobPI: %d\n",jobs[0].pI);
           printf("pgid:%d\n",getpgid(jobs[0].pI));
           int f = setpgid(getpgid(jobs[0].pI),pgid);
           //int f = setpgid(pgid,jobs[0].pI);
           printf("errno:%d",errno);
           printf("f:%d\n",f);
           printf("new pgid:%d\n",getpgid(jobs[0].pI));
           }*/
            /*printf("value:%d\n",val);
            
            for(int i=0; i < inc; i++){
                if(jobs[i].state == stopped && jobs[i].jobID == val){
                    printf("[%d] %d Stopped %s\n",jobs[i].jobID,jobs[i].pI,jobs[i].command);
                    kill(jobs[i].pI,SIGCONT);
                    int status;
                    pid_t wpid = waitpid(jobs[i].pI,&status,WUNTRACED);
                    if(WIFEXITED(status)){
                        printf("child %d exited with exit status%d\n",wpid,WEXITSTATUS(status));
                    }
                }
            }*/
            
            //if(foreground == false){
              //  printf("jobiD%d\n",jobs[0].jobID);
              //  if(jobs[0].jobID == val){
                    //setpgid(pgid,jobs[0].pI);
                    //setpgid(jobs[0].pI,pgid);
               //     printf("pgid: %d\n",pgid);
                //    printf("set:%d",getpgid(jobs[0].pI));
                    //setpgid(getpgid(jobs[0].pI),pgid);
                 //   setpgid(pgid,jobs[0].pI);
                   // setpgid(jobs[0].pI,pgid);
                    //setpgid(pgid,jobs[0].pI);
                  //  printf("set2:%d",getpgid(jobs[0].pI));
                    //setpgid(jobs[0].pI,pgid);
                   /* int cstatus;
                    pid_t wpid = waitpid(jobs[0].pI,&cstatus,WUNTRACED);
                    if(WIFEXITED(cstatus)){
                        printf("child %d exited with exit status%d\n",wpid,WEXITSTATUS(cstatus));
                    }*/
                //}
                
           // }
            //foreground = true;
            //break;
/*if(args[1] == NULL){
               char * homeVar =  getenv("HOME");
               printf("home:%s\n",homeVar);
               char p[100]={};
               strcat(p,"PWD=");
               strcat(p,homeVar);
               putenv(p);
               setenv("PWD",homeVar,1);
                strcat(tempCopy,p);  
               //tempCopy= p;
               //printf("temp:%s\n",tempCopy);
               //updatedPath=  getenv("PWD");
               homeFlag = 1;
               printf("df:%s\n",getenv("PWD"));
            }
            else{
            char * path = getenv("PWD");
            printf("path was %s\n",path);
            int fs = 2;
            if(homeFlag == 1){
                char temp[256] = {};
                strcat(temp,path);
                strcat(temp,"/");
                strcat(temp,args[1]);
                fs= chdir(temp);
                homeFlag = 0;
            }
            else{            
                fs = chdir(args[1]);
                if(fs != 0){
                    printf("cd: no such file or directory: %s\n",args[1]);
                    continue;
                }
            }
            printf("fs:%d\n",fs);
            char pwd[50]={};
            if(getcwd(pwd,sizeof(pwd))!=NULL){
                printf("Current working dir: %s\n",pwd);
            }
            char p[100]={};
            strcat(p,"PWD=");
            strcat(p,pwd);
            //printf("p:%s\n",p);
            putenv(p);
            char * pWd = getenv("PWD"); 
           
            printf("now path is %s\n",pWd);
                setenv("PWD",pWd,1);
                strcat(tempCopy,p);
                //tempCopy = p;
                // tempCopy = getenv("PWD");
                //tempCopy = pWd;
            }*/
            //printf("pWd: %s\n",tempCopy);
            //sigprocmask(SIG_SETMASK, &prev_mask1,NULL);
            //sigprocmask(SIG_SETMASK, &prev_mask, NULL);


//int main(int argc, char **argv){
    //execve(pathname,argv,envp)
    //char text[100];
    //printf("> ");
    //int fs = scanf("%[^\n]s",text);
    //printf("fs%d\n",fs);
    //getchar();
    //char * absPath[] = {"usr/bin",}
   // while( fs== 1 || fs ==0){
        //printf("text:%s\n",text);
   //     getchar();
   //     printf("text:%d\n",strlen(text));
   //     printf("> ");
        //fs = scanf("%[^\n]%*[ ]s",text);
    //    fs = scanf("%[^\n]s",text);
        
        //fs = scanf("%[^\n]%*[ ]",text);
       // printf("text:%s\n",text);
   //     if(strcmp(text,"exit")==0){
   //         exit(0);
   //     }
        //printf("text:%s",text);
        //printf("fs:%d\n",fs);
        //break;
  //  }*/
  //  int fs =1;
  ///  while(fs == 1){
        //printf("> ");
        //fs = scanf("%[^\n]s",text);
        //fs = scanf("",text);
        //if(fs == 0){
            //printf("valye:%d\n",fs);
          //  getchar();
          //  fs = 1;
         //   continue;
       // }
        //else if(fs == -1){
        //    break;
       // }
        //printf("fs:%d\n",fs);
        /*if(fs == 0){
            getchar();
        }*/
        //getchar();
        //char * token = strtok(text," ");
        //printf("Token:%s\n",token);
        //char * args[10];
        //int index = 0;
        //while(1){
           // args[index++] = token;
           // if(token == NULL){
             //   break;
           // }
         //   token = strtok(NULL," ");
       // }

     //   for(int i=0; i < index; i++){
     //       printf("%s\n",args[i]);
       // }
        /*printf("running execv\n");
        pid_t pid;
        pid = fork();
        if(pid == -1){
            return -1;
        }
        if(pid == 0){
            if(execv(args[0],args)== -1){
                char fullpath[300] ={};
                strcat(fullpath,"/bin");
                strcat(fullpath,"/");
                strcat(fullpath,args[0]);
                if(execv(fullpath,args) == -1){
                    perror("execv1:");
                    exit(0);
                }
                perror("execv");
                exit(0);
            }        
        }
        else{
            wait(NULL);
            printf("done with child\n");
        }
        printf("done with execv\n");*/
        /*while(token != NULL){
            printf(" Token:%s\n",token);
            //int value = atoi(token);
            //push(value);
            token = strtok(NULL," ");
        } */       
        //printf("text:%s\n",text);
        //char c = getchar();
        /*if(fs == 0){
            getchar();
        }*/
        //printf("bb:%c",c);
       // if(strcmp(text,"exit")==0){
       //     exit(0);
      //  }
        //printf("text:%s\n",text);
    //}
    //printf("text:%s",text);
   // return EXIT_SUCCESS;
//}
 //char pwd[50];
            /*if(strcmp(args[0],))
            char * path = getenv("PWD");
            printf("path is %s\n",path);
            chdir();
            char pwd[50]={};
            if(getcwd(pwd,sizeof(pwd))!=NULL){
                printf("Current working dir: %s\n",pwd);
            }
            char p[100]={};
            strcat(p,"PWD=");
            strcat(p,pwd);
            //printf("p:%s\n",p);
            putenv(p);
            printf("now path is %s\n",getenv("PWD"));
            setenv("PWD",pwd,1);*/
            //printf("path is %s\n",);
            /*if (getcwd(pwd, sizeof(pwd)) != NULL) {
                printf("Current working dir: %s\n", pwd);
            }*/
                   /*if(strstr(check,"../")!=NULL || strstr(check,"./")!=NULL || strstr(check,"/bin/sleep")!=NULL){
           // printf("env%s\n",env[0]);
            if(execve(argv[0],argv,env)==-1){
                printf("%s: No such file or directory\n",argv[0]);
                exit(0);
            }
            //exit(0);
        }
        else{ 
            noAbsRelPath(argv,env);
            exit(0);
        }*/