#include "design.h"
#include <ncursesw/ncurses.h>

int main(void)
{
    lav_design();

    lav_main_vindue();

    getch();

    luk_design();

    return 0;

}