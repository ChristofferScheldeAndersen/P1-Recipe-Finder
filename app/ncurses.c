
#include <stdio.h>
#include <ncurses/ncurses.h>

int main(void) {
    // 1. Start ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // Check terminal size
    if (LINES < 18 || COLS < 50) {
        endwin();
        puts("Terminal must be at least 50x18.");
        return 1;
    }

    // 2. Colors
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    bkgd(COLOR_PAIR(1));

    // 3. WINDOW DIMENSIONS
    // h = height, w = width
    // y = vertical position, x = horizontal position

    // Left window
    int left_h = LINES-4, left_w = 35, left_y = 2, left_x = 2;
    // Upper-right window ((some)20% of available height)
    int up_h = (LINES-5)/5, up_w = COLS - 40, up_y = 2, up_x = 38; // You could also do (LINES-4)*0.2, but integer division truncates the decimal part
    // Lower-right window (remaining height)
    int down_h = (LINES-4)-up_h, down_w = COLS - 40, down_y = up_y+up_h, down_x = 38;

    // 4. Create windows
    // newwin(height, width, y, x)

    WINDOW *left = newwin(left_h, left_w, left_y, left_x);
    WINDOW *rightUp = newwin(up_h, up_w, up_y, up_x);
    WINDOW *rightDown = newwin(down_h, down_w, down_y, down_x);

    // 5. Set colors
    wbkgd(left, COLOR_PAIR(1));
    wbkgd(rightUp, COLOR_PAIR(1));
    wbkgd(rightDown, COLOR_PAIR(1));

    // 6. Draw borders
    box(left, 0, 0);
    box(rightUp, 0, 0);
    box(rightDown, 0, 0);

    // 7. Display
    refresh();
    wrefresh(left);
    wrefresh(rightUp);
    wrefresh(rightDown);

    // 8. Wait for a key
    getch();

    // 9. Clean up
    delwin(left);
    delwin(rightUp);
    delwin(rightDown);
    endwin();

    return 0;
}
