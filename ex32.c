/**
 * Name : Limor Levi
 * ID : 308142389
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <wait.h>

#define TRUE 1
#define FALSE 0

#define STDERR 2
#define ERROR_MESSAGE "Error in system call\n"

#define LENGTH_OF_DIRECTORY_PATH 200
#define EXECUTABLE_FILE_NAME "prog_s"
#define LINE_LENGTH 160
#define FAILURE -1

//function declarations
void failure();
int isCFile(char* file);
int isObjectFileExist();
int compareBetweenFiles(char* usersOutput,char* correctOutput);
int getLevelOfLossingPoints(char* pathToInputFile, char* pathToCorrectOutputFile);
int compileAndRunCFile(char* cFilePath, char* pathToInputFile, char* pathToCorrectOutputFile);
void buildFullPathToCFile(char* fullPath, char* pathOFCurrentWorkingDirectory, char* fileName);
int searchCFileInSubDirectories(char* directoryPath, char* pathOfInputFile, char* pathOfCorrectOutputFile);
void writeGradeToResultsFile(int resultCsvFile, char* userName, int userGrade);

/**limor
 * the main function of the program
 * @param argc - the number of command line arguments
 * @param argv - an array of command line arguments
 */
int main(int argc, char *argv[]) {
    char buffer[LINE_LENGTH * 3], *pathOfInputFile, *pathOfCorrectOutputFile, *pathOfUsersDirectory,path[LENGTH_OF_DIRECTORY_PATH];
    int returnValue, result, csvResult, configurationFile = open(argv[1], O_RDONLY);
    DIR *pathToUsersDirectory;
    struct dirent* subDirectory;
    if (configurationFile < 0) {
        failure();
        return 0;
    }

    // read all the lines from the configuration file and then split them to three paths
    if ((returnValue = read(configurationFile, buffer, LINE_LENGTH * 3)) <= 0 ) {
        failure();
        return 0;
    }
    buffer[returnValue] = '\0';
    pathOfUsersDirectory = strtok(buffer, "\n");
    pathOfInputFile = strtok(NULL, "\n");
    pathOfCorrectOutputFile =  strtok(NULL, "\n");

    //open the users' directory and create the result.csv file
    csvResult = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (((pathToUsersDirectory = opendir(pathOfUsersDirectory)) == NULL)||(csvResult < 0)) {
        failure();
        exit(11);
    }

    // scanning all the files and sub-directories in the directory to find the C file
    while ((subDirectory = readdir(pathToUsersDirectory)) != NULL) {
        if (strcmp(subDirectory->d_name, ".") && strcmp(subDirectory->d_name, "..")) {
            buildFullPathToCFile(path,pathOfUsersDirectory,subDirectory->d_name);
            result = searchCFileInSubDirectories(path, pathOfInputFile, pathOfCorrectOutputFile);
            if ((result == 11) || (result == 10)) {
                exit(11);
            }
            writeGradeToResultsFile(csvResult, subDirectory->d_name,result);
        }
    }

    if ((close(csvResult) < 0 )||(close(configurationFile) == FAILURE))  {
        failure();
    }
    closedir(pathToUsersDirectory);
    return 0;
}

/**limor
 * the function prints error message to the error chanel (number 2)
 */
void failure() {
    write(STDERR, ERROR_MESSAGE, sizeof(ERROR_MESSAGE));
}

/**limor
 * the function checks if the file is c file or not
 * @param file - the file's name
 */
int isCFile(char* file) {
    file = strrchr(file, '.');
    if( file != NULL )
        if (!strcmp(file, ".c")) {
            return TRUE;
        }
    return FALSE;
}

/**limor
 * the function checks if an object file is exist in the directory
 */
int isObjectFileExist() {
    int isFound = FALSE;
    char path[LENGTH_OF_DIRECTORY_PATH];
    getcwd(path, LENGTH_OF_DIRECTORY_PATH);
    DIR* directoryPath;
    if ((directoryPath = opendir(path)) == NULL) {
        exit(1);
    }
    //scanning all the files in the directory until we found the c file(if exist)
    struct dirent*  fileInDirectory;
    while ((fileInDirectory = readdir(directoryPath)) != NULL) {
        if (!strcmp(fileInDirectory->d_name, EXECUTABLE_FILE_NAME)) {
            isFound = TRUE;
            break;
        }
    }

    closedir(directoryPath);
    return isFound;
}

