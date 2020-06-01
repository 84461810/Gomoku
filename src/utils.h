#ifndef GOMOKU_UTILS_H
#define GOMOKU_UTILS_H

void utils_waitEnter(const char *hint);
void __attribute__((noreturn)) utils_fatalError(const char *hint, int code);

#endif //GOMOKU_UTILS_H
