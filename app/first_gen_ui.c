/*
Compile and run: (might require installs)

gcc first_gen_ui.c cJSON.c -o first_gen_ui -lreadline
./first_gen_ui
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <readline/readline.h>
#include "cJSON.h"

// Datatypes

typedef struct {
    char **items;
    int count;
} StringList;

typedef struct {
    StringList allergies;
    StringList allowed_categories;
    StringList allowed_areas;
    StringList favorite_categories;
    StringList favorite_areas;
} UserPreferences;

typedef struct {
    cJSON *recipe;
    float score;
    StringList matching;
} RecipeResult;


// String functions

void string_add(StringList *list, const char *text) {
    /*
    Takes as the first argument an addres of the stringlist to modify and as the second argument the string to add.
    Updates the stringlist, adding the string if it does not already exist and makes sure to allocate the nessesary memory.
    */

    // Don't add string if it already exists in the list.
    for (int i = 0; i < list->count; i++) {
        if (strcasecmp(list->items[i], text) == 0){
            return;
        } 
    }

    // Allocate memory for one additional string
    list->items = realloc(list->items, (list->count + 1) * sizeof(char *));

    // Add string and update string count.
    list->items[list->count] = strdup(text);
    list->count++;
}

void string_free(StringList list) {
    /*
    Frees any memory used by a stringlist.
    */
    for (int i = 0; i < list.count; i++){
        free(list.items[i]);
    } 
    free(list.items);
}

int list_contains(StringList strings, const char *test_string){
    /*
    Returns true if a stringlist contains a specific string.
    */
    for(int i = 0; i < strings.count; i++){
        if (strcasecmp(test_string, strings.items[i]) == 0){
            return 1;
        }
    }
    return 0;
}


// Functions used by ingredient selection UI

int word_count(const char *text) {
    /*
    Returns the number of words in an input string
    */

    int count = 1;

    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == ' ')
            count++;
    }

    return count;
}

int compare_ingredients(const void *a, const void *b) {
    /*
    Takes as input two strings and returns:
    - a positive number if the first string has more words
    - a negative number if the second string has more words
    - 0 if they have the same number of words.
    */

    const char *first = *(const char **)a;
    const char *second = *(const char **)b;

    return word_count(first) - word_count(second);
}

StringList search_ingredients(const char *lookup_string) {
    /*
    Takes as input our ingredients.txt file and a the users lookupstring. 
    We initiate a stringlist called "matches". 
    We then loop through the file line by line adding any line containing the lookupstring as a substring to "matches".
    Finally we sort the found matches so the fewer number of words are placed first and returns the stringlist.
    */

    StringList matches = {0};
    FILE *file = fopen("ingredients.txt", "r");

    char *line = NULL;
    size_t size = 0;

    while (getline(&line, &size, file) != -1) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strstr(line, lookup_string) != NULL){
            string_add(&matches, line);
        }            
    }

    free(line);
    fclose(file);

    qsort(matches.items, matches.count, sizeof(char *), compare_ingredients);

    return matches;
}

int suggest_ingredient(const char *ingredient, const char *lookup) {
    /*
    Return 1 if wordcount of ingredient matches that of the lookup string and lookupstring is a substring of the full string.
    */

    return word_count(ingredient) == word_count(lookup) && strncasecmp(ingredient, lookup, strlen(lookup)) == 0;
}


// Cjson functions

cJSON *load_recipes(const char *filename) {
    /*
    Reads our JSON file and returns its contents as a cJSON object.
    Assumes the file exists, is readable and contains valid JSON.
    */

    FILE *file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *text = malloc(size + 1);
    fread(text, 1, size, file);
    text[size] = '\0';

    fclose(file);

    cJSON *root = cJSON_Parse(text);
    free(text);

    return root;
}


