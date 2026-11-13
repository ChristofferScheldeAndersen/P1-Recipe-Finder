#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui.h"

// Terminal UI functions (Will later be replaced with Ccurses)

StringList ui_chose_from(StringList starting_list, const char *lookup_string) {
    /*
    Displays the ingredients matching the lookupstring sortet by number of words. 
    Ingredients with the same number of words are suggested by default.
    The user can then add or remove choices.
    Takes as arguments a stringlist containing all matching ingredients and the users lookupstring.
    Returns a stringlist with the chosen ingredients.
    */

    // Display options
    StringList chosen = {0};
    printf("Found the following ingredient matches (0 for all):\n");
    for (int i = 0; i < starting_list.count; i++)
        printf("%d: %s\n", i + 1, starting_list.items[i]);

    // Create a string containing the suggested indexes
    char defaults[100] = "";
    for (int i = 0; i < starting_list.count; i++) {
        if (suggest_ingredient(starting_list.items[i], lookup_string)) {
            char number[16];
            snprintf(number, sizeof(number), "%d ", i + 1);
            if (strlen(defaults) + strlen(number) < sizeof(defaults))
                strcat(defaults, number);
        }
    }

    // Enter accepts the suggested numbers.
    printf("Enter numbers (Enter = %s): ", defaults);
    char input[300];
    if (fgets(input, sizeof(input), stdin) == NULL) return chosen;
    input[strcspn(input, "\r\n")] = '\0';
    if (input[0] == '\0') strcpy(input, defaults);

    // Convert user input to a stringlist of chosen ingredients and return it.
    if (strcmp(input, "0") == 0) {
        for (int i = 0; i < starting_list.count; i++)
            string_add(&chosen, starting_list.items[i]);
    } else {
        char *token = strtok(input, " \t");
        while (token != NULL) {
            int number = atoi(token);
            if (number >= 1 && number <= starting_list.count){
                string_add(&chosen, starting_list.items[number - 1]);
            }
            token = strtok(NULL, " \t");
        }
    }
    return chosen;
}


void ui_show_best_recipes(RecipeResult best[], int number) {
    /*
    Displays the the best found recipe options.
    Takes as arguments an array of recipe results and a number indicating how many to display.
    */

    printf("Best matches:\n");
    for (int i = 0; i < number; i++) {
        if (best[i].recipe == NULL){
            continue;
        }
        cJSON *name = cJSON_GetObjectItem(best[i].recipe, "name");
        printf("%d: %s (score: %.2f, %d matching ingredients - ", i + 1,
               name->valuestring, best[i].score, best[i].matching.count);

        for (int j = 0; j < best[i].matching.count; j++) {
            if (j > 0) printf(", ");
            printf("%s", best[i].matching.items[j]);
        }
        if (best[i].matching.count == 0) printf("none");
        printf(")\n");
    }
}


int ui_profile(User users[], int count) {
    /*
    Displays the user profile menu.
    */
    int profile;
    printf("Welcome. How would you like to start the meal planner?:\n");
    printf("- 0 Continue without user profile\n");
    printf("- 1 Create new user profile\n");
    for (int i = 0; i < count; i++) {
        printf("- %d Use existing profile - %s\n", i + 2, users[i].name);
    }
    if (scanf("%d", &profile) != 1) return -1;

    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }

    return profile;
}

StringList ui_search_and_choose(const char *prompt, int finish_on_empty) {
    /*
    Lets the user search and select ingredients or allergies.
    Returns a stringlist of all selected ingredients.
    */
    StringList all_chosen = {0};
    char lookup[100];
    while (1) {
        printf("%s", prompt);
        if (fgets(lookup, sizeof(lookup), stdin) == NULL) {
            break;
        }
        lookup[strcspn(lookup, "\r\n")] = '\0';
        if (strcmp(lookup, "d") == 0){
            break;
        }
        if (lookup[0] == '\0'){
            if (finish_on_empty) {
                break;
            }
            continue;
        }

        // Find ingredients with a word starting with the lookup string.
        StringList found = search_ingredients(lookup);
        printf("Found %d ingredients matching the lookup string\n", found.count);

        // For each search let the user choose ingredients to add to the total chosen ingredients and display the updates.
        if (found.count > 0) {
            StringList chosen = ui_chose_from(found, lookup);
            int before = all_chosen.count;
            for (int i = 0; i < chosen.count; i++)
                string_add(&all_chosen, chosen.items[i]);
            printf("Added %d ingredients. Total is now %d\n",
                   all_chosen.count - before, all_chosen.count);
            string_free(chosen);
        } else {
            printf("Added 0 ingredients. Total is now %d\n", all_chosen.count);
        }
        string_free(found);
    }
    return all_chosen;
}

