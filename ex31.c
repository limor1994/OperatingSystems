/**
 * Name : Limor Levi
 * ID : 308142389
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define STDERR 2
#define ERROR_LENGTH 20
#define FILES_ARE_DIFFERENT 1
#define FILES_ARE_SIMILAR 2
#define FILES_ARE_EQUAL 3

//functions declaration
int isFilesEqualSimilarDifferent(int firstFile, int secondFile);
int failure(int result);
int isLetter(char ch);
void closeFiles(int firstFile, int secondFile);
int isMoreCharacterToRead(int result1, int result2);
int isCharacterSimilarToSpaceOrEnter(char ch);
int isCharactersEqualOrSimilar(char ch1, char ch2) ;
/**
 * the main function of the program.
 * @param argc - the number of command-line arguments
 * @param argv - an array of command-line arguments
 * @return exit status
 */
int main(int argc, char **argv) {
    //open the files
    int firstFile = open(argv[1], O_RDONLY);
    if (failure(firstFile) == -1) {
        return -1;
    }
    int secondFile = open(argv[2], O_RDONLY);
    if (failure(secondFile) == -1) {
        return -1;
    }
    //check if the files are equal,similar of different and return the answer.
    return isFilesEqualSimilarDifferent(firstFile, secondFile);
}

/**
 * the function checks if the files are equal, similar or different.
 * @param firstFile - the first file
 * @param secondFile -  The second file
 * @return 3 if the files are equal
 *         2 if the files are similar
 *         1 otherwise.
 */
int isFilesEqualSimilarDifferent(int firstFile, int secondFile) {
    int filesStatus = FILES_ARE_EQUAL,result1, result2;
    char ch1='\0', ch2='\0';
    //read the first character from the files before the while loop
    result1 = read(firstFile, &ch1, sizeof(ch1));
    if (failure(firstFile) == -1) {
        return -1;
    }
    result2 = read(secondFile, &ch2, sizeof(ch2));
    if (failure(secondFile) == -1) {
        return -1;
    }

    while (isMoreCharacterToRead(result1, result2)) {
        if ((filesStatus == FILES_ARE_EQUAL) && (ch1 != ch2)) {
            //change the status of the files to 'similar'
            filesStatus = FILES_ARE_SIMILAR;
        }

        if (filesStatus == FILES_ARE_SIMILAR) {
            //skip spaces and '\n' in the files
            while ((result1 > 0) && (isCharacterSimilarToSpaceOrEnter(ch1))) {
                result1 = read(firstFile, &ch1, sizeof(ch1));
            }
            while ((result2 > 0) && (isCharacterSimilarToSpaceOrEnter(ch2))) {
                result2 = read(secondFile, &ch2, sizeof(ch2));
            }
            if (isCharactersEqualOrSimilar(ch1, ch2) == FALSE) {
                return FILES_ARE_DIFFERENT;
            }
        }

        //read the next character from the files
        result1 = read(firstFile, &ch1, sizeof(ch1));
        if (failure(firstFile) == -1) {
            return -1;
        }
        result2 = read(secondFile, &ch2, sizeof(ch2));
        if (failure(secondFile) == -1) {
            return -1;
        }
    }
    if (filesStatus == FILES_ARE_EQUAL && (result1 != result2)) {
        filesStatus = FILES_ARE_SIMILAR;
    }
    if (filesStatus == FILES_ARE_SIMILAR) {
        //skip spaces and '\n' in the files
        while ((result1 > 0) && (isCharacterSimilarToSpaceOrEnter(ch1))) {
            result1 = read(firstFile, &ch1, sizeof(ch1));
        }
        while ((result2 > 0) && (isCharacterSimilarToSpaceOrEnter(ch2))){
            result2 = read(secondFile, &ch2, sizeof(ch2));
        }
        if (!(isCharactersEqualOrSimilar(ch1,ch2)||
              isCharacterSimilarToSpaceOrEnter(ch1)||
              isCharacterSimilarToSpaceOrEnter(ch2))) {
            filesStatus = FILES_ARE_DIFFERENT;
        }
    }
    //close the files
    closeFiles(firstFile, secondFile);
    return filesStatus;
}

/**
 * the function checks if there is failure.
 * @param result - the result of the system call
 * @return an error sign (=-1) in case of failure
 *         TRUE (=1) otherwise
 */
int failure(int result) {
    if (result == -1) {
        //write an error to the error channel
        write(STDERR, "Error in system call\n", ERROR_LENGTH);
        return -1;
    }
    return TRUE;
}

/**
 * the function checks if the char is a letter or not.
 * @param ch -  The character
 * @return TRUE (=1) if the character ch is a letter
 *         FALSE (=0) otherwise
 */
int isLetter(char ch) {
    if (('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z')) {
        return TRUE;
    }
    return FALSE;
}

/**
 * the function closes both of the files.
 * @param firstFile - the first file
 * @param secondFile - the second file
 */
void closeFiles(int firstFile, int secondFile){
    close(firstFile);
    close(secondFile);
}

/**
 * the function checks if there is more characters to read.
 * @param result1 - the result of reading character from the first file
 * @param result2 - the result of reading character from the second file
 * @return TRUE (=1) if there is more characters to read from both of the files
 *         FALSE otherwise
 */
int isMoreCharacterToRead(int result1, int result2){
    if((result1 > 0) && (result2 > 0)){
        return TRUE;
    }
    return FALSE;
}

/**
 * the function checks if the character ch is equal to space or '\n'.
 * @param ch - the character
 * @return TRUE (=1) if ch is equal to space of '\n'
 *         FALSE (=0) otherwise.
 */
int isCharacterSimilarToSpaceOrEnter(char ch){
    if(ch == ' ' || ch == '\n'){
        return TRUE;
    }
    return FALSE;
}

/**
 * the function checks is character ch1 and ch2 are equal or similar to each other.
 * @param ch1 - the first char
 * @param ch2 - the second char
 * @return TRUE (=1) if the characters are equal or similar
 *         FALSE (=0) otherwise
 */
int isCharactersEqualOrSimilar(char ch1, char ch2) {
    if (ch1 == ch2 || (isLetter(ch1) == 1 && isLetter(ch2) == 1 && (ch1 + ' ' == ch2 || ch1 == ch2 + ' '))){
        return TRUE;
    }
    return FALSE;
}
