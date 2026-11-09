#include <stdio.h>
#include <ncurses/ncurses.h>
#include "check.h"

int main(int argc, char **argv) {
    char ingredient_text[MAX_INGREDIENT_TEXT];
    int menu_result;

    (void)argc;
    (void)argv;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
    }

    menu_result = menu1(1, ingredient_text, MAX_INGREDIENT_TEXT);

    if (menu_result == 1) {
        /*
         * INSERT YOUR OUTSIDE FUNCTION HERE.
         *
         * If your function returns text, the connection can look like this:
         *
         * const char *result_text = your_search_function(ingredient_text);
         * show_text_screen(1, "SEARCH RESULTS", result_text);
         *
         * Adjust this when you know the real function name and return type.
         */
        show_text_screen(1, "SEARCH RESULTS",
                         "Connect your search function in main.c.");
    }

    if (menu_result == 0) {
        endwin();
        printf("Recipe Finder closed.\n");
        return 0;
    }

    endwin();
    return 0;
}