StringList ui_choose_ingredients(void) {
    /*
    Controlls the flow allowing the user to search for and choose ingredients in a loop untill they are done.
    The choice of selected ingredients is handled by the choose_from() function which is called inside this function.
    Returns a stringlist of the combined chosen ingredients.
    */
    printf("\nEnter the ingredients you have:\n");
    return ui_search_and_choose("Enter ingredient (d = done): ", 0);
}


// Preference selection UI

StringList ui_select_options(StringList options, const char *title, StringList current) {
    /*
    Displays a list of options and lets the user select multiple numbers.
    Pressing Enter keeps the current selection; 0 clears it.
    */
    StringList selected = {0};

    printf("\n%s\n", title);
    for (int i = 0; i < options.count; i++) {
        printf("%d: %s", i + 1, options.items[i]);
        if (list_contains(current, options.items[i])) printf(" (selected)");
        printf("\n");
    }
    printf("Enter numbers separated by spaces (Enter = keep, 0 = clear): ");

    char input[300];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        input[0] = '\0';
    }
    input[strcspn(input, "\r\n")] = '\0';

    if (input[0] == '\0') {
        selected = string_copy(current);
    } else if (strcmp(input, "0") != 0) {
        char *token = strtok(input, " \t");
        while (token != NULL) {
            int index = atoi(token);
            if (index >= 1 && index <= options.count) {
                string_add(&selected, options.items[index - 1]);
            }
            token = strtok(NULL, " \t");
        }
    }

    return selected;
}

StringList ui_choose_allergies(StringList current) {
    /*
    Lets the user search and select excluded ingredients.
    Press Enter to keep existing allergies, or 0 to replace them.
    */
    if (current.count > 0) {
        printf("\nCurrent excluded ingredients: ");
        for (int i = 0; i < current.count; i++) {
            if (i > 0) printf(", ");
            printf("%s", current.items[i]);
        }
        printf("\nEnter = keep, 0 = choose new list: ");
        char answer[20] = "";
        fgets(answer, sizeof(answer), stdin);
        if (answer[0] != '0') return string_copy(current);
    }
    printf("\nAllergies / excluded ingredients (Enter = none / done)\n");
    return ui_search_and_choose("Search for an ingredient (Enter = done): ", 1);
}

UserPreferences ui_choose_preferences(UserPreferences current, StringList categories, StringList areas) {
    /*
    Returns user preferences. Enter keeps the current selection.
    An empty allowed_categories list means all categories are allowed.
    */
    UserPreferences preferences = {0};

    printf("\nChoose your preferences (press Enter to accept each default).\n");
    preferences.allowed_categories = ui_select_options(categories, "Allowed categories (empty = all)", current.allowed_categories);
    preferences.allergies = ui_choose_allergies(current.allergies);
    preferences.favorite_categories = ui_select_options(categories, "Favorite categories", current.favorite_categories);
    preferences.favorite_areas = ui_select_options(areas, "Favorite areas", current.favorite_areas);

    return preferences;
}


void ask(const char *question, char *answer, int size) {
    printf("%s", question);
    if (fgets(answer, size, stdin) == NULL) {
        answer[0] = '\0';
        return;
    }
    answer[strcspn(answer, "\n")] = '\0';
}

void create_user(User *u) {
    do {
        ask("Name: ", u->name, sizeof(u->name));
    } while (u->name[0] == '\0');
}

