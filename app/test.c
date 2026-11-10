/*This design c file tells the computer what it should do with each function*/
#include <ncursesw/ncurses.h> /*Library for: windows, text, frames and keyboard buttons*/ /*Header file with our function declarations*/
/*HUSK kompiler sådan: gcc navn.c -o navn.exe -lncursesw*/
#include "test.h"

int main(void)
{
    lav_design();
    main_screen();
    close_design();

    return 0;
}