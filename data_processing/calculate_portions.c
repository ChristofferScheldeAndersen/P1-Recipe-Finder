/* Add estimated portions while keeping one ingredient per line.
   Compile: gcc calculate_portions.c -o calculate_portions -lm */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cJSON.h"

#define INPUT "recipes_simplified.json"
#define OUTPUT "recipes_with_portions.json"

static void write_meal(FILE *f, const cJSON *meal) {
    fputs("    {\n", f);
    int first = 1;
    for (const cJSON *field = meal->child; field; field = field->next) {
        if (!first) fputs(",\n", f);
        first = 0;
        fprintf(f, "      \"%s\": ", field->string);
        if (!strcmp(field->string, "ingredients") && cJSON_IsArray(field)) {
            fputs("[\n", f);
            int first_ingredient = 1;
            for (const cJSON *ing = field->child; ing; ing = ing->next) {
                char *s = cJSON_PrintUnformatted(ing);
                if (!first_ingredient) fputs(",\n", f);
                fprintf(f, "        %s", s);
                first_ingredient = 0;
                free(s);
            }
            fputs("\n      ]", f);
        } else {
            char *s = cJSON_PrintUnformatted(field);
            fputs(s, f);
            free(s);
        }
    }
    fputs("\n    }", f);
}

int main(void) {
    FILE *f = fopen(INPUT, "rb");
    if (!f) { perror(INPUT); return 1; }
    if (fseek(f, 0, SEEK_END)) { fclose(f); return 1; }
    long size = ftell(f);
    if (size < 0) { fclose(f); return 1; }
    rewind(f);

    char *text = malloc((size_t)size + 1);
    if (!text) { fclose(f); return 1; }
    size_t read_size = fread(text, 1, (size_t)size, f);
    fclose(f);
    if (read_size != (size_t)size) { free(text); return 1; }
    text[size] = '\0';

    cJSON *root = cJSON_Parse(text);
    free(text);
    if (!root) { fprintf(stderr, "Invalid JSON\n"); return 1; }
    cJSON *meals = cJSON_GetObjectItemCaseSensitive(root, "meals");
    if (!cJSON_IsArray(meals)) {
        fprintf(stderr, "Missing meals array\n");
        cJSON_Delete(root);
        return 1;
    }

    for (cJSON *meal = meals->child; meal; meal = meal->next) {
        double total = 0;
        cJSON *ingredients = cJSON_GetObjectItemCaseSensitive(meal, "ingredients");
        for (cJSON *ing = ingredients ? ingredients->child : NULL; ing; ing = ing->next) {
            cJSON *amount = cJSON_GetObjectItemCaseSensitive(ing, "amount");
            cJSON *unit = cJSON_GetObjectItemCaseSensitive(ing, "unit");
            if (cJSON_IsNumber(amount) && cJSON_IsString(unit) &&
                (!strcmp(unit->valuestring, "g") || !strcmp(unit->valuestring, "ml")))
                total += amount->valuedouble;
        }
        cJSON *category = cJSON_GetObjectItemCaseSensitive(meal, "category");
        int dessert = cJSON_IsString(category) && !strcmp(category->valuestring, "Dessert");
        int portions = total > 0 ? (int)fmax(1, round(total / (dessert ? 100 : 300))) : 0;
        cJSON_DeleteItemFromObjectCaseSensitive(meal, "portions");
        cJSON_AddNumberToObject(meal, "portions", portions);
    }

    f = fopen(OUTPUT, "wb");
    if (!f) { perror(OUTPUT); cJSON_Delete(root); return 1; }
    fputs("{\n  \"meals\": [\n", f);
    int first = 1;
    for (const cJSON *meal = meals->child; meal; meal = meal->next) {
        if (!first) fputs(",\n", f);
        write_meal(f, meal);
        first = 0;
    }
    fputs("\n  ]\n}\n", f);
    int error = ferror(f);
    if (fclose(f) || error) {
        fprintf(stderr, "Failed to write output\n");
        cJSON_Delete(root);
        return 1;
    }
    cJSON_Delete(root);
    printf("Saved %s\n", OUTPUT);
    return 0;
}

#include "cJSON.c"