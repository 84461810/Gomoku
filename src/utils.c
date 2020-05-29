#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include "input.h"

void utils_waitEnter(const char *hint) {
    if (hint) {
        printf(hint);
    }
    input_nextChar(NULL);
    input_endLine();
}

void utils_fatalError(const char *hint, int code) {
    utils_waitEnter(hint);
    exit(code);
}
