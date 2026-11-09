
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
    int menu_h = LINES-5, menu_w = 35, menu_y = 2, menu_x = 2;
    // Upper-right window ((some)20% of available height)
    int up_h = 5, up_w = COLS - 40, up_y = 2, up_x = 38; // You could also do (LINES-4)*0.2, but integer division truncates the decimal part
    // Lower-right window (remaining height)
    int down_h = (LINES-5)-up_h, down_w = COLS - 40, down_y = up_y+up_h, down_x = 38;
    
    //int rightDownVisible = 0; // Decides whether the rightDown window is visible, disabled by default
   

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
    keypad(rightUp, TRUE);

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
    The 0's don't hide the windows, they only tell us to hide them, like by checking if rightResize[0][2] is 0 in an if-statement
    */
    const int rightResize[][4] = {
        {menu_h, up_w, 0, 0}, 
        {up_h, up_w, down_h, down_w},
        {menu_h, up_w, 0, 0},
        {up_h, up_w, down_h, down_w},
        {menu_h, up_w, 0, 0}
    };

    int fullscreen_w = COLS-4; // Fullscreen window (for selected menu items)
    int selected = 0; // The section you're hovering over in the menu
    int whileLoopRunning = 1; // Enables or disables the while loop, 1 = enabled 0 = disabled
    int inSelectedView = 0; // The section after selecting it
    int rightDownVisible;
    
    /* WHILE LOOP */
    while (whileLoopRunning) {
        rightDownVisible = (rightResize[selected][2] != 0);

        // Constantly visible info
        mvprintw(LINES-3, 2, "Use arrow keys to navigate");
        mvprintw(LINES-2, 2, "Right arrow to select, left arrow to go back");
        mvprintw(0, 2, "USER PROFILE: NONE");
        refresh();
        
        // wresize, resizes the windows rightUp and rightDown based on the selected menu item. The rightResize array contains the new dimensions for each window based on the selected menu item.
        if (inSelectedView) {
            wresize(rightUp, rightResize[selected][0], fullscreen_w);
            wresize(rightDown, rightResize[selected][2], fullscreen_w);
            mvwin(rightUp, up_y, menu_x);
            mvwin(rightDown, down_y, menu_x);
        } else {
            wresize(rightUp, rightResize[selected][0], rightResize[selected][1]);
            if (rightDownVisible) wresize(rightDown, rightResize[selected][2], rightResize[selected][3]);
            mvwin(rightUp, up_y, up_x);
            mvwin(rightDown, down_y, down_x);
        }

        // what?
        werase(menu);
        werase(rightDown);
        werase(rightUp);
        
        // Draw borders
        if (!inSelectedView) box(menu, 0, 0);
        if (rightDownVisible) box(rightDown, 0, 0);
        box(rightUp, 0, 0);
        

        // Window interaction for loop
        if (!inSelectedView) {
            for (int i = 0; i < 5; i++) {
                if (i == selected) {
                    wattron(menu, A_REVERSE);
                }
                mvwprintw(menu, i + 2, 3, "%-*s", menu_w - 6, menuItems[i]); // Uses a pointer to calculate the width of menu minus the spaces on either side (2+2) and minus the borders (1+1)
                if (i == selected) {
                    wattroff(menu, A_REVERSE);
                }
            }
        }

        /* TEXT LOADING */
        // Menu window title loader
        if (!inSelectedView) mvwprintw(menu, 0, 2, "[ SELECTION MENU ]");
        // Both right windows' title loader
        mvwprintw(rightUp, 0, 2, "%s", rightTitles[selected][0]);
        if (rightDownVisible) mvwprintw(rightDown, 0, 2, "%s", rightTitles[selected][1]);
        // Contents (description) of windows
        mvwprintw(rightUp, 2, 3, "%s", rightContents[selected][0]);
        if (rightDownVisible) mvwprintw(rightDown, 2, 3, "%s", rightContents[selected][1]);
        
        // Display
        wrefresh(menu);
        wrefresh(rightDown);
        wrefresh(rightUp);

        // Wait for a key
        int menuKeyPressed = wgetch(menu);

        if (!inSelectedView) {
            if (menuKeyPressed == KEY_UP && selected > 0) {
                selected--;
            } else if (menuKeyPressed == KEY_DOWN && selected < 4) {
                selected++;
            } else if (menuKeyPressed == '\n' || menuKeyPressed == KEY_RIGHT) {
                inSelectedView = 1;
            }
        } else {
            if (menuKeyPressed == KEY_LEFT) {
                werase(rightUp);
                werase(rightDown);
                wrefresh(rightUp);
                wrefresh(rightDown);
                inSelectedView = 0;
            } else if (selected == 4 && menuKeyPressed == KEY_RIGHT) {
                whileLoopRunning = 0;
            }
        }
    }

    // Clean up
    delwin(menu);
    delwin(rightUp);
    delwin(rightDown);
    endwin();

    return 0;
}