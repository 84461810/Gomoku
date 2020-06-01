#ifndef GOMOKU_RULE_H
#define GOMOKU_RULE_H

#define BOARD_SIZE 15
#define ROLE_PLAYER 0
#define ROLE_GOMOKO 1

#define GRID_EMPTY '+'
#define GRID_PLAYER 'O'
#define GRID_GOMOKO 'X'

#define DIRECTION_HORIZONTAL 0
#define DIRECTION_VERTICAL 1
#define DIRECTION_DIAGONAL 2
#define DIRECTION_BACK_DIAGONAL 3

#define is_valid_pos(row, col) (row >= 0 && row <= BOARD_SIZE && col >= 0 && col <= BOARD_SIZE)

typedef char board_t[BOARD_SIZE][BOARD_SIZE];

int rule_judgeWinner(board_t board, int lastTurn, int lastPlayRow, int lastPlayColumn);

#endif //GOMOKU_RULE_H
