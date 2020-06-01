#include "eval.h"

#include <stdio.h>
#include <string.h>
#include "../utils.h"

typedef enum {
    FIVE = 0,
    OPEN_FOUR, CLOSED_FOUR,
    OPEN_THREE, CLOSED_THREE_NORMAL,
    CLOSED_THREE_LEFT /* (O)_O_O */, CLOSED_THREE_MID /* O_(O)_O */,
    OPEN_TWO, CLOSED_TWO,
    OPEN_ONE, CLOSED_ONE,
    NO_PATTERN
} pattern_kind_t;

typedef struct {
    pattern_kind_t kind;
    int singleSeg;
} pattern_t;

static int dirRow[4] = {0, 1, 1, 1};
static int dirCol[4] = {1, 0, 1, -1};

static int scores[NO_PATTERN + 1] = {500000, 50000, 5000, 5000, 500, 500, 500, 500, 50, 5, 1, 0};

#define row_left(offset) (rowLeft - dr * (offset))
#define col_left(offset) (colLeft - dc * (offset))
#define row_right(offset) (rowRight + dr * (offset))
#define col_right(offset) (colRight + dc * (offset))
#define assert_left_empty(offset) (is_valid_pos(row_left(offset), col_left(offset)) && board[row_left(offset)][col_left(offset)] == GRID_EMPTY)
#define assert_left_chess(offset) (is_valid_pos(row_left(offset), col_left(offset)) && board[row_left(offset)][col_left(offset)] == chess)
#define assert_left_border(offset) (!is_valid_pos(row_left(offset), col_left(offset)) || (board[row_left(offset)][col_left(offset)] != GRID_EMPTY && board[row_left(offset)][col_left(offset)] != chess))
#define assert_right_empty(offset) (is_valid_pos(row_right(offset), col_right(offset)) && board[row_right(offset)][col_right(offset)] == GRID_EMPTY)
#define assert_right_chess(offset) (is_valid_pos(row_right(offset), col_right(offset)) && board[row_right(offset)][col_right(offset)] == chess)
#define assert_right_border(offset) (!is_valid_pos(row_right(offset), col_right(offset)) || (board[row_right(offset)][col_right(offset)] != GRID_EMPTY && board[row_right(offset)][col_right(offset)] != chess))

#define assert_blanks_1(dir) (assert_##dir##_empty(1))
#define assert_blanks_2(dir) (assert_##dir##_empty(1) && assert_##dir##_empty(2))
#define assert_blanks_3(dir) (assert_##dir##_empty(1) && assert_##dir##_empty(2) && assert_##dir##_empty(3))
#define assert_blanks_4(dir) (assert_##dir##_empty(1) && assert_##dir##_empty(2) && assert_##dir##_empty(3) && assert_##dir##_empty(4))
#define assert_left_blanks(count) (assert_blanks_##count(left))
#define assert_left_blanks_border(count) (assert_left_blanks(count) && assert_left_border((count) + 1))
#define assert_right_blanks(count) (assert_blanks_##count(right))
#define assert_right_blanks_border(count) (assert_right_blanks(count) && assert_right_border((count) + 1))

static int isOpenFourWithFour(board_t board, char chess,
                              int rowLeft, int colLeft,
                              int rowRight, int colRight,
                              int dr, int dc) {
    return assert_left_empty(1) && assert_right_empty(1);
}

static int isClosedFourWithOne(board_t board, char chess,
                               int rowLeft, int colLeft,
                               int rowRight, int colRight,
                               int dr, int dc) {
    return assert_right_empty(1) && assert_right_chess(2)
        && assert_right_chess(3) && assert_right_chess(4);
}

static int isClosedFourWithTwo(board_t board, char chess,
                               int rowLeft, int colLeft,
                               int rowRight, int colRight,
                               int dr, int dc) {
    return assert_right_empty(1)
           && assert_right_chess(2) && assert_right_chess(3);
}

static int isClosedFourWithThree(board_t board, char chess,
                                 int rowLeft, int colLeft,
                                 int rowRight, int colRight,
                                 int dr, int dc) {
    return assert_right_empty(1) && assert_right_chess(2);
}