/**limor
 * the function compares between the output of the program and the correct output
 * @param usersOutput - path to the file that contains the user's program output
 * @param correctOutput - path to the file that contains the correct output of the program
 */
int compareBetweenFiles(char* usersOutput,char* correctOutput) {
    //arrange the arguments for 'comp.out' file
    char* argumentsForCompOut[] = {"./comp.out", correctOutput, usersOutput, NULL};
    int pid, status, returnValueFromCompOut;
    if ((pid = fork()) < 0) {
        failure();
        exit(FAILURE);
    }

    if (pid == 0) {
        //the son process
        execvp(argumentsForCompOut[0], argumentsForCompOut);
        exit(11);
    }
    if (pid > 0) {
        //the dad process
        waitpid(pid, &status, 0);
        returnValueFromCompOut = WEXITSTATUS(status);
        if (returnValueFromCompOut == 11) {
            exit(11);
        }
        return returnValueFromCompOut;
    }
}


/**limor
 * the function runs the user's program, compares the output to the correct output and determines it's grade
 * @param pathToInputFile - the path to the input file
 * @param pathToCorrectOutputFile - the path to the correct output file
 */
int getLevelOfLossingPoints(char* pathToInputFile, char* pathToCorrectOutputFile) {
    int status,inputFile, outputFile,pid;
    if ((pid = fork()) < 0) {
        failure();
        exit(FAILURE);
    }

    if (pid == 0) {
        //the son process
        inputFile = open(pathToInputFile, O_RDONLY);
        if (inputFile == FAILURE) {
            failure();
            exit(10);
        }

        // delete an existing output file (if exist)
        unlink("output");
        outputFile = open("output",  O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (outputFile == FAILURE) {
            failure();
            exit(10);
        }

        //replace the standard input with the input file and replace the standard output with the output file
        if ((dup2(inputFile, STDIN_FILENO) == FAILURE) || (dup2(outputFile, STDOUT_FILENO) == FAILURE)) {
            failure();
            exit(10);
        }

        char* argumentsForTheUserProgram[] = {"./prog_s", NULL};
        execvp(argumentsForTheUserProgram[0], argumentsForTheUserProgram);
        exit(1);
    } else {
        //the dad process
        sleep(5); //wait 5 seconds
        if (waitpid(pid, &status, WNOHANG) == 0) {
            return 5;
        }
    }
    return compareBetweenFiles("output", pathToCorrectOutputFile);
}

/**limor
 * the function compiles and runs the c file
 * @param cFilePath - the path to the c file
 * @param pathToInputFile - the path to the input file
 * @param pathToCorrectOutputFile - the path to the correct output file
 */
int compileAndRunCFile(char* cFilePath, char* pathToInputFile, char* pathToCorrectOutputFile) {
    char* argumentsForGcc[] = {"gcc", "-o", EXECUTABLE_FILE_NAME, cFilePath, NULL};
    int returnValue, pid;
    if ((pid = fork()) < 0) {
        failure();
    }

    if (pid == 0) {
        // the son process
        unlink(EXECUTABLE_FILE_NAME);
        execvp(argumentsForGcc[0], &argumentsForGcc[0]);
        exit(4);
    } else {
        // the dad process
        waitpid(pid, NULL, 0);
        if (!isObjectFileExist()) {
            returnValue = 4;
        } else {
            returnValue = getLevelOfLossingPoints(pathToInputFile, pathToCorrectOutputFile);
        }
    }
    return returnValue;
}

/**limor
 * the function builds the full path to the c file
 * @param fullPath - the string that should contain the full path to the c file
 * @param pathOFCurrentWorkingDirectory - path of the current working directory
 * @param fileName - the name of the c file
 */
void buildFullPathToCFile(char* fullPath, char* pathOFCurrentWorkingDirectory, char* fileName) {
    strcpy(fullPath, pathOFCurrentWorkingDirectory);
    strcat(fullPath, "/");
    strcat(fullPath, fileName);
}

/**limor
 * the function searches for C file in the user's directory and in sub-directories in the user's directory.
 * if a C file is exist - compile it,run it and calculate the user's grade.
 * @param directoryPath - the path of the directory
 * @param pathOfInputFile - the path to the input file
 * @param pathOfCorrectOutputFile - the path to the correct output file
 */
int searchCFileInSubDirectories(char* directoryPath, char* pathOfInputFile, char* pathOfCorrectOutputFile) {
    struct stat status;
    char initialPath[LENGTH_OF_DIRECTORY_PATH];
    int isFoundCFile = FALSE, returnValue = 6;
    DIR* pathToDirectory;
    struct dirent*  fileInDirectory;
    if ((pathToDirectory = opendir(directoryPath)) == NULL) {
        failure();
    }

    //scanning all the files in the directory until finding C file
    while ((fileInDirectory = readdir(pathToDirectory)) != NULL) {
        if (strcmp(fileInDirectory->d_name, ".") && strcmp(fileInDirectory->d_name, "..")) {
            buildFullPathToCFile(initialPath, directoryPath, fileInDirectory->d_name);
            if (stat(initialPath, &status) == FAILURE) {
                exit(11);
            }
            //in case that we found the C file
            if (isCFile(fileInDirectory->d_name) && !S_ISDIR(status.st_mode)) {
                isFoundCFile = TRUE;
                returnValue = compileAndRunCFile(initialPath, pathOfInputFile, pathOfCorrectOutputFile);
            }
        }
    }
    closedir(pathToDirectory);

    if (isFoundCFile) {
        return returnValue;
    } else {
        //scanning all the sub-directories in the directory until finding C file
        if ((pathToDirectory = opendir(directoryPath)) == NULL) {
            failure();
        }

        while ((fileInDirectory = readdir(pathToDirectory)) != NULL) {
            if (!strcmp(fileInDirectory->d_name, ".") || !strcmp(fileInDirectory->d_name, "..")) {
                continue;
            }
            buildFullPathToCFile(initialPath, directoryPath, fileInDirectory->d_name);
            if (stat(initialPath, &status) == FAILURE) {
                exit(11);
            } else {
                if (S_ISDIR(status.st_mode)) {
                    buildFullPathToCFile(initialPath, directoryPath, fileInDirectory->d_name);
                    returnValue = searchCFileInSubDirectories(initialPath, pathOfInputFile, pathOfCorrectOutputFile);
                    if (returnValue != FAILURE) {
                        closedir(pathToDirectory);
                        return returnValue;
                    }
                }
            }
        }
    }
    return 6;
}

/**limor
 * the function writes the grades of the users to the result.csv file
 * @param resultCsvFile - the path to the result.csv file
 * @param userName - the user's name
 * @param userGrade - the user's grade
 */
void writeGradeToResultsFile(int resultCsvFile, char* userName, int userGrade) {
    char lineToWrite[LENGTH_OF_DIRECTORY_PATH] ="";
    strcpy(lineToWrite, userName);
    strcat(lineToWrite, ",");
    switch (userGrade) {
        case 1:
            strcat(lineToWrite, "60,BAD_OUTPUT\n");
            break;
        case 2:
            strcat(lineToWrite, "80,SIMILAR_OUTPUT\n");
            break;
        case 3:
            strcat(lineToWrite, "100,GREAT_JOB\n");
            break;
        case 4:
            strcat(lineToWrite, "0,COMPILATION_ERROR\n");
            break;
        case 5:
            strcat(lineToWrite, "0,TIMEOUT\n");
            break;
        case 6:
            strcat(lineToWrite, "0,NO_C_FILE\n");
            break;
        default:
            break;
    }

    if (write(resultCsvFile, lineToWrite, strlen(lineToWrite)) < 0) {
        failure();
        exit(11);
    }
}

