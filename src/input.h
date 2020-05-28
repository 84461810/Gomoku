#ifndef GOMOKU_INPUT_H
#define GOMOKU_INPUT_H

/**
 * Skip the rest of the current line. Start a new input line.
 */
void input_endLine();

/**
 * Get the next character in the current line.
 * @param c the pointer of output character, or NULL if you want to skip the character
 * @return 0 on success, or -1 if the current line has reached its end.
 */
int input_nextChar(char *c);

/**
 * Skip consecutive blank characters (i.e. space and tab),
 * and then try to get the next integer within the current line.
 * It will try to get the longest substring which represents a valid unsigned integer.
 * <br/>
 * A valid unsigned integer should be within the range of [0, 4294967295].
 *
 * @param i the pointer of output integer, or NULL if you want to skip the integer
 * @return 0 on success;
 * -1 if the first character does not indicate an unsigned integer,
 * or the current line has reached its end;
 * -2 if the integer is out of range.
 */
int input_nextUnsignedInt(unsigned int *i);

#endif //GOMOKU_INPUT_H
