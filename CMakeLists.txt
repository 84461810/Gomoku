cmake_minimum_required(VERSION 3.15)
project(Gomoku C)

set(CMAKE_C_STANDARD 99)

add_executable(Gomoku src/main.c src/game.h src/game.c src/input.h src/input.c src/rule.h src/rule.c)