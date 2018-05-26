/**
 * Name : Limor Levi
 * Id : 308142389
 */
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0
#define INPUT_SIZE 1000
#define NUM_JOBS 1000

//function declarations
void makeArgsToExecvp(char *args[INPUT_SIZE], char userInput[INPUT_SIZE], int* isBackground);
int systemCall(char **argsToExecvp, int isBackground);
int changeDirectory(char *args[],char prevDirectory[INPUT_SIZE]);
void printBackgroundJobs(int pidarr[], char jobs[NUM_JOBS][INPUT_SIZE], int numJobs);
void exitFromShell(int pidarr[], char jobs[NUM_JOBS][INPUT_SIZE], int numJobs);
int changeToHomeDirectory(char prevDirectory[INPUT_SIZE]);
int changeToSpecificDirectory(char *dir, char prevDirectory[INPUT_SIZE], int isArgumentMinus);

/**
 * main function
 */
    int main() {
        char jobs[NUM_JOBS][INPUT_SIZE], *args[INPUT_SIZE], userInput[INPUT_SIZE];
        char currentDirectory[INPUT_SIZE] = "", copyUserInput[INPUT_SIZE];
        int pidarr[NUM_JOBS], numJobs = 0, isBackground = FALSE, pid;

        //the endless while loop simulates the shell
        while (TRUE) {
            printf("prompt> ");
            fgets(userInput, INPUT_SIZE, stdin);
            //put end-of-string-character in the end of the input
            userInput[strlen(userInput) - 1] = '\0';
            //take care of cases that
            if(strcmp(userInput,"") == 0) {
                continue;
            }else if (strcmp(userInput, "jobs") == 0) {
                printBackgroundJobs(pidarr, jobs, numJobs);
            } else {
                //make copy of 'userInput' because 'strtok' function destroy the original string
                strcpy(copyUserInput, userInput);
                makeArgsToExecvp(args, userInput, &isBackground);

                if (strcmp(args[0], "cd") == 0) {
                    changeDirectory(args, currentDirectory);
                } else {
                    if (strcmp(args[0], "exit") == 0) {
                        exitFromShell(pidarr, jobs, numJobs);
                    }
                    pid = systemCall(args, isBackground);
                    //update pidarr
                    pidarr[numJobs] = pid;
                    strcpy(jobs[numJobs++], copyUserInput);
                }
            }
        }
    }


/**
 * the function make system call by using the execvp function
 * @param argsToExecvp - array of arguments for execvp function
 * @param isBackground - flag that identifies if the parent process need to wait to the son process
 */
    int systemCall(char **argsToExecvp, int isBackground) {
        int stat, isSuccess;
        pid_t pid = fork();
        if (pid == 0) {
            // son
            isSuccess = execvp(argsToExecvp[0], &argsToExecvp[0]);
            if (isSuccess == -1) {
                fprintf(stderr, "Error in system call\n");
                exit(-1);
            }
        } else {
            // dad
            printf("%d \n", pid);
            if (!isBackground) {
                //wait to the son to finish it's job
                wait(&stat);
            }
        }
        return pid;
    }


/**
 * 'cd' command implementation
 * @param args - arguments of the command
 * @param prevDirectory - represents the previous directory
 */
    int changeDirectory(char *args[], char prevDirectory[INPUT_SIZE]) {
        printf("%d\n", getpid());

        //case 1 : 'cd' command has no parameters or recieved '~' parameter-> back to home directory
        if ((args[1] == NULL) || (strcmp(args[1], "~") == 0)) {
            changeToHomeDirectory(prevDirectory);

            //case 2: 'cd' command recieved '..' parameter -> go to previous directory
        } else if (strcmp(args[1], "..") == 0) {
            chdir("..");

        } else if (strcmp(args[1], "-") == 0) {
            changeToSpecificDirectory(prevDirectory, prevDirectory, TRUE);

            //case 3: 'cd' command recieved a path of a directory -> change the working directory
        } else {
            DIR *dir = opendir(args[1]);
            if (dir) {
                changeToSpecificDirectory(args[1], prevDirectory, FALSE);

                //case 4: 'cd' command recieved '.' parameter -> stay in the same directory
            } else if ((strcmp(args[1], ".") == 0)) {

                //case 5: 'cd' command has wrong parameters -> print error message
            } else {
                fprintf(stderr, "Error in system call\n");
                return -1;
            }
        }
    }



