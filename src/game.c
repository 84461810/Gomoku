#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "input.h"
#include "rule.h"
#include "stats.h"
#include "utils.h"
#include "gomoko/gomoko.h"

#define is_valid_index(c) ((c) >= 'a' && (c) <= ('a' + BOARD_SIZE - 1))
#define player_name(turn) ((turn == ROLE_PLAYER) ? "Player" : "Gomoko")

static int difficulty = 0;
static board_t board;
static int steps;
static int turn, firstGoer;
static int lastPlayRow = -1, lastPlayColumn = -1;

static void printBoardLine(int row) {
    if (lastPlayRow != row) {
        // normal line
        printf("  %c\t %c", ('a' + row), board[row][0]);
        for (int j = 1; j < BOARD_SIZE; j++) {
            printf("--%c", board[row][j]);
        }
    } else {
        // line where last play lies
        // first column
        if (lastPlayColumn == 0) {
            printf("  %c\t[%c]", ('a' + row), board[row][0]);
        } else {
            printf("  %c\t %c-", ('a' + row), board[row][0]);
        }
        // middle columns
        for (int j = 1; j < BOARD_SIZE - 1; j++) {
            if (lastPlayColumn == j) {
                printf("[%c]", board[row][j]);
            } else {
                printf("-%c-", board[row][j]);
            }
        }
        // last column
        if (lastPlayColumn == BOARD_SIZE - 1) {
            printf("[%c]", board[row][BOARD_SIZE - 1]);
        } else {
            printf("-%c", board[row][BOARD_SIZE - 1]);
        }
    }
    // return
    printf("\n");
}

static void printBoardLineSeparator() {
    printf("\t |");
    for (int j = 1; j < BOARD_SIZE; j++) {
        printf("  |");
    }
    printf("\n");
}

static void printBoardBottom() {
    printf("\n\t a");
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
    memset((void *) board, GRID_EMPTY, BOARD_SIZE * BOARD_SIZE);
}

static void selectDiff() {
    unsigned int diff = 0;
    int invalidInput = 0;
    do {
        system("cls");
        // hint for error
        if (invalidInput) {
            printf("* Invalid input! Please enter an integer within 1~5. *\n\n");
        }
        printf("Select difficulty (1~5): ");
        invalidInput = input_nextUnsignedInt(&diff);
        input_endLine();
    } while (diff < 1 || diff > 5);
    difficulty = (int) diff;
}

static void decideOrder() {
    int playerRoll, gomokoRoll;
    system("cls");
    do {
        // press ENTER
        utils_waitEnter("Press ENTER to roll a dice...");
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
    utils_waitEnter("Press ENTER to continue...");
    // initialize the turn
    turn = firstGoer;
}

static int playAt(char c, int row, int col) {
    if (board[row][col] == GRID_EMPTY) {
        // empty position
        board[row][col] = c;
        lastPlayRow = row;
        lastPlayColumn = col;
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
        if (playAt(GRID_PLAYER, row, col) < 0) {
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
    // AI
    int row, col;
    gomoko_suggest((char *) board, difficulty, &row, &col);
    playAt(GRID_GOMOKO, row, col);
}

static int judgeWinner() {
    return rule_judgeWinner(board, 1 - turn, lastPlayRow, lastPlayColumn);
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
        printf("* Fair. No one has won. *\n\n");
    } else {
        printf("* %s has won the game! *\n\n", player_name(1 - turn));
        if (winner == ROLE_PLAYER) {
            stats_addWin();
        } else {
            stats_addLose();
        }
    }
    utils_waitEnter("Press ENTER to return to menu...");
}
