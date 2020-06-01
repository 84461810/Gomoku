#ifndef GOMOKU_EVAL_H
#define GOMOKU_EVAL_H

#include "../rule.h"

int eval_evaluateBoard(board_t board);
int eval_evaluateStep(board_t board, int row, int col, int turn);

#endif //GOMOKU_EVAL_H
