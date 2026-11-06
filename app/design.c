/*This design c file tells the computer what it should do with each function*/
#include <ncursesw/ncurses.h> /*Library for: windows, text, frames and keyboard buttons*/
#include "design.h" /*Header file with our function declarations*/
/*HUSK kompiler sådan: gcc navn.c -o navn.exe -lncursesw*/
void lav_design(void)
{
    initscr(); /* Starts ncurses and makes the termianan ready*/
    noecho(); /*Users keyboard typing doesnt show as text in the terminal*/
    keypad(stdscr, TRUE); /*To register special keys like arrows in the standard window*/
    
start_color(); /*Activates usage of colors*/
/*colorpair id, text color, background color*/
    init_pair(1, COLOR_BLUE, COLOR_BLACK); 
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_BLACK, COLOR_BLACK);
}

void ingredients_menu (void) /*The functions that draws a new window when the user chooses 
"type in ingredients"*/
{
   WINDOW* ingredients_window;
    ingredients_window = newwin(8, 30, 1, 1);
    box(ingredients_window, 0, 0);
    mvwprintw(ingredients_window, 1, 1, "Type in your ingredients");
    wrefresh(ingredients_window);
    wgetch(ingredients_window);
    delwin(ingredients_window);

}

void lav_main_vindue(void) /*Function to create the main window (Welcome screen)*/
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

vis_main_menu(); /*This function is now called - computer will now perform code in this funct*/
}

void vis_main_menu(void) {/*The functions actions - creates next window with the interactive menu*/

    WINDOW* menu; /*First window where the user can choose different options*/
    WINDOW* menu2; /*Second window just to explain our  menu (maybe not necessary)*/
    // '*' means the variable is a pointer 
   
    int tast; 
    int valg = 0; /*Currently two choices in menu (0; ingredients and 1; exit means standard is always 0)*/


    menu = newwin(8, 30, 1, 1); /*Creates window 1 */
    menu2 = newwin(8, 25, 1, 35); /*Creates window 2*/
    /*newwin funct has four arguments: (height, width, start colomn, start row)*/

    keypad(menu, TRUE); /*Registers special keys in MENU så arrow up/down, ENTER etc*/
    
    wattron(menu, A_REVERSE); /*Funct to make highligt */
    wattroff(menu, A_REVERSE); /*Removes highlight*/

    wrefresh(menu); /**/
    wrefresh(menu2);


do { /*Loop for the window*/
    werase(menu); /*Clears the menu so interactive functions does not stay constantly on like 
    hightlights and colors*/

    box(menu, 0, 0); /*Draws a frame around the window 0,0 is ncurses standard frame
    'menu' makes sures it draws our menu window declared earlier*/
    mvwprintw(menu, 1, 1, "MENU"); /*Prints text in window 
    mvwprintw four arguments: (window, row, column, "text" */


    wattron(menu2, COLOR_PAIR(5)); /*Uses the color id 5, from color declaration ealier*/
    /*Working in 'menu2'*/
    box(menu2, 0, 0); /*Draws frame for 'menu2*/
    wattroff(menu2, COLOR_PAIR(5)); /*Turns off color*/

    
    /*Text and color in second window - used like this to color all the text same color*/
    wattron(menu2, COLOR_PAIR(3));
    mvwprintw(menu2, 1, 1, "Use key up/down");
    mvwprintw(menu2, 2, 1, "Press ENTER to choose");
     wattroff(menu2, COLOR_PAIR(3));


/*If statement is used to activate the functions based on users choice*/
if (valg == 0) { /*If the choice is 0 (currently 'ingredients') the following will happen:*/
    wattron(menu, COLOR_PAIR(2)); /*Color 2 will turn on*/
    wattron(menu, A_REVERSE); /*Highlighter will turn on*/
} 
    mvwprintw(menu, 2, 1, ">Type in ingredients"); /*The text in which the previous functions will work on*/
    wattroff(menu, A_REVERSE); /*Turns off highlighter*/
    wattroff(menu, COLOR_PAIR(2)); /*Turns off color*/


if (valg == 1) { /*If the other option is chosen (currently 'exit') */
    wattron(menu, A_REVERSE); /*Turns on highlighter */
    wattron(menu, COLOR_PAIR(2)); /*Turns on color 2*/
}
    mvwprintw(menu, 3, 1, ">Exit"); /*The text where previous functions is used on*/
    wattroff(menu, A_REVERSE); /*Turns off highlighter*/
    wattroff(menu, COLOR_PAIR(2)); /*Turns off color*/

wrefresh(menu);  /*Window with menu gets shown on the screen*/
wrefresh(menu2); /*Window with menu2 gets shown on the screen*/
 
tast = wgetch(menu); /*This function waits for the user to type something - 'w' getch 
means this only works in the window and not stdscr*/

/*Another if statement, if the user is on choice 0 or 1*/
    if (tast == KEY_UP) { /*If the user presses key up, the choice is 0 and the previous declaration
        of what happens when 0 is chosen gets activated (color, highliht and so on)*/
        valg = 0; 

    }

     else if (tast == KEY_DOWN) { /*If its key down instead the choice is 1 ^ same thing happens except 
        choice 1 */
        valg = 1;   
        }
   
     } while (tast != '\n' && tast!=KEY_ENTER); /*Means as long as the user DOES NOT type ENTER key the menu stays open in a loop*/
     {
    delwin(menu); /*Closes the window and releases the memory that ncurses was using*/
    delwin(menu2); 
     }

    if (valg==0){

        ingredients_menu();
    }
    else if (valg==1) {
        vis_main_menu();
    }



}


void ask_for_ingredients(char input[], int max_length)
{

}

    /*hvis valget er 0 og brugeren trykker ENTER så åbnes der et nyt vindue (type_ingredients_menu)


else if (valget er 1 og brugeren trykker ENTER så lukker vinduet og går tilbage til hoved menu)
*/
    /*indsæt ingrediens indtastning senere*/

void luk_design(void)
{
    endwin();
}


/*Menu kode:*/


/*

    if (valg ==0)
{
    ingredients_menu();
    box(ingredients_menu, 0, 0);
    wrefresh(ingredients_menu);
    delwin(ingredients_menu);
}
else if(valg == 1);
{
    return;

     WINDOW* ingredients_menu; /*The window that opens if the user chooses ingredients

        ingredients_menu = newwin(8, 30, 1, 1); /*Window for ingredients*/ 
