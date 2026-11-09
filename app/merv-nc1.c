#include <stdio.h>
#include <string.h>
#include <ncurses/ncurses.h>
#include "check.h"

static void draw_program_frame(WINDOW *window) {
    box(window, 0, 0);
    mvwprintw(window, 0, 2, "[ PROGRAM ]");
}

static void draw_start_screen(WINDOW *menu_window) {
    werase(menu_window);
    draw_program_frame(menu_window);
    mvwprintw(menu_window, 2, 2, "FIND RECIPES");
    wattron(menu_window, A_REVERSE);
    mvwprintw(menu_window, 4, 4, "Find recipes");
    wattroff(menu_window, A_REVERSE);
    mvwprintw(menu_window, getmaxy(menu_window) - 2, 2,
              "Enter: Continue  S: Settings");
    wrefresh(menu_window);
}

static void show_create_preferences(WINDOW *preferences_window) {
    int key;

    werase(preferences_window);
    draw_program_frame(preferences_window);
    mvwprintw(preferences_window, 2, 2, "CREATE PREFERENCES");
    mvwprintw(preferences_window, 4, 2,
              "Connect your profile creation function here.");
    mvwprintw(preferences_window, getmaxy(preferences_window) - 2, 2,
              "Esc: Back");

    /* TODO: Call the outside profile creation function on this screen. */
    wrefresh(preferences_window);

    do {
        key = wgetch(preferences_window);
    } while (key != 27);
}

static void show_preferences_message(WINDOW *preferences_window) {
    int key;

    werase(preferences_window);
    draw_program_frame(preferences_window);
    mvwprintw(preferences_window, 2, 2, "PREFERENCES");
    mvwprintw(preferences_window, 4, 2,
              "No profile found. Would you like to create one?");
    mvwprintw(preferences_window, getmaxy(preferences_window) - 2, 2,
              "Enter: Create profile  Esc: Back to Settings");

    /* TODO: Replace this check when a profile database function exists. */
    wrefresh(preferences_window);

    while (1) {
        key = wgetch(preferences_window);
        if (key == '\n' || key == KEY_ENTER || key == '\r') {
            show_create_preferences(preferences_window);
            break;
        }
        if (key == 27) {
            break;
        }
    }

}

/* Returns 1 only when Quit is selected. */
static int show_settings(WINDOW *menu_window) {
    const char *options[] = {"Preferences", "Quit"};
    int selected_option = 0;
    int key;
    int i;
    int height = 12;
    int width = 58;
    WINDOW *settings_window = newwin(height, width,
                                     (LINES - height) / 2,
                                     (COLS - width) / 2);

    if (settings_window == NULL) {
        return 0;
    }

    keypad(settings_window, TRUE);

    while (1) {
        werase(settings_window);
        draw_program_frame(settings_window);
        mvwprintw(settings_window, 2, 2, "SETTINGS");

        for (i = 0; i < 2; i++) {
            if (i == selected_option) {
                wattron(settings_window, A_REVERSE);
            }
            mvwprintw(settings_window, 4 + i, 4, "%s", options[i]);
            if (i == selected_option) {
                wattroff(settings_window, A_REVERSE);
            }
        }

        mvwprintw(settings_window, getmaxy(settings_window) - 2, 2,
                  "Enter: Select  Esc: Back");
        wrefresh(settings_window);
        key = wgetch(settings_window);

        if (key == KEY_UP || key == KEY_DOWN) {
            selected_option = (selected_option + 1) % 2;
        } else if (key == '\n' || key == KEY_ENTER || key == '\r') {
            if (selected_option == 0) {
                show_preferences_message(settings_window);
            } else {
                delwin(settings_window);
                return 1;
            }
        } else if (key == 27) {
            delwin(settings_window);
            touchwin(menu_window);
            return 0;
        }
    }
}

