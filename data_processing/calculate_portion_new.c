/*
* Build gcc calculate_portions_new.c -o calculate_portion_new.exe -lm
 * Calculate estimated portions for every recipe.
 * The program reads recipes_simplified.json and writes it in recipes_with_portions.json.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "cJSON.h"
#include "cJSON.c"

#define INPUT_FILE "recipes_simplified.json"
#define OUTPUT_FILE "recipes_with_portions.json"

/* Read the complete contents of a file into a string. */
static char *read_json_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    long file_size;
    char *buffer;
    size_t bytes_read;

    if (file == NULL) {
        perror(filename);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);

    buffer = malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    bytes_read = fread(buffer, 1, (size_t)file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size) {
        free(buffer);
        return NULL;
    }

    buffer[file_size] = '\0';
    return buffer;
}

/* Estimate portions using only g and ml amounts */
static int estimate_portions(const cJSON *recipe)
{
    double total_amount = 0.0;
    const cJSON *ingredients =
        cJSON_GetObjectItemCaseSensitive(recipe, "ingredients");
    const cJSON *category =
        cJSON_GetObjectItemCaseSensitive(recipe, "category");
    const cJSON *ingredient;

    for (ingredient = ingredients ? ingredients->child : NULL;
         ingredient != NULL;
         ingredient = ingredient->next) {

        const cJSON *amount =
            cJSON_GetObjectItemCaseSensitive(ingredient, "amount");
        const cJSON *unit =
            cJSON_GetObjectItemCaseSensitive(ingredient, "unit");

        if (cJSON_IsNumber(amount) && cJSON_IsString(unit)) {
            if (strcmp(unit->valuestring, "g") == 0 ||
                strcmp(unit->valuestring, "ml") == 0) {
                total_amount += amount->valuedouble;
            }
        }
    }

    if (total_amount <= 0.0) {
        return 0;
    }

    if (cJSON_IsString(category) &&
        strcmp(category->valuestring, "Dessert") == 0) {
        return (int)fmax(1.0, round(total_amount / 100.0));
    }

    return (int)fmax(1.0, round(total_amount / 300.0));
}

/* Add or replace the portions property in each recipe. */
static int update_all_recipes(cJSON *recipes)
{
    cJSON *recipe;

    for (recipe = recipes->child; recipe != NULL; recipe = recipe->next) {
        int portions = estimate_portions(recipe);

        cJSON_DeleteItemFromObjectCaseSensitive(recipe, "portions");

        if (cJSON_AddNumberToObject(recipe, "portions", portions) == NULL) {
            return 0;
        }
    }

    return 1;
}

/* Write one recipe, keeping each ingredient object on a single line. */
static int write_recipe(FILE *output, const cJSON *recipe)
{
    const cJSON *property;
    int first_property = 1;

    fputs("    {\n", output);

    for (property = recipe->child; property != NULL; property = property->next) {
        char *value;

        if (!first_property) {
            fputs(",\n", output);
        }
        first_property = 0;

        fprintf(output, "      \"%s\": ", property->string);

        if (strcmp(property->string, "ingredients") == 0 &&
            cJSON_IsArray(property)) {

            const cJSON *ingredient;
            int first_ingredient = 1;

            fputs("[\n", output);

            for (ingredient = property->child;
                 ingredient != NULL;
                 ingredient = ingredient->next) {

                value = cJSON_PrintUnformatted(ingredient);
                if (value == NULL) {
                    return 0;
                }

                if (!first_ingredient) {
                    fputs(",\n", output);
                }

                fprintf(output, "        %s", value);
                first_ingredient = 0;
                free(value);
            }

            fputs("\n      ]", output);
        } else {
            value = cJSON_PrintUnformatted(property);
            if (value == NULL) {
                return 0;
            }

            fputs(value, output);
            free(value);
        }
    }

    fputs("\n    }", output);
    return !ferror(output);
}

/* Save all recipes in the expected JSON structure. */
static int save_json_file(const char *filename, const cJSON *recipes)
{
    FILE *output = fopen(filename, "wb");
    const cJSON *recipe;
    int first_recipe = 1;

    if (output == NULL) {
        perror(filename);
        return 0;
    }

    fputs("{\n  \"meals\": [\n", output);

    for (recipe = recipes->child; recipe != NULL; recipe = recipe->next) {
        if (!first_recipe) {
            fputs(",\n", output);
        }

        if (!write_recipe(output, recipe)) {
            fclose(output);
            return 0;
        }

        first_recipe = 0;
    }

    fputs("\n  ]\n}\n", output);

    if (ferror(output)) {
        fclose(output);
        return 0;
    }

    return fclose(output) == 0;
}

int main(void)
{
    char *json_text = read_json_file(INPUT_FILE);
    cJSON *root;
    cJSON *recipes;

    if (json_text == NULL) {
        fprintf(stderr, "Could not read input file.\n");
        return 1;
    }

    root = cJSON_Parse(json_text);
    free(json_text);

    if (root == NULL) {
        fprintf(stderr, "Invalid JSON.\n");
        return 1;
    }

    recipes = cJSON_GetObjectItemCaseSensitive(root, "meals");

    if (!cJSON_IsArray(recipes)) {
        fprintf(stderr, "The JSON file does not contain a meals array.\n");
        cJSON_Delete(root);
        return 1;
    }

    if (!update_all_recipes(recipes)) {
        fprintf(stderr, "Could not add portions to every recipe.\n");
        cJSON_Delete(root);
        return 1;
    }

    if (!save_json_file(OUTPUT_FILE, recipes)) {
        fprintf(stderr, "Could not write output file.\n");
        cJSON_Delete(root);
        return 1;
    }

    cJSON_Delete(root);
    printf("Saved %s\n", OUTPUT_FILE);
    return 0;
}