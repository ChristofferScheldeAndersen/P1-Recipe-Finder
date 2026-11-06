void vis_main_menu(void)

{

    WINDOW* menu;
    int tast;
    int valg = 0;

    menu = newwin(5, 30, 1, 1);

    keypad(menu, TRUE);
    
    wattron(menu, A_REVERSE); /*Til at highlight*/
    wattroff(menu, A_REVERSE); /*Fjerner highlight*/

    box(menu, 0, 0);
    mvwprintw(menu, 1, 1, "MENU");
    mvwprintw(menu, 2, 3, ">Type in ingredients");
    mvwprintw(menu, 3, 4, ">Exit");

    wrefresh(menu);

do {
    tast = wgetch(menu);


    if (tast == KEY_UP) {
        valg = 0;   
        wattron(menu, A_REVERSE);{
    }

  mvwprintw(menu, 2, 3, ">Type in ingredients");

    wattroff(menu, A_REVERSE); {
}
            mvwprintw(menu, 3, 4, ">Exit");

  

     } else if (tast == KEY_DOWN) {
        valg = 1;
        wattroff(menu, A_REVERSE); {
             mvwprintw(menu, 2, 3, ">Type in ingredients");
            
        }
        wattron(menu, A_REVERSE); {
        mvwprintw(menu, 3, 4, ">Exit");
        }
     }
}
while (tast !='\n');

delwin(menu);