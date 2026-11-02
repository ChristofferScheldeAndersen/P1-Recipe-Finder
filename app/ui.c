#include <stdio.h>
#include <ncurses/ncurses.h>

int main(void) {
    // 1. START NCURSES
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    if (LINES < 16 || COLS < 65) {
        endwin();
        puts("Please make the terminal at least 65x16.");
        return 0;
    }

    // White text, black background
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        bkgd(COLOR_PAIR(1));
    }

    // 2. CREATE TWO WINDOWS
    // newwin(height, width, y, x)
    // y and x determine the top-left corner.

    WINDOW *menu = newwin(LINES - 4, 22, 2, 2);
    WINDOW *info = newwin(LINES - 4, COLS - 27, 2, 25);

    if (has_colors()) {
        wbkgd(menu, COLOR_PAIR(1));
        wbkgd(info, COLOR_PAIR(1));
    }

    // Allow arrow keys in the menu window
    keypad(menu, TRUE);

    // 3. CONTENT FOR OUR INTERACTIVE TUTORIAL
    const char *names[] = {
        "newwin()", "box()", "mvwprintw()",
        "werase()", "wrefresh()", "wgetch()", "delwin()"
    };

    const char *help[][2] = {
        {"Creates a rectangular window.", "Arguments: height, width, y, x."},
        {"Draws a border around a window.", "We use it on both windows."},
        {"Prints text inside a window.", "Coordinates are window-relative."},
        {"Clears a window's contents.", "We redraw the border afterward."},
        {"Displays a window's changes.", "Each window refreshes separately."},
        {"Reads a key from a window.", "Arrow keys need keypad()."},
        {"Deletes a window when finished.", "We call it before endwin()."}
    };

    int selected = 0;

    // 4. MAIN LOOP
    while (1) {
        // Draw the background screen
        erase();
        mvprintw(0, 2, "NCURSES WINDOW TUTORIAL");
        mvprintw(LINES - 1, 2, "UP/DOWN: Explore | Q: Quit");
        refresh();

        // Clear the two windows
        werase(menu);
        werase(info);

        // Draw their borders
        box(menu, 0, 0);
        box(info, 0, 0);

        // Add titles
        mvwprintw(menu, 0, 2, " FUNCTIONS ");
        mvwprintw(info, 0, 2, " EXPLANATION ");

        // Display all menu items
        for (int i = 0; i < 7; i++) {
            if (i == selected)
                wattron(menu, A_REVERSE);

            mvwprintw(menu, i + 2, 2, "%-16s", names[i]);

            if (i == selected)
                wattroff(menu, A_REVERSE);
        }

        // Display help for the selected item
        mvwprintw(info, 2, 2, "%s", names[selected]);
        mvwprintw(info, 4, 2, "%s", help[selected][0]);
        mvwprintw(info, 5, 2, "%s", help[selected][1]);

        mvwprintw(info, 8, 2, "This is a separate window!");

        // Make both windows visible
        wrefresh(menu);
        wrefresh(info);

        // 5. KEYBOARD INPUT
        int key = wgetch(menu);

        if (key == 'q' || key == 'Q')
            break;

        if (key == KEY_UP && selected > 0)
            selected--;

        if (key == KEY_DOWN && selected < 6)
            selected++;
    }

    // 6. CLEAN UP
    delwin(menu);
    delwin(info);
    endwin();

    return 0;
}