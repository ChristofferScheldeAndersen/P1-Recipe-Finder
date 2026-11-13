#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "backend.h"

// String functions

int string_compare(const char *a, const char *b) {
    while (*a && *b && tolower((unsigned char)*a) == tolower((unsigned char)*b)) {
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int string_compare_n(const char *a, const char *b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int x = tolower((unsigned char)a[i]);
        int y = tolower((unsigned char)b[i]);
        if (x != y || x == 0) return x - y;
    }
    return 0;
}

char *string_duplicate(const char *text) {
    char *copy = malloc(strlen(text) + 1);
    strcpy(copy, text);
    return copy;
}

void string_add(StringList *list, const char *text) {
    /*
    Takes as the first argument an addres of the stringlist to modify and as the second argument the string to add.
    Updates the stringlist, adding the string if it does not already exist and makes sure to allocate the nessesary memory.
    */

    // Don't add string if it already exists in the list.
    for (int i = 0; i < list->count; i++) {
        if (string_compare(list->items[i], text) == 0){
            return;
        } 
    }

    // Allocate memory for one additional string
    list->items = realloc(list->items, (list->count + 1) * sizeof(char *));

    // Add string and update string count.
    list->items[list->count] = string_duplicate(text);
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
        if (string_compare(test_string, strings.items[i]) == 0){
            return 1;
        }
    }
    return 0;
}

StringList string_copy(StringList original) {
    StringList copy = {0};
    for (int i = 0; i < original.count; i++) {
        string_add(&copy, original.items[i]);
    }
    return copy;
}

UserPreferences preferences_copy(UserPreferences original) {
    UserPreferences copy = {0};
    copy.allergies = string_copy(original.allergies);
    copy.allowed_categories = string_copy(original.allowed_categories);
    copy.favorite_categories = string_copy(original.favorite_categories);
    copy.favorite_areas = string_copy(original.favorite_areas);
    return copy;
}

void preferences_free(UserPreferences preferences) {
    string_free(preferences.allergies);
    string_free(preferences.allowed_categories);
    string_free(preferences.favorite_categories);
    string_free(preferences.favorite_areas);
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
    We then loop through the file line by line adding any line where a word starts with the lookupstring to "matches".
    Finally we sort the found matches so the fewer number of words are placed first and returns the stringlist.
    */

    StringList matches = {0};
    FILE *file = fopen("ingredients.txt", "r");

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        for (char *p = line; *p != '\0'; p++) {
            if ((p == line || !isalnum((unsigned char)p[-1])) &&
                string_compare_n(p, lookup_string, strlen(lookup_string)) == 0) {
                string_add(&matches, line);
                break;
            }
        }
    }

    fclose(file);

    qsort(matches.items, matches.count, sizeof(char *), compare_ingredients);

    return matches;
}

int suggest_ingredient(const char *ingredient, const char *lookup) {
    /*
    Return 1 if wordcount of ingredient matches that of the lookup string and lookupstring is a substring of the full string.
    */

    return word_count(ingredient) == word_count(lookup) && string_compare_n(ingredient, lookup, strlen(lookup)) == 0;
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


int compare_options(const void *a, const void *b) {
    return string_compare(*(const char **)a, *(const char **)b);
}

StringList recipe_options(cJSON *meals, const char *field) {
    StringList options = {0};
    cJSON *recipe;
    cJSON_ArrayForEach(recipe, meals) {
        cJSON *item = cJSON_GetObjectItem(recipe, field);
        if (cJSON_IsString(item)) {
            string_add(&options, item->valuestring);
        }
    }
    qsort(options.items, options.count, sizeof(char *), compare_options);
    return options;
}

cJSON *filter_allowed_recipes(cJSON *all_recipes, UserPreferences preferences){
    /*
    Takes as input a cJSON recipes object and user preferences for allowed categories and allergies.
    Returns a cJSON recipes object with all recipes that does not violate these limitations.
    */
    cJSON *allowed_recipes = cJSON_CreateArray();

    for(int i = 0; i < cJSON_GetArraySize(all_recipes); i++){
        cJSON *recipe = cJSON_GetArrayItem(all_recipes, i);

        if (preferences.allowed_categories.count > 0){
            cJSON *category_item = cJSON_GetObjectItem(recipe, "category");
            if (!cJSON_IsString(category_item) || !list_contains(preferences.allowed_categories, category_item->valuestring)) {
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
                if (cJSON_IsString(name) && string_compare(chosen.items[i], name->valuestring) == 0) {
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




// User profile functions

void copy_text(char *to, const char *from, int size) {
    strncpy(to, from, size - 1);
    to[size - 1] = '\0';
}

void read_list(char *text, StringList *list) {
    char *token = strtok(text, ";");
    while (token != NULL) {
        string_add(list, token);
        token = strtok(NULL, ";");
    }
}

int read_user(char *line, User *u) {
    char *fields[5];
    int n = 0;
    char *p = line;
    fields[n++] = p;
    while (*p != '\0' && *p != '\n') {
        if (*p == '|') {
            if (n == 5) return 0;
            *p = '\0';
            fields[n++] = p + 1;
        }
        p++;
    }
    *p = '\0';
    if (n != 5 || fields[0][0] == '\0') return 0;

    copy_text(u->name, fields[0], sizeof(u->name));
    read_list(fields[1], &u->preferences.allergies);
    read_list(fields[2], &u->preferences.allowed_categories);
    read_list(fields[3], &u->preferences.favorite_categories);
    read_list(fields[4], &u->preferences.favorite_areas);
    return 1;
}

void save_list(FILE *file, StringList list) {
    for (int i = 0; i < list.count; i++) {
        if (i > 0) fprintf(file, ";");
        fprintf(file, "%s", list.items[i]);
    }
}

void save_user(FILE *file, const User *u) {
    fprintf(file, "%s|", u->name);
    save_list(file, u->preferences.allergies);
    fprintf(file, "|");
    save_list(file, u->preferences.allowed_categories);
    fprintf(file, "|");
    save_list(file, u->preferences.favorite_categories);
    fprintf(file, "|");
    save_list(file, u->preferences.favorite_areas);
    fprintf(file, "\n");
}

int read_users(const char *filename, User users[], int max) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return 0;
    char line[2000];
    int count = 0;
    while (count < max && fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '#' || line[0] == '\0') continue;
        if (read_user(line, &users[count])) count++;
    }
    fclose(file);
    return count;
}

void save_all_users(const char *filename, User users[], int count) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error: could not save profiles to '%s'\n", filename);
        return;
    }
    for (int i = 0; i < count; i++) {
        save_user(file, &users[i]);
    }
    fclose(file);
}

