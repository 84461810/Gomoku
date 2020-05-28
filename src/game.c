#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include "input.h"
#include "rule.h"

#define is_valid_index(c) ((c) >= 'a' && (c) <= ('a' + BOARD_SIZE - 1))
#define player_name(turn) ((turn == ROLE_PLAYER) ? "Player" : "Gomoko")
#define is_valid_pos(row, col) (row >= 0 && row <= BOARD_SIZE && col >= 0 && col <= BOARD_SIZE)

static int dirRow[4] = {0, 1, 1, 1};
static int dirCol[4] = {1, 0, 1, -1};

static int difficulty = 0;
static char board[BOARD_SIZE][BOARD_SIZE];
static int steps;
static int turn, firstGoer;
static int lastPlayRow = -1, lastPlayColumn = -1;
static char lastPlayChess;

static void printBoardLine(int row) {
    printf("  %c\t%c", ('a' + row), board[row][0]);
    for (int j = 1; j < BOARD_SIZE; j++) {
        printf("--%c", board[row][j]);
    }
    printf("\n");
}

static void printBoardLineSeparator() {
    printf("\t|");
    for (int j = 1; j < BOARD_SIZE; j++) {
        printf("  |");
    }
    printf("\n");
}

static void printBoardBottom() {
    printf("\n\ta");
    for (int j = 1; j < BOARD_SIZE; j++) {
        printf("  %c", 'a' + j);
    }
    printf("\n\n");
}

static void printBoard() {
    printf("\n  %s goes first in this game.\n\n", player_name(firstGoer));
    printBoardLine(0);
    for (int i = 1; i < BOARD_SIZE; i++) {
        printBoardLineSeparator();
        printBoardLine(i);
    }
    printBoardBottom();
}

static void clearBoard() {
    memset((void *) board, '+', BOARD_SIZE * BOARD_SIZE);
}

static void selectDiff() {
    // TODO: select difficulty
    difficulty = 0;
}

static void decideOrder() {
    int playerRoll, gomokoRoll;
    system("cls");
    do {
        // press ENTER
        printf("Press ENTER to roll a dice...");
        input_nextChar(NULL);
        input_endLine();
        // both player and Gomoko roll
        playerRoll = rand() % 6 + 1;
        gomokoRoll = rand() % 6 + 1;
        printf("Player rolls %d. Gomoko rolls %d.\n", playerRoll, gomokoRoll);
        if (playerRoll == gomokoRoll) {
            printf("Fair.\n\n");
        } else if (playerRoll > gomokoRoll) {
            printf("\n* Player goes first. *\n\n");
            firstGoer = ROLE_PLAYER;
        } else {
            printf("\n* Gomoko goes first. *\n\n");
            firstGoer = ROLE_GOMOKO;
        }
    } while (playerRoll == gomokoRoll);
    // end rolling
    printf("Press ENTER to continue...");
    input_nextChar(NULL);
    input_endLine();
    // initialize the turn
    turn = firstGoer;
}

static int playAt(char c, int row, int col) {
    if (board[row][col] == '+') {
        // empty position
        board[row][col] = c;
        lastPlayRow = row;
        lastPlayColumn = col;
        lastPlayChess = c;
        steps++;
        return 0;
    }
    return -1;
}

static void playerPlay() {
    int invalidInput = 0, ruleDenied = 0;
    int row, col;
    while (1) {
        // print the board
        system("cls");
        printBoard();
        // error message
        if (invalidInput) {
            printf("* Invalid input! Please input your choice as following. *\n\n");
            invalidInput = 0;
        } else if (ruleDenied) {
            printf("* Your previous choice is not allowed according to the rule. *\n\n");
            ruleDenied = 0;
        }
        // hint for input
        printf("Player, now it is your turn. Input your choice.\n"
               "For example, the choice \"ab\" means you play at row 'a', column 'b'.\n"
               "(input) ");
        // check input
        char rowCh, colCh;
        if (input_nextChar(&rowCh) < 0 || input_nextChar(&colCh) < 0) {
            invalidInput = 1;
            input_endLine();
            continue;
        }
        input_endLine();
        if (!is_valid_index(rowCh) || !is_valid_index(colCh)) {
            invalidInput = 1;
            continue;
        }
        // try to play at the specified position
        row = rowCh - 'a';
        col = colCh - 'a';
        if (playAt('O', row, col) < 0) {
            ruleDenied = 1;
            continue;
        }
        break;
    }
}

static void gomokoPlay() {
    // print the board
    system("cls");
    printBoard();
    printf("Gomoko thinking...");
    // TODO: GOMOKO AI
    Sleep(500);
    int ret;
    do {
        int x = rand() % BOARD_SIZE;
        int y = rand() % BOARD_SIZE;
        ret = playAt('X', x, y);
    } while (ret < 0);
}

static int checkGomokuInDirection(char chess, int row, int col, int dir) {
    int i, moku = 1;
    // to negative direction
    int r = row, c = col, dr = dirRow[dir], dc = dirCol[dir];
    for (i = 1; i < 5; i++) {
        r -= dr;
        c -= dc;
        if (!is_valid_pos(r, c)) {
            break;
        }
        if (board[r][c] != chess) {
            break;
        }
    }
    moku += i - 1;
    // to positive direction
    r = row;
    c = col;
    for (i = 1; i < 5; i++) {
        r += dr;
        c += dc;
        if (!is_valid_pos(r, c)) {
            break;
        }
        if (board[r][c] != chess) {
            break;
        }
    }
    moku += i - 1;
    // result
    return (moku >= 5);
}

static int checkGomoku(char chess, int row, int col) {
    for (int i = 0; i < 4; i++) {
        if (checkGomokuInDirection(chess, row, col, i)) return 1;
    }
    return 0;
}

static int judgeWinner() {
    if (lastPlayRow < 0) {
        return -1;
    }
    if (checkGomoku(lastPlayChess, lastPlayRow, lastPlayColumn)) {
        return (1 - turn);
    } else {
        return -1;
    }
}

static int boardIsFull() {
    return (steps >= BOARD_SIZE * BOARD_SIZE);
}

void game_newGame() {
    // initialize
    srand(time(NULL));
    clearBoard();
    difficulty = 0;
    steps = 0;
    // select difficulty
    selectDiff();
    decideOrder();
    // game
    int winner;
    while ((winner = judgeWinner()) < 0 && !boardIsFull()) {
        // the one in turn plays
        if (turn == ROLE_PLAYER) {
            playerPlay();
        } else {
            gomokoPlay();
        }
        // take turn
        turn = 1 - turn;
    }
    // the game ends
    system("cls");
    printBoard();
    if (winner < 0) {
        printf("* Fair. No one has won. *");
    }
    printf("* %s has won the game! *", player_name(1 - turn));
    input_nextChar(NULL);
    input_endLine();
}
