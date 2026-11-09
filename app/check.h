#ifndef CHECK_H
#define CHECK_H

#define MAX_INGREDIENT_TEXT 1000

/*
 * Shows the main menu.
 * Returns 1 when the user continues to recipe search and 0 when the user exits.
 */
int menu1(int color_pair, char *ingredient_text, int ingredient_text_size);

/* Displays text supplied by an outside function on a simple ncurses screen. */
void show_text_screen(int color_pair, const char *title, const char *text);

#endif
