#include "rule.h"

static int dirRow[4] = {0, 1, 1, 1};
static int dirCol[4] = {1, 0, 1, -1};

static int checkGomokuInDirection(board_t board, char chess, int row, int col, int dir) {
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

static int checkGomoku(board_t board, char chess, int row, int col) {
    for (int i = 0; i < 4; i++) {
        if (checkGomokuInDirection(board, chess, row, col, i)) return 1;
    }
    return 0;
}

int rule_judgeWinner(board_t board, int lastTurn, int lastPlayRow, int lastPlayColumn) {
    // empty board
    if (lastPlayRow < 0) {
        return -1;
    }
    // non-empty board
    char lastPlayChess;
    if (lastTurn == ROLE_PLAYER) {
        lastPlayChess = GRID_PLAYER;
    } else {
        lastPlayChess = GRID_GOMOKO;
    }
    if (checkGomoku(board, lastPlayChess, lastPlayRow, lastPlayColumn)) {
        return lastTurn;
    } else {
        return -1;
    }
}
