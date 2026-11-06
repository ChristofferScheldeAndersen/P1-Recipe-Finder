
#include <stdio.h>
#include <ncurses/ncurses.h>

int main(void) {
    // Start ncurses
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

    // Colors
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    bkgd(COLOR_PAIR(1));

    // Window dimensions
    // h = height, w = width
    // y = vertical position, x = horizontal position

    /*
    Maybe this should be the size for history only? Or recipes too
    */
    // Left window
    int menu_h = LINES-4, menu_w = 35, menu_y = 2, menu_x = 2;
    // Upper-right window ((some)20% of available height)
    int up_h = (LINES-5)/5, up_w = COLS - 40, up_y = 2, up_x = 38; // You could also do (LINES-4)*0.2, but integer division truncates the decimal part
    // Lower-right window (remaining height)
    int down_h = (LINES-4)-up_h, down_w = COLS - 40, down_y = up_y+up_h, down_x = 38;

    // Create windows
    // newwin(height, width, y, x)
    WINDOW *menu = newwin(menu_h, menu_w, menu_y, menu_x);
    WINDOW *rightUp = newwin(up_h, up_w, up_y, up_x);
    WINDOW *rightDown = newwin(down_h, down_w, down_y, down_x);

    // Set colors
    wbkgd(menu, COLOR_PAIR(1));
    wbkgd(rightUp, COLOR_PAIR(1));
    wbkgd(rightDown, COLOR_PAIR(1));

    // Allow arrow keys in the menu window
    keypad(menu, TRUE);

    /*
    Maybe profile creation first? Or it checks if you have one and prompts you if you enter preferences/history without one
    
    Searching is in the menu
    View recipes shows the recipe name and a few details in rightUp and the full recipe/description in rightDown
    Preferences allows you to choose between metric and imperial units(?), allergies, and dietary restrictions (vegan, vegetarian, etc.)
    History shows the last 5 recipes you searched for and/or viewed
    Exit exits
    Maybe the windows change depending on what menu item is selected
    */
    const char *menuItems[] = {"Search Ingredients", "View Recipes", "Preferences", "History", "Exit"};

    // Window titles
    const char *rightTitles[][2] = {
            {"| RECIPE DESCRIPTION |", ""},
            {"| RECIPES |", "| SELECTED INGREDIENTS |"},
            {"| PREFERENCES |", ""},
            {"| RECENT RECIPES |", "| HISTORY |"},
            {"| EXIT |", ""}
    };

    const char *rightContents[][2] = {
            {"blablabla", ""},
            {"blebleble", "blublublu"},
            {"ddddd", ""},
            {"fffff", "rrrrrrr"},
            {"Are you sure you want to exit?", ""}
    };

    /*
    index meanings:
    0 = Search Ingredients, second index 0 = rightUp and 1 = rightDown
    1 = View Recipes
    2 = Preferences
    3 = History
    4 = Exit
    */
    const int rightResize[5][4] = {
        {menu_h, up_w, 0, 0},
        {up_h, up_w, down_h, down_w},
        {menu_h, up_w, 0, 0},
        {up_h, up_w, down_h, down_w},
        {menu_h, up_w, 0, 0}
    };

    int selected = 0;

    while (1) {
        erase();
        refresh();
        
        werase(menu);
        werase(rightUp);
        werase(rightDown);

         // Draw borders
        box(menu, 0, 0);
        box(rightUp, 0, 0);
        box(rightDown, 0, 0);

        // Window interaction for loop
        for (int i = 0; i < 5; i++) {
            if (i == selected) {
                wattron(menu, A_REVERSE);
            }
            mvwprintw(menu, i + 2, 3, "%-29s", menuItems[i]);
            if (i == selected) {
                wattroff(menu, A_REVERSE);
            }
        }

        // Right window title loader
        mvwprintw(rightUp, 0, 2, "%s", rightTitles[selected][0]);
        mvwprintw(rightDown, 0, 2, "%s", rightTitles[selected][1]);
        
        // Contents of windows
        mvwprintw(rightUp, 1, 2, "%s", rightContents[selected][0]);
        mvwprintw(rightDown, 1, 2, "%s", rightContents[selected][1]);
        
        //wresize
        wresize(rightUp, rightResize[selected][0], rightResize[selected][1]);

        if (rightResize[selected][2] > 0) {
            wresize(rightDown, rightResize[selected][2], rightResize[selected][3]);
        }
        
        // Display
        wrefresh(menu);
        wrefresh(rightUp);
        wrefresh(rightDown);

        // Wait for a key
        int keyPressed = wgetch(menu);

        if (keyPressed == KEY_UP && selected > 0) {
            selected--;
        } else if (keyPressed == KEY_DOWN && selected < 4) {
            selected++;
        }
    }
    
    // Clean up
    delwin(menu);
    delwin(rightUp);
    delwin(rightDown);
    endwin();

    return 0;
}