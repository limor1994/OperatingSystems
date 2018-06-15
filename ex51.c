/**
 * Name: Limor Levi
 * ID : 308142389
 */
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

#define FALSE 0
#define TRUE 1
#define STDERR 2

#define RIGHT 'd'
#define LEFT 'a'
#define TURN 'w'
#define DOWN 's'
#define QUIT 'q'

//function declarations
char getch() ;
void failure();
int isValidKey(char ch);

/**
 * the main process of the program
 */
int main() {
    int pipeArgument[2],pid;
    pipe(pipeArgument);
    if ((pid = fork()) < 0) {
        failure();
    }
    if (pid == 0) {
        //the child process
        //change stdin to read in the size of the pipe
        dup2(pipeArgument[0], 0);
        if (execlp("./draw.out", "./draw.out", NULL) == -1){
            failure();
        }
    } else {
        //the dad process
        while (TRUE) {
            char ch = getch();
            if (isValidKey(ch)) {
                if(write(pipeArgument[1], &ch, 1) < 0) {
                    failure();
                }
                //send kill signal
                kill(pid, SIGUSR2);
                if (ch == QUIT) {
                    break;
                }
            }
        }
        return 0;
    }
}

/**
 * the function prints error message to the error channel
 */
void failure() {
    write(STDERR, "Error in system call\n", 21);
}

/**
 * the function checks if the key 'ch' is valid ('d','a','w','s','q')
 * @param ch - the char that the user inserted
 */
int isValidKey(char ch) {
    if((ch==RIGHT)||(ch==LEFT)||(ch==DOWN)||(ch==TURN)||(ch==QUIT)){
        return TRUE;
    } else {
        return FALSE;
    }
}

char getch() {
    char buffer = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buffer, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return buffer;
}


