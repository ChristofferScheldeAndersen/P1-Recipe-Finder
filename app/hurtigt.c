#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "UI.H"

// Struct for storing a list of strings

// Add a string to the list
void string_add(StringList *list, const char *text) {

    char **new_items = realloc(
        list->items,
        (list->count + 1) * sizeof(char *)
    );

    if (new_items == NULL) {
        return;
    }

    list->items = new_items;

    list->items[list->count] = malloc(strlen(text) + 1);

    if (list->items[list->count] == NULL) {
        return;
    }

    strcpy(list->items[list->count], text);

    list->count++;
}


// Free the memory used by the list
void string_free(StringList list) {

    for (int i = 0; i < list.count; i++) {
        free(list.items[i]);
    }

    free(list.items);
}


/* Function to compare two strings without case sensitivity */
int small_letters(const char* s, const char *t) {

    while (*s && *t) {
        if (tolower((unsigned char)*s) !=
            tolower((unsigned char)*t)) {
            return 0;
        }

        s++;
        t++;
    }

    return *s == '\0' && *t == '\0';
}


// Search for ingredients in the text file
StringList search_ingredients(const char *lookup_string) {

    StringList matches = {0};

    if (lookup_string == NULL || lookup_string[0] == '\0') {
        return matches;
    }

    FILE *file = fopen("ingredients.txt", "r");

    if (file == NULL) {
        printf("Could not open ingredients.txt\n");
        return matches;
    }

    char line[256];

    while (fgets(line, sizeof(line), file) != NULL) {

        line[strcspn(line, "\r\n")] = '\0';

        if (strstr(line, lookup_string) != NULL) {
            string_add(&matches, line);
        }
    }

    fclose(file);

    return matches;
}


