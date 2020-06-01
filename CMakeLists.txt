cmake_minimum_required(VERSION 3.15)
project(Gomoku C)

set(CMAKE_C_STANDARD 99)

add_executable(Gomoku src/main.c src/game.h src/game.c src/input.h src/input.c src/rule.h src/rule.c src/stats.h src/stats.c src/utils.h src/utils.c src/md5.c src/md5.h src/gomoko/gomoko.c src/gomoko/gomoko.h src/gomoko/eval.c src/gomoko/eval.h)