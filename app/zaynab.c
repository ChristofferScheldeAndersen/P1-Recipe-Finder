#include <stdio.h>
#include <ncurses/ncurses.h>

// struct for showing e.g. a "(1)" to the right of currently selected menu_items item
struct Display {
    char user_name[30];
    int ingredients_typed;
    int preferences;
    char erase;

    
};

int main(void) {
    // start ncurses


    initscr();
    cbreak();
    noecho();
    curs_set(0);

    // check terminal size
    if (LINES < 18 || COLS < 50) {
        endwin();
        puts("Terminal must be at least 50x18.");
        return 1;
    }

struct Display profile = {0}; 

profile.ingredients_typed = 4;
    // colors
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK); 
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_BLACK, COLOR_BLACK);
    bkgd(COLOR_PAIR(1));

 
 box(stdscr, 0, 0); 
  int tast;
    for (int i = 0; i < 15; i++)

    {     
    clear();
    mvprintw(i, 35, "Welcome to our meal planning program");
    refresh();
    napms(100);

    }
    attron(COLOR_PAIR(3));
    napms(500);
    mvprintw(10, 40, "Press SPACE to show menu");
    attroff(COLOR_PAIR(3));
    refresh(); 
   
    do 
    {
        tast = getch(); 
    }
    while (tast != ' '); 

     werase(stdscr); 
     wrefresh(stdscr); 
   

    // menu window
    int menu_h = 10, menu_w = 35, menu_y = 2, menu_x = 2;
    // information window
    int info_h = 10, info_w = COLS - 40, info_y = 2, info_x = 38; // You could also do (LINES-4)*0.2, but integer division truncates the decimal part
    // interaction window
    int action_h = (LINES-5)-info_h, action_w = COLS - 2, action_y = info_y+info_h, action_x = 2;

    // create windows
    // newwin(height, width, y, x)
    WINDOW *menu = newwin(menu_h, menu_w, menu_y, menu_x);
    WINDOW *info = newwin(info_h, info_w, info_y, info_x);
    WINDOW *action = newwin(action_h, action_w, action_y, action_x);

    // set the colors in each of our windows
    wbkgd(menu, COLOR_PAIR(1));
    wbkgd(info, COLOR_PAIR(1));
    wbkgd(action, COLOR_PAIR(1));

    // allows arrow keys in the menu window
    keypad(menu, TRUE);
    keypad(info, TRUE);

    // menu selection items
    const char *menu_items[] = {
        "User Profile", "Type in Ingredients", "Preferences", "Calculate Best Recipes", "Exit"
        
    };

    // interaction window titles, only one that needs to update
    const char *action_titles[] = {
        "[ USER PROFILES ]", "[ INGREDIENTS SEARCH ]", "[ PREFERENCES ]", "[ 3 BEST RECIPES ]", "[ EXIT ]"
    };

    const char *info_action_contents[][2] = {
            {"Your user profile allows you to...", ""},
            {"Here you can type in your ingredients and...", "Input Ingredients: "},
            {"Here are your preferences...", "Toggled: "},
            {"Using your ingredients, we can calculate the best recipes...", "Three best recipes: "},
            {"Are you sure you want to exit?", ""}
    };
    

    /*
     *  index meanings:
     *  0 = User Profile, second index 0 = info and 1 = action
     *  1 = Type in Ingredients
     *  2 = Preferences
     *  3 = Calculate Best Recipes
     *  4 = Exit
     */

    int selected = 0; // the section you're hovering over in the menu
    int while_loop_running = 1; // enables or disables the while loop, 1 = enabled 0 = disabled, which ends the program
    
    /* WHILE LOOP */
    while (while_loop_running) {
        // constantly visible info
        mvprintw(LINES-3, 2, "Use arrow keys to navigate");
        mvprintw(LINES-2, 2, "Right arrow to select, left arrow to go back");
        refresh();

        // what?
        werase(menu);
        werase(action);
        werase(info);
        
        // draw borders
        box(menu, 0, 0);
        box(action, 0, 0);
        box(info, 0, 0);        

        // window interaction for loop
        for (int i = 0; i < 5; i++) {
            if (i == selected) {
                wattron(menu, A_REVERSE);
            }

            mvwprintw(menu, i + 2, 3, "%-*s", menu_w - 6, menu_items[i]); // Uses a pointer to calculate the width of menu minus the spaces on either side (2+2) and minus the borders (1+1)
            if (i == 1) {
                mvwprintw(menu, i +2, 25, "(%d)", profile.ingredients_typed);
            }
            
            if (i == selected) {
                wattroff(menu, A_REVERSE);
            }
        }

        /* TEXT LOADING */
        // menu window title loader
        mvwprintw(menu, 0, 2, "[ SELECTION MENU ]");
        mvwprintw(info, 0, 2, "[ INFORMATION ]");
        mvwprintw(info, 0, 2, "[ INFORMATION ]");
        // both right windows' title loader);
        mvwprintw(action, 0, 2, "%s", action_titles[selected]);
        // contents (description) of windows
        mvwprintw(info, 2, 3, "%s", info_action_contents[selected][0]);
        mvwprintw(action, 2, 3, "%s", info_action_contents[selected][1]);
        
        // display
        wrefresh(menu);
        wrefresh(action);
        wrefresh(info);

        // wait for a key
        int menu_key_pressed = wgetch(menu);
        if (menu_key_pressed == KEY_UP && selected > 0) {
            selected--;
        } else if (menu_key_pressed == KEY_UP && selected == 0) {
            selected = 4;
        } else if (menu_key_pressed == KEY_DOWN && selected < 4) {
            selected++;
        } else if (menu_key_pressed == KEY_DOWN && selected == 4) {
            selected = 0;
        } else if (menu_key_pressed == '\n' && selected == 4) {
            while_loop_running = 0;
        }
    }

    // clean up
    delwin(menu);
    delwin(info);
    delwin(action);
    endwin();

    return 0;
}

void text_parser(char *text) {
    
    return;
}