/**
 * the function prints all the background jobs
 * @param pidarr - array of pids of the jobs
 * @param jobs - array of the jobs commands
 * @param numJobs - the number of jobs
 */
    void printBackgroundJobs(int pidarr[], char jobs[NUM_JOBS][INPUT_SIZE], int numJobs) {
        int len, i, j;
        pid_t returnPid;
        for (i = 0; i < numJobs; i++) {
            returnPid = waitpid(pidarr[i], NULL, WNOHANG);
            if (returnPid == 0) {
                printf("%d ", pidarr[i]);
                len = strlen(jobs[i]);
                for (j = 0; j < len; j++) {
                    if (!((jobs[i][j] == '&') && (j == len - 1))) {
                        printf("%c", jobs[i][j]);
                    }
                }
                printf("\n");
            }
        }
    }


/**
 * the function exits from the shell
 */
    void exitFromShell(int pidarr[], char jobs[NUM_JOBS][INPUT_SIZE], int numJobs) {
        int i;
        pid_t returnPid;
        printf("%d\n", getpid());
        for (i = 0; i < numJobs; i++) {
            returnPid = waitpid(pidarr[i], NULL, WNOHANG);
            if (returnPid == 0) {
                kill(pidarr[i], SIGKILL);
            }
        }
        exit(0);
    }


/**
 * the function defines specific directory as the working directory
 * @param prevDirectory - the previous working directory
 * @param isBackground - flag that identifies if the parent process need to wait to the son process
 */
    int changeToSpecificDirectory(char *dir, char prevDirectory[INPUT_SIZE], int isArgumentMinus) {
        //find what is the current working directory
        char currentWorkingDirectory[INPUT_SIZE];
        getcwd(currentWorkingDirectory, INPUT_SIZE);

        if (chdir(dir) == -1) {
            fprintf(stderr, "Error in system call\n");
            return -1;
        } else {
            if (isArgumentMinus == TRUE) {
                printf("%s\n", dir);
            }
            strcpy(prevDirectory, currentWorkingDirectory);
            return 1;
        }
    }


/**
 * the function defines the home directory as the working directory
 * @param prevDirectory - the previous working directory
 */
    int changeToHomeDirectory(char prevDirectory[INPUT_SIZE]) {
        //find what is the current working directory
        char currentWorkingDirectory[INPUT_SIZE];
        getcwd(currentWorkingDirectory, INPUT_SIZE);

        if (chdir(getenv("HOME")) == -1) {
            fprintf(stderr, "Error in system call\n");
            return -1;
        } else {
            strcpy(prevDirectory, currentWorkingDirectory);
            return 1;
        }
    }


/**
 * the function arrange the arguments to execvp function
 * @param args - array of arguments to execvp function
 * @param input - the user's input
 * @param isBackground - flag that identifies if the parent process need to wait to the son process
 */
void makeArgsToExecvp(char *args[INPUT_SIZE], char userInput[INPUT_SIZE], int *isBackground) {
    int index = 0,i;
    const char delimeter[2] = " ";
    char *token;
    //get the first token
    token = strtok(userInput, delimeter);
    args[index] = token;
    //walk through other tokens
    while (token != NULL) {
        token = strtok(NULL, delimeter);
        if (token != NULL) {
            if (strcmp(token, "&") != 0) {
                args[++index] = token;
            } else if (strcmp(token, "&") == 0) {
                *isBackground = TRUE;
            }
        }
    }
    args[++index] = NULL;
    //take care of cases that the command includes
    if((args[1]!= NULL) && (args[1][0] == '\"')){
        memmove(args[1], args[1]+1, strlen(args[1]));
        int lastCharIndex = strlen(args[index-1]) -1;
        args[index-1][lastCharIndex] = '\0';
        char concatAll[INPUT_SIZE]="";
        for(i=1;i<index;i++){
            strcat(concatAll, args[i]);
            strcat(concatAll, " ");
        }
        strcpy(args[1], concatAll);
        args[2] = NULL;
        lastCharIndex = strlen(args[1]) -1;
        args[1][lastCharIndex] = '\0';

    }
}