static int isClosedFourWithFour(board_t board, char chess,
                                int rowLeft, int colLeft,
                                int rowRight, int colRight,
                                int dr, int dc) {
    return (assert_left_border(1) && assert_right_empty(1))
    || (assert_left_empty(1) && assert_right_border(1));
}

static int isOpenThreeWithOne(board_t board, char chess,
                              int rowLeft, int colLeft,
                              int rowRight, int colRight,
                              int dr, int dc) {
    return assert_left_empty(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_chess(3)
        && assert_right_empty(4);
}

static int isOpenThreeWithTwo(board_t board, char chess,
                              int rowLeft, int colLeft,
                              int rowRight, int colRight,
                              int dr, int dc) {
    return assert_left_empty(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_empty(3);
}

static int isOpenThreeWithThree(board_t board, char chess,
                                int rowLeft, int colLeft,
                                int rowRight, int colRight,
                                int dr, int dc) {
    return assert_left_empty(1) && assert_right_empty(1)
        && (assert_left_empty(2) || assert_right_empty(2));
}

static int isClosedThreeNormalWithOne(board_t board, char chess,
                                      int rowLeft, int colLeft,
                                      int rowRight, int colRight,
                                      int dr, int dc) {
    return (assert_left_empty(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_chess(3)
        && assert_right_border(4))
    || (assert_right_blanks(2) && assert_right_chess(3)
        && assert_right_chess(4))
    || (assert_left_border(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_chess(3)
        && assert_right_empty(4));
}

static int isClosedThreeLeftWithOne(board_t board, char chess,
                                    int rowLeft, int colLeft,
                                    int rowRight, int colRight,
                                    int dr, int dc) {
    return assert_right_empty(1) && assert_right_chess(2)
        && assert_right_empty(3) && assert_right_chess(4);
}

static int isClosedThreeMidWithOne(board_t board, char chess,
                                   int rowLeft, int colLeft,
                                   int rowRight, int colRight,
                                   int dr, int dc) {
    return assert_left_empty(1) && assert_left_chess(2)
        && assert_right_empty(1) && assert_right_chess(2);
}

static int isClosedThreeWithTwo(board_t board, char chess,
                                int rowLeft, int colLeft,
                                int rowRight, int colRight,
                                int dr, int dc) {
    return (assert_left_border(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_empty(3))
    || (assert_right_blanks(2) && assert_right_chess(3))
    || (assert_left_empty(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_border(3));
}

static int isClosedThreeWithThree(board_t board, char chess,
                                  int rowLeft, int colLeft,
                                  int rowRight, int colRight,
                                  int dr, int dc) {
    return (assert_left_blanks_border(1) && assert_right_blanks_border(1))
    || (assert_left_border(1) && assert_right_blanks(2))
    || (assert_right_border(1) && assert_left_blanks(2));
}

static int isOpenTwoWithOne(board_t board, char chess,
                            int rowLeft, int colLeft,
                            int rowRight, int colRight,
                            int dr, int dc) {
    return (assert_left_empty(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_empty(3)
        && assert_right_empty(4))
    || (assert_left_blanks(2) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_empty(3))
    || (assert_left_empty(1) && assert_right_blanks(2)
        && assert_right_chess(3) && assert_right_empty(4));
}

static int isOpenTwoWithTwo(board_t board, char chess,
                            int rowLeft, int colLeft,
                            int rowRight, int colRight,
                            int dr, int dc) {
    return (assert_left_blanks(1) && assert_right_blanks(3))
    || (assert_left_blanks(2) && assert_right_blanks(2))
    || (assert_left_blanks(3) && assert_right_blanks(1));
}

static int isClosedTwoWithOne(board_t board, char chess,
                              int rowLeft, int colLeft,
                              int rowRight, int colRight,
                              int dr, int dc) {
    return (assert_left_blanks_border(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_empty(3)
        && assert_right_border(4))
    || (assert_left_border(1) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_empty(3)
        && assert_right_empty(4))
    || (assert_left_blanks(2) && assert_right_empty(1)
        && assert_right_chess(2) && assert_right_border(3))
    || (assert_left_border(1) && assert_right_blanks(2)
        && assert_right_chess(3) && assert_right_empty(4))
    || (assert_right_blanks(3) && assert_right_chess(4))
    || (assert_left_empty(1) && assert_right_blanks(2)
        && assert_right_chess(3) && assert_right_border(4));
}

static int isClosedTwoWithTwo(board_t board, char chess,
                              int rowLeft, int colLeft,
                              int rowRight, int colRight,
                              int dr, int dc) {
    return (assert_left_blanks_border(1) && assert_right_blanks_border(2))
    || (assert_left_blanks_border(2) && assert_right_blanks_border(1))
    || (assert_left_border(1) && assert_right_blanks(3))
    || (assert_right_border(1) && assert_left_blanks(3));
}

static int isOpenOneWithOne(board_t board, char chess,
                            int rowLeft, int colLeft,
                            int rowRight, int colRight,
                            int dr, int dc) {
    return (assert_left_blanks(1) && assert_right_blanks(4))
    || (assert_left_blanks(2) && assert_right_blanks(3))
    || (assert_left_blanks(3) && assert_right_blanks(2))
    || (assert_left_blanks(4) && assert_right_blanks(1));
}

static int isClosedOneWithOne(board_t board, char chess,
                              int rowLeft, int colLeft,
                              int rowRight, int colRight,
                              int dr, int dc) {
    return (assert_left_border(1) && assert_right_blanks(4))
    || (assert_right_border(1) && assert_left_blanks(4))
    || (assert_left_blanks_border(1) && assert_right_blanks_border(3))
    || (assert_left_blanks_border(2) && assert_right_blanks_border(2))
    || (assert_left_blanks_border(3) && assert_right_blanks_border(1));
}

static int checkRange(board_t board, char chess, int *rp, int *cp, int dr, int dc) {
    int count = 0;
    int r = *rp, c = *cp;
    while (1) {
        r += dr;
        c += dc;
        if (!is_valid_pos(r, c)) {
            break;
        }
        if (board[r][c] != chess) {
            break;
        }
        count++;
    }
    r -= dr;
    c -= dc;
    *rp = r;
    *cp = c;
    return count;
}

static pattern_t newPattern(pattern_kind_t kind, int singleSeg) {
    pattern_t pat;
    pat.kind = kind;
    pat.singleSeg = singleSeg;
    return pat;
}

#define ckpt(pat, cnt) is##pat##With##cnt(board, chess, rowLeft, colLeft, rowRight, colRight, dr, dc)
static pattern_t checkPatternInDir(board_t board, char chess, int count,
                                   int rowLeft, int colLeft,
                                   int rowRight, int colRight,
                                   int dr, int dc) {
    switch (count) {
        case 1: {
            if ((ckpt(ClosedFour, One))) return newPattern(CLOSED_FOUR, 0);
            if ((ckpt(OpenThree, One))) return newPattern(OPEN_THREE, 0);
            if ((ckpt(ClosedThreeNormal, One))) return newPattern(CLOSED_THREE_NORMAL, 0);
            if ((ckpt(ClosedThreeLeft, One))) return newPattern(CLOSED_THREE_LEFT, 0);
            if ((ckpt(ClosedThreeMid, One))) return newPattern(CLOSED_THREE_MID, 0);
            if ((ckpt(OpenTwo, One))) return newPattern(OPEN_TWO, 0);
            if ((ckpt(ClosedTwo, One))) return newPattern(CLOSED_TWO, 0);
            if ((ckpt(OpenOne, One))) return newPattern(OPEN_ONE, 1);
            if ((ckpt(ClosedOne, One))) return newPattern(CLOSED_ONE, 1);
            return newPattern(NO_PATTERN, 1);
        }
        case 2: {
            if ((ckpt(ClosedFour, Two))) return newPattern(CLOSED_FOUR, 0);
            if ((ckpt(OpenThree, Two))) return newPattern(OPEN_THREE, 0);
            if ((ckpt(ClosedThree, Two))) return newPattern(CLOSED_THREE_NORMAL, 0);
            if ((ckpt(OpenTwo, Two))) return newPattern(OPEN_TWO, 1);
            if ((ckpt(ClosedTwo, Two))) return newPattern(CLOSED_TWO, 1);
            return newPattern(NO_PATTERN, 1);
        }
        case 3: {
            if ((ckpt(ClosedFour, Three))) return newPattern(CLOSED_FOUR, 0);
            if ((ckpt(OpenThree, Three))) return newPattern(OPEN_THREE, 1);
            if ((ckpt(ClosedThree, Three))) return newPattern(CLOSED_THREE_NORMAL, 1);
            return newPattern(NO_PATTERN, 1);
        }
        case 4: {
            if ((ckpt(OpenFour, Four))) return newPattern(OPEN_FOUR, 1);
            if ((ckpt(ClosedFour, Four))) return newPattern(CLOSED_FOUR, 1);
            return newPattern(NO_PATTERN, 1);
        }
    }
    utils_fatalError("FATAL: unexpected execution flow. ENTER to exit...", 3);
}

static pattern_t checkPatternInDirection(board_t board, char chess, int row, int col, int dr, int dc, char *visited) {
    // detect consecutive block
    int count = 1;
    int rowLeft = row, colLeft = col;
    int rowRight = row, colRight = col;
    count += checkRange(board, chess, &rowLeft, &colLeft, -dr, -dc);
    count += checkRange(board, chess, &rowRight, &colRight, dr, dc);
    // update visited flag (if exists)
    if (visited) {
        for (int i = 0, r = rowLeft, c = colLeft; i < count; i++, r += dr, c += dc) {
            visited[r * BOARD_SIZE + c] = 1;
        }
    }
    // check pattern
    if (count >= 5) {
        pattern_t pat;
        pat.kind = FIVE;
        pat.singleSeg = 1;
        return pat;
    }
    return checkPatternInDir(board, chess, count,
            rowLeft, colLeft, rowRight, colRight, dr, dc);
}

static int checkTechnique(board_t board, char chess, int row, int col) {
    int score = 0;
    for (int dir = 0; dir < 4; dir++) {
        int dr = dirRow[dir], dc = dirCol[dir];
        pattern_t pattern = checkPatternInDirection(board, chess, row, col, -dr, -dc, NULL);
        score += scores[pattern.kind];
        pattern = checkPatternInDirection(board, chess, row, col, dr, dc, NULL);
        // special case: symmetric patterns (or single-block patterns)
        // can be recorded twice; should be fixed
        if (pattern.kind != CLOSED_THREE_MID && pattern.singleSeg == 0) {
            score += scores[pattern.kind];
        }
    }
    return score;
}

int evaluateBoard(board_t board, char chess) {
    int score = 0;
    char visited[BOARD_SIZE][BOARD_SIZE];
    char duplicates[BOARD_SIZE][BOARD_SIZE];
    // 4 directions
    for (int dir = 0; dir < 4; dir++) {
        memset(visited, 0, BOARD_SIZE * BOARD_SIZE);
        memset(duplicates, 0, BOARD_SIZE * BOARD_SIZE);
        int dr = dirRow[dir], dc = dirCol[dir];
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == chess && !visited[i][j]) {
                    pattern_t pattern = checkPatternInDirection(board, chess, i, j, dr, dc, (char *) visited);
                    // record duplicates
                    if (pattern.kind == CLOSED_THREE_LEFT) {
                        duplicates[i + dr * 2][j + dc * 2] = 1;
                    }
                    // add to score if not duplicated
                    if (!(duplicates[i][j] && pattern.kind == CLOSED_THREE_MID)) {
                        score += scores[pattern.kind];
                    }
                }
            }
        }
    }
    return score;
}

int eval_evaluateBoard(board_t board) {
    int playerScore = evaluateBoard(board, GRID_PLAYER);
    int gomokoScore = evaluateBoard(board, GRID_GOMOKO);
    //TODO:DEBUG
    /*int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != GRID_EMPTY) count++;
        }
    }
    if (count >= 6) {
        printf("chess count=%d; ",count);
    }
    printf("gomoko %d ; player %d\n", gomokoScore, playerScore);*/
    return gomokoScore - playerScore;
}

int eval_evaluateStep(board_t board, int row, int col, int turn) {
    char chess;
    if (turn == ROLE_PLAYER) {
        chess = GRID_PLAYER;
    } else {
        chess = GRID_GOMOKO;
    }
    // detect special patterns
    return checkTechnique(board, chess, row, col);
}
