#include "rule.h"

#include <string.h>

#define checker_ref(dir) (checkers + (dir) * BOARD_SIZE * BOARD_SIZE)

#define element(ref, row, col) (*((ref) + (row) * BOARD_SIZE + (col)))
#define checker_element(dir, row, col) (element(checker_ref(dir), row, col))
#define horizontal_checker(row, col) checker_element(DIRECTION_HORIZONTAL, row, col)
#define vertical_checker(row, col) checker_element(DIRECTION_VERTICAL, row, col)
#define diagonal_checker(row, col) checker_element(DIRECTION_DIAGONAL, row, col)
#define back_diagonal_checker(row, col) checker_element(DIRECTION_BACK_DIAGONAL, row, col)

char checkers[4][BOARD_SIZE][BOARD_SIZE];

static int findHorizontalGomoku(char *board) {
    memset(checker_ref(DIRECTION_HORIZONTAL), 0, BOARD_SIZE * BOARD_SIZE);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            char ele = element(board, i, j);
            if (ele != '+' && horizontal_checker(i, j) == 0) {
                int k;
                for (k = j + 1; k < BOARD_SIZE; k++) {
                    if (element(board, i, k) != ele) {
                        break;
                    }
                }
                for (int l = j; l < k; l++) {
                    element(board, i, l) = k - j;
                }
            }
        }
    }
}

static int findVerticalGomoku(char *board) {

}

static int findDiagonalGomoku(char *board) {

}

static int findBackDiagonalGomoku(char *board) {

}

int rule_judgeWinner(char *board) {
    int ret;
    ret = findHorizontalGomoku(board);
    if (ret >= 0) return ret;
    ret = findVerticalGomoku(board);
    if (ret >= 0) return ret;
    ret = findDiagonalGomoku(board);
    if (ret >= 0) return ret;
    ret = findBackDiagonalGomoku(board);
    return ret;
}
