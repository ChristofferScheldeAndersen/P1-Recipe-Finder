#ifndef UI_H
#define UI_H

#include "backend.h"

StringList ui_chose_from(StringList starting_list, const char *lookup_string);
void ui_show_best_recipes(RecipeResult best[], int number);
int ui_profile(User users[], int count);
StringList ui_search_and_choose(const char *prompt, int finish_on_empty);
StringList ui_choose_ingredients(void);
StringList ui_select_options(StringList options, const char *title, StringList current);
StringList ui_choose_allergies(StringList current);
UserPreferences ui_choose_preferences(UserPreferences current, StringList categories, StringList areas);
void ask(const char *question, char *answer, int size);
void create_user(User *u);

#endif
