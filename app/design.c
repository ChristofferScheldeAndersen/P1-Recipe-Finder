#include <ncursesw/ncurses.h>
#include <stdio.h>
#include "design.h"
/*HUSK kompiler sådan: gcc navn.c -o navn.exe -lncursesw*/
void lav_design(void)
{
    initscr(); /*Starter ncurses - og gør terminalen klar*/
    noecho(); /*Så brugerens tastetryk ikke vises som tekst i terminalen*/
    keypad(stdscr, TRUE); /*Til at bruge piletaster*/
}

void lav_main_vindue(void)
{
    box(stdscr, 0, 0);

    mvprintw(1, 35, "Welcome to our meal planning program");
    mvprintw(3, 40, "Press ENTER to show menu");

    refresh();
}

void vis_main_menu(void)
{
    /*tilføj menu senere*/
}

void ask_for_ingredients(char input[], int max_length)
{
    /*indsæt ingrediens indtastning senere*/
}
void luk_design(void)
{
    endwin();
}