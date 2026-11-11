/*Dette dokument er til at lave funktionerne*/

#include <stdio.h>
#include <ncurses/ncurses.h>

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



