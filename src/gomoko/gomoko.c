#include "gomoko.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../rule.h"
#include "../utils.h"
#include "eval.h"

#define EVAL_NORMAL 0
#define EVAL_CUT 1

typedef struct {
    int row;
    int col;
    int score;
} grid_eval_t;

static board_t board;
static int maxDepth;

static int evaluateBoard() {
    //static int count = 0;
    //static clock_t start = 0, end = 0;
    // TODO: DEBUG
    int ret = eval_evaluateBoard(board);
    /*if (count % 500000 == 0) {
        end = clock();
        printf("time cost of 500000 evaluation: %.3f\n", ((double)(end - start)) / CLOCKS_PER_SEC);
        printf("evaluated %d times\n",count);
        start = clock();
    }
    count++;*/
    return ret;
}

// How much Player [turn] playing at (row,col) values
static int evaluateGrid(int row, int col, int turn) {
    return eval_evaluateStep(board, row, col, turn);
}

// descending sort
static void gridEvalSort(grid_eval_t *grids, int count) {
    if (count <= 1) {
        return;
    }
    // divide
    int leftHalf = count / 2;
    int rightHalf = count - leftHalf;
    // prepare copies
    grid_eval_t *tmpLeft = (grid_eval_t *) malloc(leftHalf * sizeof(grid_eval_t));
    grid_eval_t *tmpRight = (grid_eval_t *) malloc(rightHalf * sizeof(grid_eval_t));
    if (tmpLeft == NULL || tmpRight == NULL) {
        utils_fatalError("FATAL: Out of memory. ENTER to exit...", 2);
    }
    memcpy(tmpLeft, grids, leftHalf * sizeof(grid_eval_t));
    memcpy(tmpRight, grids + leftHalf, rightHalf * sizeof(grid_eval_t));
    // recursion
    gridEvalSort(tmpLeft, leftHalf);
    gridEvalSort(tmpRight, rightHalf);
    // merge results
    int l = 0, r = 0;
    for (int i = 0; i < count; i++) {
        if (l == leftHalf) {
            grids[i] = tmpRight[r];
            r++;
        } else if (r == rightHalf) {
            grids[i] = tmpLeft[l];
            l++;
        } else {
            if (tmpLeft[l].score > tmpRight[r].score) {
                grids[i] = tmpLeft[l];
                l++;
            } else {
                grids[i] = tmpRight[r];
                r++;
            }
        }
    }
    // return
    free(tmpLeft);
    free(tmpRight);
}

static int heuristicSort(int **rows, int **cols, int turn) {
    static grid_eval_t grids[BOARD_SIZE * BOARD_SIZE];
    int count = 0;
    char chess;
    if (turn == ROLE_PLAYER) {
        chess = GRID_PLAYER;
    } else {
        chess = GRID_GOMOKO;
    }
    // evaluate each empty (available) grid
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == GRID_EMPTY) {
                // try this step
                board[i][j] = chess;
                grids[count].row = i;
                grids[count].col = j;
                grids[count].score = evaluateGrid(i, j, turn);
                // restore
                board[i][j] = GRID_EMPTY;
                count++;
            }
        }
    }
    // allocate arrays
    *rows = (int *) malloc(count * sizeof(int));
    *cols = (int *) malloc(count * sizeof(int));
    if (*rows == NULL || *cols == NULL) {
        utils_fatalError("FATAL: Out of memory. ENTER to exit...", 2);
    }
    // sort & get output
    gridEvalSort(grids, count);
    for (int i = 0; i < count; i++) {
        //if (turn==ROLE_PLAYER && count>=223)
        //    printf("%d:step score:%d at row %d,col %d\n",i,grids[i].score,grids[i].row,grids[i].col);
        (*rows)[i] = grids[i].row;
        (*cols)[i] = grids[i].col;
    }
    return count;
}

static int playerPrefers(int val, int refVal) {
    return val < refVal;
}

static int gomokoPrefers(int val, int refVal) {
    return val > refVal;
}

