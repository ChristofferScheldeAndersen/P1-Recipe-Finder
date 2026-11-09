#include <stdio.h>
#include <ncurses/ncurses.h>

void main() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // terminal size
    if (LINES < 16 || COLS < 50) {
        endwin();
        printf("Terminal needs to be at least 50x16");
        return 0;
    }

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
}