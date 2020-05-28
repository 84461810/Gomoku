#include "input.h"

#include <stdio.h>

#define BUFFER_SIZE 256
#define cursor_next(i) (((i) + 1) % BUFFER_SIZE)
#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define digit(c) ((c) - '0')

static char buffer[BUFFER_SIZE];
static int headCursor = 0, tailCursor = 0;
static int lineEnds = 0;

// return how many chars has been newly appended in buffer,
// or -1 if input reaches EOF and no new char is fetched
static int fetchInput() {
    // receive new input with at most (BUFFER_SIZE - 1) chars, which ends with NEWLINE
    int ch = getc(stdin), len = 0;
    while (ch != '\n' && ch != EOF) {
        buffer[tailCursor] = (char) ch;
        tailCursor = cursor_next(tailCursor);
        len++;
        if (len == BUFFER_SIZE - 1) {
            break;
        }
        ch = getc(stdin);
    }
    // whether the current line is completely received
    lineEnds = (ch == '\n' || ch == EOF);

    if (len == 0 && ch == EOF) {
        return -1;
    }
    return len;
}

// get the next char without moving cursor
static int seeNextChar(int inLine) {
    // if there's no more chars in buffer,
    // try to fetch chars from input
    if (headCursor == tailCursor) {
        // under inline mode, do not try fetching from the next line of input
        if (inLine && lineEnds) {
            return -1;
        }
        int ret = fetchInput();
        if (ret == -1) {
            // input has ended
            return -1;
        }
        if (inLine && ret == 0) {
            // nothing new in the current line; current line has ended
            return -1;
        }
    }
    // return the head char in buffer
    unsigned char ch = buffer[headCursor];
    return (int) ((unsigned int) ch);
}

// get the next char
// move the cursor on success
static int popNextChar(int inLine) {
    // look up for the next char
    int ch = seeNextChar(inLine);
    if (ch == -1) {
        return -1;
    }
    // move the cursor
    headCursor = cursor_next(headCursor);
    return ch;
}

static void skipBlank() {
    int ch = seeNextChar(1);
    while (ch == ' ' || ch == '\t') {
        headCursor = cursor_next(headCursor);
        ch = seeNextChar(1);
    }
}

void input_endLine() {
    lineEnds = 0;
    headCursor = tailCursor;
}

int input_nextChar(char *c) {
    int ret = popNextChar(1);
    if (ret < 0) {
        return -1;
    }
    if (c) {
        *c = (char) ret;
    }
    return 0;
}

int input_nextUnsignedInt(unsigned int *i) {
    // skip blank
    skipBlank();

    // try the first char
    int ch = seeNextChar(1);
    if (ch == -1 || !is_digit(ch)) {
        // input ends or is not a number
        return -1;
    }

    // read a number
    unsigned int ret = 0;
    int overflow = 0;
    do {
        // evaluate & check overflow
        if (ret > 429496729) {
            overflow = 1;
        }
        ret *= 10;
        unsigned int tailDigit = digit(ch);
        if (ret > 4294967295 - tailDigit) {
            overflow = 1;
        }
        ret += tailDigit;
        // next char (digit)
        headCursor = cursor_next(headCursor);
        ch = seeNextChar(1);
    } while (ch != -1 && is_digit(ch));

    // return the value
    if (overflow) {
        return -2;
    }
    if (i) {
        *i = ret;
    }
    return 0;
}