static int findBestChoice(int depth, int turn, int refBestScore,
        int lastPlayRow, int lastPlayCol,
        int *score, int *steps, int *row, int *col) {
    // TODO: DEBUG
    //if (depth <= 2)
    //    printf("%d\n", depth);
    if (depth == maxDepth || rule_judgeWinner(board, 1 - turn, lastPlayRow, lastPlayCol) >= 0) {
        // has reached the max depth of search
        // or one has won
        *score = evaluateBoard();
        *steps = 0;
        return EVAL_NORMAL;
    }

    // heuristic sort
    int *rows, *cols;
    int emptyGrids = heuristicSort(&rows, &cols, turn);

    // prepare for traversing
    int (*prefers)(int, int);
    char chess;
    int bestScore, maxStep = 0;
    if (turn == ROLE_PLAYER) {
        prefers = playerPrefers;
        chess = GRID_PLAYER;
        bestScore = INT_MAX;
    } else {
        prefers = gomokoPrefers;
        chess = GRID_GOMOKO;
        bestScore = INT_MIN;
    }
    int *bestRows = NULL, *bestCols = NULL, *bestSteps = NULL, sc, stp;
    int bestCursor = 0;
    if (depth == 0) {
        // there may be multiple best solution at the first step
        bestRows = (int *) malloc(emptyGrids * sizeof(int));
        bestCols = (int *) malloc(emptyGrids * sizeof(int));
        bestSteps = (int *) malloc(emptyGrids * sizeof(int));
        if (bestRows == NULL || bestCols == NULL || bestSteps == NULL) {
            utils_fatalError("FATAL: Out of memory. ENTER to exit...", 2);
        }
    }

    // traverse & cut
    // TODO: DEBUG
    int nocutCount = 0;
    for (int i = 0; i < emptyGrids; i++) {
        // try playing a step on board
        int r = rows[i], c = cols[i];
        // TODO: DEBUG
        /*if (board[r][c] != GRID_EMPTY) {
            utils_fatalError("FATAL: Played at non-empty grids..", 4);
        }*/
        board[r][c] = chess;
        // evaluate
        int ret = findBestChoice(depth + 1, 1 - turn, bestScore, r, c,
                &sc, &stp, NULL, NULL);
        if (ret == EVAL_NORMAL) {
            nocutCount++;
            if (prefers(sc, refBestScore)) {
                // the board benefits me more
                // the competitor may want to cut my branch
                // TODO: DEBUG
                //if (depth == 3)
                    //printf("depth %d: cut at %d, sc=%d, ref=%d\n", depth, i, sc, refBestScore);
                // recover the board
                board[r][c] = GRID_EMPTY;
                return EVAL_CUT;
            }
            // update the best score
            // and the best vector (if exists)
            // TODO:debug
            //if (depth % 2 == 1)
                //printf("depth %d: sub-branch score %d, best %d; ref %d\n",depth,sc,bestScore,refBestScore);
            if (prefers(sc, bestScore)) {
                bestScore = sc;
                // reset the best vector
                bestCursor = 0;
                maxStep = stp + 1;
            }
            // collect info about best choices
            if (sc == bestScore) {
                if (depth == 0) {
                    bestRows[bestCursor] = r;
                    bestCols[bestCursor] = c;
                    bestSteps[bestCursor] = stp;
                    bestCursor++;
                }
                // update the number of steps
                if (maxStep < stp + 1) {
                    maxStep = stp + 1;
                }
            }
        }
        // recover the board
        board[r][c] = GRID_EMPTY;
    }

    // TODO: DEBUG
    //if (depth <= 5)
    //    printf("depth %d: searched %d uncut branches\n", depth, nocutCount);

    // compare multiple best choices at the first step
    // choose the solution with minimal steps
    if (depth == 0) {
        int minStep = INT_MAX, bestRow = 0, bestCol = 0;
        for (int i = 0; i < bestCursor; i++) {
            int r = bestRows[i], c = bestCols[i], stps = bestSteps[i];
            if (stps < minStep) {
                minStep = stps;
                bestRow = r;
                bestCol = c;
            }
        }
        *row = bestRow;
        *col = bestCol;
    }

    // return
    *score = bestScore;
    *steps = maxStep;
    free(rows);
    free(cols);
    if (bestRows) {
        free(bestRows);
    }
    if (bestCols) {
        free(bestCols);
    }
    if (bestSteps) {
        free(bestSteps);
    }
    return EVAL_NORMAL;
}

void gomoko_suggest(char *boardArray, int maxRounds, int *row, int *col) {
    maxDepth = maxRounds * 2;
    memcpy(board, boardArray, BOARD_SIZE * BOARD_SIZE);
    int score, steps;
    findBestChoice(0, ROLE_GOMOKO, INT_MAX, -1, -1,
            &score, &steps, row, col);
}
