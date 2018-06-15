/**
 * Name: Limor Levi
 * ID : 308142389
 */
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>

#define SIZE_OF_BOARD 20
#define HORIZONTAL 0
#define VERTICAL 1

#define TRUE 1
#define FALSE 0

#define TURN 'w'
#define LEFT 'a'
#define RIGHT 'd'
#define DOWN 's'
#define QUIT 'q'

//struct Board
typedef struct Board {
    //board properties
    char board[SIZE_OF_BOARD][SIZE_OF_BOARD];
    int mode;

    //board location
    int xCordinate;
    int yCordinate;
} Board;


//function declarations
void moveShape();
void operateAccordingToUserInput();
void changeShapeMode(Board* gameBoard);
void printGameBoard(Board* gameBoard);
void makeMove(Board* gameBoard, int x, int y, int needToTurn);
void deleteShapeFromBoard(Board* gameBoard);
void createGameBoard(Board* gameBoard);

//global game board
Board gameBoard;

/**limor
 * the main function of the program
 */
int main() {
    //initialize the game board and print it
    createGameBoard(&gameBoard);
    printGameBoard(&gameBoard);
    //define signal handling functions
    signal(SIGALRM, moveShape);
    alarm(1);
    signal(SIGUSR2, operateAccordingToUserInput);
    while (TRUE) {
        //wait until the process gets a signal
        pause();
    }
}


/**limor
 * the function initializes the game board
 * @param gameBoard - the game board
 */
void createGameBoard(Board* gameBoard) {
    int i,j;
    //define the board location
    gameBoard->xCordinate = 0;
    gameBoard->yCordinate = (SIZE_OF_BOARD / 2) - 1;
    //define the board's mode od shapes
    gameBoard->mode = HORIZONTAL;
    //create the board
    for (i = 0; i < SIZE_OF_BOARD - 1; i++) {
        for (j = 0; j < SIZE_OF_BOARD; j++) {
            if ((j != 0) && (j != SIZE_OF_BOARD - 1)) {
                gameBoard->board[i][j] = ' ';
            } else{
                gameBoard->board[i][j] = '*';
            }
            gameBoard->board[SIZE_OF_BOARD - 1][j] = '*';
        }
    }    makeMove(gameBoard, gameBoard->xCordinate, gameBoard->yCordinate, 0);
}

/**limor
 * the function deletes the shape from the board
 * @param gameBoard - the game board
 */
void deleteShapeFromBoard(Board* gameBoard) {
    if (gameBoard->mode == VERTICAL) {
        //in case the shape is in vertical mode
        gameBoard->board[gameBoard->xCordinate + 2][gameBoard->yCordinate] = ' ';
        gameBoard->board[gameBoard->xCordinate + 1][gameBoard->yCordinate] = ' ';
        gameBoard->board[gameBoard->xCordinate][gameBoard->yCordinate] = ' ';
    } else {
        //in case the shape is in horizontal mode
        gameBoard->board[gameBoard->xCordinate][gameBoard->yCordinate + 2] = ' ';
        gameBoard->board[gameBoard->xCordinate][gameBoard->yCordinate + 1] = ' ';
        gameBoard->board[gameBoard->xCordinate][gameBoard->yCordinate] = ' ';
    }
}

/**limor
 * the function makes the shape's moves on the board.
 * @param gameBoard - the game board
 * @param x - the x coordinate of the new shape
 * @param y - the y coordinate of the new shape
 * @param needToTurn - a parameter that represent if we need to turn the shape or not
 */
void makeMove(Board* gameBoard, int x, int y, int needToTurn) {
    //ensure that the shape is moving inside the borders of the board
    if (gameBoard->mode == HORIZONTAL) {
        if ((y <= 0) || (y + 2 >= SIZE_OF_BOARD -1 )){
            return;
        }
    } else if (gameBoard->mode == VERTICAL ) {
        if ((y <= 0) || (y >= SIZE_OF_BOARD - 1)){
            return;
        }
    }
    if (!needToTurn){
        deleteShapeFromBoard(gameBoard);
    }
    //update the position of the shape
    gameBoard->xCordinate = x;
    gameBoard->yCordinate = y;
    // draw the shape in the new position and create the updated board
    if (gameBoard->mode == VERTICAL) {
        gameBoard->board[x + 2][y] = '-';
        gameBoard->board[x + 1][y] = '-';
        gameBoard->board[x][y] = '-';
        //create the updated board
        if (x + 2 >= SIZE_OF_BOARD - 1) {
            createGameBoard(gameBoard);
        }
    } else {
        gameBoard->board[x][y + 2] = '-';
        gameBoard->board[x][y + 1] = '-';
        gameBoard->board[x][y] = '-';
        //create the updated board
        if (x >= SIZE_OF_BOARD - 1) {
            createGameBoard(gameBoard);
        }
    }
}

/**limor
 * the function prints the game board
 * @param gameBoard - the game board
 */
void printGameBoard(Board* gameBoard) {
    int i,j;
    //clear the current board and print the new one
    system("clear");
    for (i = 0; i < SIZE_OF_BOARD; i++) {
        for (j = 0; j < SIZE_OF_BOARD; j++) {
            printf("%c", gameBoard->board[i][j]);
        }
        printf("\n");
    }
}

/**limor
 * the function turn the shape mode from vertical to horizontal or from horizontal to vertical
 * @param gameBoard - the game board
 */
void changeShapeMode(Board* gameBoard) {
    int oldX,oldY,newX,newY;
    //delete the old shape from the bord
    deleteShapeFromBoard(gameBoard);
    //save the old position of the shape
    oldX = gameBoard->xCordinate;
    oldY = gameBoard->yCordinate;
    //change the shape's mode
    if (gameBoard->mode == VERTICAL) {
        gameBoard->mode = HORIZONTAL;
        //calculate the new location of the shape
        newX = oldX + 1;
        newY = oldY - 1;
        //ensure that changing the shape's mode will be inside the borders of the board
        if (newY + 2 >= SIZE_OF_BOARD - 1) {
            newY = SIZE_OF_BOARD - 4;
        } else if (newY < 1) {
            newY = 1;
        }
        makeMove(gameBoard, newX, newY, 1);
    } else  {
        gameBoard->mode = VERTICAL;
        makeMove(gameBoard , oldX - 1, oldY + 1, 1);
    }
}

/**limor
 * the function gets input from the user and do the appropriate move according to it
 */
void operateAccordingToUserInput() {
    char ch;
    //create signal handling function
    signal(SIGUSR2, operateAccordingToUserInput);
    ch = (char)getchar();
    switch (ch) {
        case QUIT: {
            exit(1);
        }
        case RIGHT:{
            makeMove(&gameBoard, gameBoard.xCordinate, gameBoard.yCordinate + 1, 0);
            break;
        }
        case LEFT:{
            makeMove(&gameBoard, gameBoard.xCordinate, gameBoard.yCordinate - 1, 0);
            break;
        }
        case DOWN:
        {
            makeMove(&gameBoard, gameBoard.xCordinate + 1, gameBoard.yCordinate, 0);
            break;
        }
        case TURN:{
            changeShapeMode(&gameBoard);
            break;
        }
        default:{
            break;
        }
    }
    printGameBoard(&gameBoard);
}


/**limor
 * the function moves the shape in the borders of the game board
 */
void moveShape() {
    signal(SIGALRM, moveShape);
    alarm(1);
    //move the shape down in the game board
    makeMove(&gameBoard, gameBoard.xCordinate + 1, gameBoard.yCordinate, 0);
    //print the board after the shape's movement
    printGameBoard(&gameBoard);
}
