/*Dette dokument er til at lave funktionerne*/

#include <stdio.h>
#include <ncurses/ncurses.h>
#include "ui.h"

/*Lav - */
void start_screen(void) {
box(stdscr, 0, 0); 
int tast;
for (int i = 0; i < 15; i++)


clear();
mvprintw(i, 35, "Welcome to our meal planning program");
refresh();
napms(100);


    attron(COLOR_PAIR(3));
    napms(500);
    mvprintw(10, 40, "Press SPACE to show menu");
    attroff(COLOR_PAIR(3));
    refresh(); 

}


void input_ingredients(WINDOW *action) {

    char ingredient[100];

    werase(action);
    box(action, 0, 0);

    mvwprintw(action, 2, 3, "Type in your ingredients:");

    echo();
    curs_set(1);

    wmove(action, 3, 3);
    wrefresh(action);

    wgetnstr(action, ingredient, 99);

    noecho();
    curs_set(0);

    mvwprintw(action, 5, 3, "You typed in: %s", ingredient);

    wrefresh(action);
    wgetch(action);
}


