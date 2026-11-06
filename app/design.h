/*Denne fil er en slags indholdsfortegnelse */
/*Hvilke funktioner har jeg tænkt mig at lave*/

#ifndef DESIGN_H
#define DESIGN_H

/*List of functions*/

void lav_design(void); /*This function starts ncurses and readies the terminal for the design*/
void lav_main_vindue(void); /*Shows "welcome" screen before the menu (press space to show menu)*/
void vis_main_menu(void); /*Shows our main menu and the options (type in ingredients)  */
void menu_ingredients(void); /*Shows the menu if user chooses "type in ingredients"*/
void ask_for_ingredients(char input[], int max_length); /*For the user to type in their ingredients*/
void luk_design(void); /*Stops ncurses og "restarts" the terminal*/



#endif