cJSON *filter_allowed_recipes(cJSON *all_recipes, UserPreferences preferences){
    /*
    Takes as input a cJSON recipes object and user preferences for allowed areas, categories and allergies.
    Returns a cJSON recipes object with all recipes that does not violate the limitations of the 3 stringlists.
    */
    cJSON *allowed_recipes = cJSON_CreateArray();

    for(int i = 0; i < cJSON_GetArraySize(all_recipes); i++){
        cJSON *recipe = cJSON_GetArrayItem(all_recipes, i);

        if (preferences.allowed_categories.count < 14){ // Rather than having a magic number these should be calculated at the start of the program.
            cJSON *category_item = cJSON_GetObjectItem(recipe, "category");
            if (!cJSON_IsString(category_item) || !list_contains(preferences.allowed_categories, category_item->valuestring)) {
                continue;
            }
        }

        if (preferences.allowed_areas.count < 37){ // Rather than having a magic number these should be calculated at the start of the program.
            cJSON *area_item = cJSON_GetObjectItem(recipe, "area");
            if (!cJSON_IsString(area_item) || !list_contains(preferences.allowed_areas, area_item->valuestring)) {
                continue;
            }
        }

        int passes_allergy_test = 1;
        cJSON *ingredients_object = cJSON_GetObjectItem(recipe, "ingredients");
        for(int j = 0; j < cJSON_GetArraySize(ingredients_object); j++){
            cJSON *ingredient_object = cJSON_GetArrayItem(ingredients_object, j);
            char *ingredient = cJSON_GetObjectItem(ingredient_object, "name")->valuestring;
            if (list_contains(preferences.allergies, ingredient)){
                passes_allergy_test = 0;
                break;
            }
        }
        if (!passes_allergy_test) {
            continue;
        }

        cJSON_AddItemToArray(allowed_recipes, cJSON_Duplicate(recipe, 1));
    }
    return allowed_recipes;
}

void choose_best_recipes(cJSON *meals, StringList chosen, UserPreferences preferences, int number, RecipeResult best[]) {
    /*
    Updates the array outside the function with the top recipes. Each result stores:
    - recipe: the recipe-object
    - score: the total score of the recipe
    - matching: a stringlist containing all matching ingredients
    */

    // Initiate our top recipies
    for (int i = 0; i < number; i++) {
        best[i].recipe = NULL;
        best[i].score = -1;
        best[i].matching = (StringList){0};
    }

    // Loop through each recipe of our json file
    cJSON *recipe;
    cJSON_ArrayForEach(recipe, meals) {

        StringList matches = {0};

        // For each recipe, loop through all ingredients chosen by the user adding matches to our "matches" stringlist.
        cJSON *ingredients = cJSON_GetObjectItem(recipe, "ingredients");
        for (int i = 0; i < chosen.count; i++) {
            cJSON *ingredient;
            cJSON_ArrayForEach(ingredient, ingredients) {
                cJSON *name = cJSON_GetObjectItem(ingredient, "name");
                if (cJSON_IsString(name) && strcasecmp(chosen.items[i], name->valuestring) == 0) {
                    string_add(&matches, chosen.items[i]);
                    break;
                }
            }
        }

        // Check for prefferences
        int is_favorite_category = 0;
        if (preferences.favorite_categories.count > 0){
            cJSON *category_item = cJSON_GetObjectItem(recipe, "category");
            if (!cJSON_IsString(category_item)) {
                string_free(matches);
                continue;
            }
            char *category = category_item->valuestring;
            is_favorite_category = list_contains(preferences.favorite_categories, category);
        }

        int is_favorite_area = 0;
        if (preferences.favorite_areas.count > 0){
            cJSON *area_item = cJSON_GetObjectItem(recipe, "area");
            if (!cJSON_IsString(area_item)) {
                string_free(matches);
                continue;
            }
            char *area = area_item->valuestring;
            is_favorite_area = list_contains(preferences.favorite_areas, area);
        }

        // Calculate the total score
        float score = matches.count;
        if (is_favorite_category){
            score += 0.1;
        }
        if (is_favorite_area){
            score += 0.1;
        }

        // Insert the recipe into the top-N list
        int i;
        for (i = 0; i < number; i++) {
            if (score > best[i].score) {
                break;
            }
        }

        if (i == number) {
            string_free(matches);
            continue;
        }

        string_free(best[number - 1].matching);

        for (int j = number - 1; j > i; j--) {
            best[j] = best[j - 1];
        }

        best[i] = (RecipeResult){recipe, score, matches};
    }
}




// Terminal UI functions (Will later be replaced with Ccurses)

static const char *suggested_numbers;

int ui_insert_suggestions(void) {
    /*
    The readline lib needs this function as an argument to insert a string in the terminal.
    */
    rl_insert_text(suggested_numbers);
    return 0;
}


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

    // Insert the suggested numbers string into the terminal usingthe readline lib
    suggested_numbers = defaults;
    rl_startup_hook = ui_insert_suggestions;
    char *input = readline("Enter numbers: ");
    rl_startup_hook = NULL;

    // Convert user input to a stringlist of chosen ingredients and return it.
    if (input == NULL){
        return chosen;
    } 
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
    free(input);
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


