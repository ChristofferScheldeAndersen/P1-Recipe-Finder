#include <stdio.h>
#include <ncurses/ncurses.h>



void lav_design(void)
{
    initscr(); /* Starts ncurses and makes the termianan ready*/
    noecho(); /*Users keyboard typing doesnt show as text in the terminal*/
    keypad(stdscr, TRUE); /*To register special keys like arrows in the standard window*/

/*Function for color pairs*/
start_color(); /*Activates usage of colors*/
/*colorpair id, text color, background color*/
    init_pair(1, COLOR_BLUE, COLOR_BLACK); 
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_BLACK, COLOR_BLACK);
}

void main_screen(void) /*Function to create the main window (Welcome screen)*/
{
    int tast; /*Variable to save the key, the user presses*/
    box(stdscr, 0, 0); /*Draws a frame around standardwindow (stdscr)
    0,0 means ncurses standard frame */
    /*Box  = (vindue, tegn til sider, tegn til top/bund)*/

    attron(COLOR_PAIR(4)); /*Call to the function of colors (used color pair 4)*/
    mvprintw(2, 30, "Welcome to our meal planning program"); /*mvprintw shows text in the window*/
    /*mvprintw(Række, Kolonne "tekst")*/
    /*mv = move 
    printw = print in window*/
    attroff(COLOR_PAIR(4)); /*turns off the color */


    mvprintw(6, 35, "Press SPACE to show menu");

    refresh(); /*Updates the standard window and shows changes in terminal */
   
    do /*Loop*/
    {
        tast = getch(); /*Waits for the user to press a key - key is saved in variable "tast"*/
    }
    while (tast != ' '); /*As long as pressed key is not space (space changes closes the window)*/
    /*While loop: Keep waiting for key as long as its not 'space' */

     werase(stdscr); /*When the user presses 'space' the window (stdscr) gets erased*/
     wrefresh(stdscr); /*This command updates the standard window after previous deletion */

}