static int enter_ingredients(WINDOW *menu_window,
                             char *ingredient_text,
                             int ingredient_text_size) {
    int key;
    int text_length = 0;
    int input_row = 6;
    int input_column = 15;

    werase(menu_window);
    draw_program_frame(menu_window);
    mvwprintw(menu_window, 2, 2, "FIND RECIPES");
    mvwprintw(menu_window, 4, 2, "Type your ingredients");
    mvwprintw(menu_window, input_row, 2, "Ingredients: ");
    mvwprintw(menu_window, getmaxy(menu_window) - 2, 2,
              "Enter: Search  Esc: Back");
    wmove(menu_window, input_row, input_column);
    wrefresh(menu_window);

    /*
     * Database integration point: suggested ingredients can be loaded from the
     * database and displayed above this input field later.
     */
    ingredient_text[0] = '\0';
    noecho();
    curs_set(1);

    while (1) {
        key = wgetch(menu_window);

        if (key == '\n' || key == KEY_ENTER || key == '\r') {
            curs_set(0);
            return 1;
        }

        if (key == 27) {
            ingredient_text[0] = '\0';
            curs_set(0);
            return 0;
        }

        if (key == KEY_BACKSPACE || key == 127 || key == 8) {
            if (text_length > 0) {
                text_length--;
                ingredient_text[text_length] = '\0';
                mvwaddch(menu_window, input_row,
                         input_column + text_length, ' ');
                wmove(menu_window, input_row,
                      input_column + text_length);
                wrefresh(menu_window);
            }
        } else if (key >= 32 && key <= 126 &&
                   text_length < ingredient_text_size - 1) {
            ingredient_text[text_length] = (char)key;
            text_length++;
            ingredient_text[text_length] = '\0';
            waddch(menu_window, key);
            wrefresh(menu_window);
        }
        /* Special keys, including all arrow keys, are ignored. */
    }

}

int menu1(int color_pair, char *ingredient_text, int ingredient_text_size) {
    int key;
    int menu_height = LINES - 5;
    int menu_width = COLS - 4;
    int menu_y = 2;
    int menu_x = 2;
    WINDOW *menu_window;

    if (ingredient_text == NULL || ingredient_text_size < 2) {
        return 0;
    }

    ingredient_text[0] = '\0';
    menu_window = newwin(menu_height, menu_width, menu_y, menu_x);
    if (menu_window == NULL) {
        return 0;
    }

    wbkgd(menu_window, COLOR_PAIR(color_pair));
    keypad(menu_window, TRUE);

    while (1) {
        draw_start_screen(menu_window);
        key = wgetch(menu_window);

        if (key == '\n' || key == KEY_ENTER || key == '\r') {
            if (enter_ingredients(menu_window, ingredient_text,
                                  ingredient_text_size) == 1) {
                delwin(menu_window);
                return 1;
            }
        } else if (key == 's' || key == 'S') {
            if (show_settings(menu_window) == 1) {
                delwin(menu_window);
                return 0;
            }
        }
        /* Esc deliberately does nothing on this first screen. */
    }
}

/*
 * This screen only prints text supplied by another function. Put the outside
 * search function call in main.c and pass its returned text to this function.
 */
void show_text_screen(int color_pair, const char *title, const char *text) {
    int key;
    WINDOW *text_window = newwin(LINES - 5, COLS - 4, 2, 2);

    if (text_window == NULL) {
        return;
    }

    wbkgd(text_window, COLOR_PAIR(color_pair));
    keypad(text_window, TRUE);
    draw_program_frame(text_window);
    mvwprintw(text_window, 2, 2, "%s", title != NULL ? title : "RESULTS");
    mvwprintw(text_window, 4, 2, "%s", text != NULL ? text : "No results.");
    mvwprintw(text_window, getmaxy(text_window) - 2, 2,
              "Press Enter to continue or Esc to go back.");
    wrefresh(text_window);

    do {
        key = wgetch(text_window);
    } while (key != '\n' && key != KEY_ENTER && key != '\r' && key != 27);

    delwin(text_window);
}
