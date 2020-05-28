#include <stdio.h>
#include <stdlib.h>
#include "input.h"
#include "game.h"

void printMainMenu() {
    printf(""
           "+----------------------------------+\n"
           "|                                  |\n"
           "|              Gomoku              |\n"
           "|                                  |\n"
           "|                                  |\n"
           "|                                  |\n"
           "|    1. Play a game with Gomoko    |\n"
           "|    2. See statistics             |\n"
           "|                                  |\n"
           "|    0. Quit                       |\n"
           "|                                  |\n"
           "+----------------------------------+\n");
}

int choice() {
    int wrong = 0;
    while (1) {
        // print UI
        system("cls");
        printMainMenu();
        if (wrong) {
            // print hint for previous wrong choice
            printf("Invalid choice. Please reenter your choice.\n");
        }
        // hint for user to input
        printf("(input) ");
        char c;
        // check user input
        if (input_nextChar(&c) == 0) {
            if (c >= '0' && c <= '2') {
                input_endLine();
                return c - '0';
            }
        }
        input_endLine();
        wrong = 1;
    }
}

int main() {
    int ch;
    while ((ch = choice()) != 0) {
        if (ch == 1) {
            // play with Gomoko (AI)
            game_newGame();
        } else if (ch == 2) {
            // see stats
        }
    }
    printf("See you next time.\n");
    return 0;
}