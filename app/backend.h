#ifndef BACKEND_H
#define BACKEND_H

#include <stdio.h>
#include "cJSON.h"

typedef struct {
    char **items;
    int count;
} StringList;

typedef struct {
    StringList allergies;
    StringList allowed_categories;
    StringList favorite_categories;
    StringList favorite_areas;
} UserPreferences;

typedef struct {
    char name[128];
    UserPreferences preferences;
} User;

typedef struct {
    cJSON *recipe;
    float score;
    StringList matching;
} RecipeResult;

// String functions
int string_compare(const char *a, const char *b);
int string_compare_n(const char *a, const char *b, size_t n);
char *string_duplicate(const char *text);
void string_add(StringList *list, const char *text);
void string_free(StringList list);
int list_contains(StringList strings, const char *test_string);
StringList string_copy(StringList original);
UserPreferences preferences_copy(UserPreferences original);
void preferences_free(UserPreferences preferences);

// Ingredient functions
int word_count(const char *text);
int compare_ingredients(const void *a, const void *b);
StringList search_ingredients(const char *lookup_string);
int suggest_ingredient(const char *ingredient, const char *lookup);

// Recipe functions
cJSON *load_recipes(const char *filename);
int compare_options(const void *a, const void *b);
StringList recipe_options(cJSON *meals, const char *field);
cJSON *filter_allowed_recipes(cJSON *all_recipes, UserPreferences preferences);
void choose_best_recipes(cJSON *meals, StringList chosen, UserPreferences preferences, int number, RecipeResult best[]);

// User profile functions
void copy_text(char *to, const char *from, int size);
void read_list(char *text, StringList *list);
int read_user(char *line, User *u);
void save_list(FILE *file, StringList list);
void save_user(FILE *file, const User *u);
int read_users(const char *filename, User users[], int max);
void save_all_users(const char *filename, User users[], int count);

#endif
