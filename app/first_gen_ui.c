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


// String functions

void string_add(StringList *list, const char *text) {
    /*
    Takes as the first argument an addres of the stringlist to modify and as the second argument the string to add.
    Updates the stringlist, adding the string if it does not already exist and makes sure to allocate the nessesary memory.
    */

    // Don't add string if it already exists in the list.
    for (int i = 0; i < list->count; i++) {
        if (strcasecmp(list->items[i], text) == 0) return;
    }

    // Allocate memory for one additional string
    list->items = realloc(list->items, (list->count + 1) * sizeof(char *));

    // Add string and update string count.
    list->items[list->count] = strdup(text);
    list->count++;
}

void string_free(StringList *list) {
    /*
    Frees any memory used by a stringlist.
    */
    for (int i = 0; i < list->count; i++){
        free(list->items[i]);
    } 
    free(list->items);
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


void choose_best_recipes(cJSON *meals, const StringList *chosen, int number, cJSON *best[], int scores[], StringList matching[]) {
    /*
    Updates 3 arrays outside the function that for each best recipe found stores: 
    - best[]: the recipe-object
    - scores[]: number of ingredients matching the user chosen ingredients
    - matching[]: a stringlist containing all the matching strings for the recipe
    (It might make sense to get rid of the scores[] array since the information is already contained in stringlist.count
    However the best implimentation will likely be more clear once we incoorperate the remaining prefference functionality)
    */

    // Initiate our top recipies
    for (int i = 0; i < number; i++) {
        best[i] = NULL;
        scores[i] = -1;
        matching[i] = (StringList){0};
    }

    // Loop through each recipe of our json file
    cJSON *recipe;
    cJSON_ArrayForEach(recipe, meals) {

        StringList matches = {0};

        // For each recipe, loop through all ingredients chosen by the user adding matches to our "matches" stringlist.
        cJSON *ingredients = cJSON_GetObjectItem(recipe, "ingredients");
        for (int i = 0; i < chosen->count; i++) {
            cJSON *ingredient;
            cJSON_ArrayForEach(ingredient, ingredients) {
                cJSON *name = cJSON_GetObjectItem(ingredient, "name");
                if (cJSON_IsString(name) && strcasecmp(chosen->items[i], name->valuestring) == 0) {
                    string_add(&matches, chosen->items[i]);
                    break;
                }
            }
        }

        // Loop through our 3 top recipe arrays outside the function updating each array with the new match info.
        int inserted = 0;
        for (int i = 0; i < number; i++) {
            if (matches.count > scores[i]) {
                string_free(&matching[number - 1]);
                for (int j = number - 1; j > i; j--) {
                    best[j] = best[j - 1];
                    scores[j] = scores[j - 1];
                    matching[j] = matching[j - 1];
                }
                best[i] = recipe;
                scores[i] = matches.count;
                matching[i] = matches;
                inserted = 1;
                break;
            }
        }
        if (!inserted) string_free(&matches);
    }
}




// Terminal UI functions (Will later be replaced with Ccurses)

static const char *suggested_numbers;

int insert_suggestions(void) {
    /*
    The readline lib needs this function as an argument to insert a string in the terminal.
    */
    rl_insert_text(suggested_numbers);
    return 0;
}
StringList chose_from(StringList starting_list, const char *lookup_string) {
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
    rl_startup_hook = insert_suggestions;
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

void show_best_recipes(cJSON *best[], int scores[], const StringList matching[], int number) {
    /*
    Displays the the best found recipe options.
    Takes as arguments a cJSON object array pointer of the best recipes, their scores/matches and a number indicating how many to display.
    */

    printf("Best matches:\n");
    for (int i = 0; i < number; i++) {
        if (best[i] == NULL){
            continue;
        } 
        cJSON *name = cJSON_GetObjectItem(best[i], "name");
        printf("%d: %s (%d matching ingredients - ", i + 1,
               name->valuestring, scores[i]);

        for (int j = 0; j < matching[i].count; j++) {
            if (j > 0) printf(", ");
            printf("%s", matching[i].items[j]);
        }
        if (matching[i].count == 0) printf("none");
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

StringList ui_choose_ingredients(void) {
    /*
    Controlls the flow allowing the user to search for and choose ingredients in a loop untill they are done. 
    The choice of selected ingredients is handled by the choose_from() function which is called inside this function.
    Returns a stringlist of the combined chosen ingredients.
    */

    // Loop forever untill the user types "d" for done.
    StringList all_chosen = {0};
    char lookup[100];
    while (1) {
        printf("Enter ingredient (d = done): ");
        fgets(lookup, sizeof(lookup), stdin);
        lookup[strcspn(lookup, "\r\n")] = '\0';
        if (strcmp(lookup, "d") == 0){
            break;
        } 
        if (lookup[0] == '\0'){
            continue;
        } 

        // Find ingredients with a substring matching the lookup string.
        StringList found = search_ingredients(lookup);
        printf("Found %d ingredients matching the lookup string\n", found.count);

        // For each search let the user choose ingredients to add to the total chosen ingredients and display the updates.
        if (found.count > 0) {
            StringList chosen = chose_from(found, lookup);
            int before = all_chosen.count;
            for (int i = 0; i < chosen.count; i++)
                string_add(&all_chosen, chosen.items[i]);
            printf("Added %d ingredients. Total is now %d\n",
                   all_chosen.count - before, all_chosen.count);
            string_free(&chosen);
        } else {
            printf("Added 0 ingredients. Total is now %d\n", all_chosen.count);
        }
        string_free(&found);
    }
    return all_chosen;
}




// Main

int main(void) {
    int profile = ui_profile();
    if (profile != 0) {
        printf("User profiles are currently not supported\n");
        return 1;
    }

    StringList all_chosen = ui_choose_ingredients();

    cJSON *root = load_recipes("recipes_with_portions.json");
    if (root != NULL) {
        cJSON *meals = cJSON_GetObjectItem(root, "meals");
        int number = 3;
        cJSON *best[number];
        int scores[number];
        StringList matching[number];

        choose_best_recipes(meals, &all_chosen, number, best, scores, matching);
        show_best_recipes(best, scores, matching, number);
        for (int i = 0; i < number; i++){
            string_free(&matching[i]);
        } 
        cJSON_Delete(root);
    }
    string_free(&all_chosen);
    return 0;
}