int ui_profile(void) {
    /*
    Displays the user profile menu. For now only 0 is supported. Will be changed later
    */
    int profile;
    printf("Welcome. How would you like to start the meal planner?:\n");
    printf("- 0 Continue without user profile\n");
    printf("- 1 Create new user profile\n");
    printf("- 2 Use existing profile - Christoffer\n");
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

        // Find ingredients with a substring matching the lookup string.
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
    return ui_search_and_choose("Enter ingredient (d = done): ", 0);
}


// Preference selection UI

StringList ui_select_options(StringList options, const char *title, int default_all) {
    /*
    Displays a list of options and lets the user select multiple numbers.
    Pressing Enter uses the default: all options for allowed lists, none for favorites.
    */
    StringList selected = {0};

    printf("\n%s\n", title);
    for (int i = 0; i < options.count; i++) {
        printf("%d: %s\n", i + 1, options.items[i]);
    }

    if (default_all) {
        printf("Enter numbers separated by spaces (Enter = all): ");
    } else {
        printf("Enter numbers separated by spaces (Enter = none): ");
    }

    char input[300];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        input[0] = '\0';
    }
    input[strcspn(input, "\r\n")] = '\0';

    if (input[0] == '\0' && default_all) {
        for (int i = 0; i < options.count; i++) {
            string_add(&selected, options.items[i]);
        }
    } else {
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

StringList ui_choose_allergies(void) {
    /*
    Lets the user search and select excluded ingredients.
    Press Enter without typing a search to finish (default: no allergies).
    */
    printf("\nAllergies / excluded ingredients (Enter = none / done)\n");
    return ui_search_and_choose("Search for an ingredient (Enter = done): ", 1);
}

UserPreferences ui_choose_preferences(void) {
    /*
    Returns user preferences. Allowed categories and areas default to all;
    allergies and favorite categories and areas default to none.
    */
    UserPreferences preferences = {0};

    StringList categories = {(char *[]) {"Beef", "Breakfast", "Chicken", "Dessert", "Goat", "Lamb", "Miscellaneous", "Pasta", "Pork", "Seafood", "Side", "Starter", "Vegan", "Vegetarian"}, 14};
    StringList areas = {(char *[]) {"Algerian", "Argentina", "Australian", "British", "Canadian", "Chinese", "Croatian", "Egyptian", "Filipino", "France", "Greek", "India", "Irish", "Italian", "Jamaican", "Japanese", "Kenyan", "Malaysian", "Mexican", "Moroccan", "Netherlands", "Norway", "Polish", "Portuguese", "Russian", "Saudi Arabian", "Slovakia", "Spanish", "Syrian", "Thai", "Tunisian", "Turkish", "Ukrainian", "United States", "Uruguayan", "Venezuela", "Vietnamese"}, 37};

    printf("\nChoose your preferences (press Enter to accept each default).\n");
    preferences.allowed_categories = ui_select_options(categories, "Allowed categories", 1);
    preferences.allowed_areas = ui_select_options(areas, "Allowed areas", 1);
    preferences.allergies = ui_choose_allergies();
    preferences.favorite_categories = ui_select_options(categories, "Favorite categories", 0);
    preferences.favorite_areas = ui_select_options(areas, "Favorite areas", 0);

    return preferences;
}


// Main

int main(void) {
    int profile = ui_profile();
    if (profile != 0) {
        printf("User profiles are currently not supported\n");
        return 1;
    }

    StringList all_chosen = ui_choose_ingredients();
    UserPreferences preferences = ui_choose_preferences();

    cJSON *root = load_recipes("recipes_with_portions.json");
    if (root != NULL) {
        cJSON *meals = cJSON_GetObjectItem(root, "meals");
        cJSON *allowed_recipes = filter_allowed_recipes(meals, preferences);

        int number = 3;
        RecipeResult best[number];
        choose_best_recipes(allowed_recipes, all_chosen, preferences, number, best);
        ui_show_best_recipes(best, number);

        for (int i = 0; i < number; i++) {
            string_free(best[i].matching);
        }
        cJSON_Delete(allowed_recipes);
        cJSON_Delete(root);
    }

    string_free(all_chosen);
    string_free(preferences.allergies);
    string_free(preferences.allowed_categories);
    string_free(preferences.allowed_areas);
    string_free(preferences.favorite_categories);
    string_free(preferences.favorite_areas);
    return 0;
}