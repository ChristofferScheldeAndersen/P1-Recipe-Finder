
/* Header file for both backend (ingredients) and frontend (ncurses) */

#ifndef UI_H
#define UI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ncurses/ncurses.h>

// Struct for storing a list of strings
typedef struct {
    char **items;
    int count;
} StringList;

/* Backend functions */
void string_add(StringList *list, const char *text);
void string_free(StringList list);
int small_letters(const char *s, const char *t);
StringList search_ingredients(const char *lookup_string);

/* Frontend functions */
void input_ingredients(WINDOW *action);

#endif
