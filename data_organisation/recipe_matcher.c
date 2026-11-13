/* Recipe matching for P1 Recipe Finder.
 * Requires cJSON: https://github.com/DaveGamble/cJSON
 * Linux: gcc recipe_matcher.c -o recipe_matcher -lcjson
 * Usage: ./recipe_matcher [recipes_with_portions.json]
 * Accepts either a root array or an object containing a "meals" array.
 * Ownership: RecipeResult.recipe points into database (do not free it separately).
 * matched_ingredients points to the caller's strings; free only its array.
 * Keep database and input strings alive until free_results() is called.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

typedef struct {
    const cJSON *recipe;
    int match_count;
    const char **matched_ingredients;
} RecipeResult;

/* Case-insensitive exact comparison; ignore leading/trailing spaces. */
static int same_ingredient(const char *a, const char *b) {
    if (!a || !b) return 0;
    while (isspace((unsigned char)*a)) a++;
    while (isspace((unsigned char)*b)) b++;
    size_t na = strlen(a), nb = strlen(b);
    while (na && isspace((unsigned char)a[na - 1])) na--;
    while (nb && isspace((unsigned char)b[nb - 1])) nb--;
    if (!na || na != nb) return 0;
    for (size_t i = 0; i < na; i++)
        if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return 0;
    return 1;
}

static int recipe_has_ingredient(const cJSON *recipe, const char *name) {
    const cJSON *items = cJSON_GetObjectItemCaseSensitive(recipe, "ingredients");
    if (!cJSON_IsArray(items)) return 0;
    const cJSON *item = NULL;
    cJSON_ArrayForEach(item, items) {
        const cJSON *field = cJSON_GetObjectItemCaseSensitive(item, "name");
        if (cJSON_IsString(field) && same_ingredient(field->valuestring, name)) return 1;
    }
    return 0;
}

static int compare_results(const void *a, const void *b) {
    const RecipeResult *x = a, *y = b;
    return (y->match_count > x->match_count) - (y->match_count < x->match_count);
}

void free_results(RecipeResult *results, int count) {
    if (!results) return;
    for (int i = 0; i < count; i++) free(results[i].matched_ingredients);
    free(results);
}

/* Returns up to n recipes; *out_count is the actual number returned.
 * On allocation failure returns NULL and sets *out_count = 0.
 * Input duplicates count only once, and no-match recipes are excluded.
 */
RecipeResult *find_recipes(const cJSON *database, const char *ingredients[],
                           int ingredient_count, int n, int *out_count) {
    if (!out_count) return NULL;
    *out_count = 0;
    if (!cJSON_IsArray(database) || !ingredients || ingredient_count <= 0 || n <= 0)
        return NULL;

    int recipe_count = cJSON_GetArraySize(database);
    RecipeResult *all = calloc((size_t)(recipe_count ? recipe_count : 1), sizeof(*all));
    if (!all) return NULL;
    int found = 0;

    const cJSON *recipe = NULL;
    cJSON_ArrayForEach(recipe, database) {
        if (!cJSON_IsObject(recipe)) continue;
        const char **matched = calloc((size_t)ingredient_count, sizeof(*matched));
        if (!matched) { free_results(all, found); return NULL; }
        int matches = 0;
        for (int i = 0; i < ingredient_count; i++) {
            if (!ingredients[i] || !recipe_has_ingredient(recipe, ingredients[i])) continue;
            int duplicate = 0;
            for (int j = 0; j < matches; j++)
                if (same_ingredient(matched[j], ingredients[i])) { duplicate = 1; break; }
            if (!duplicate) matched[matches++] = ingredients[i];
        }
        if (matches) all[found++] = (RecipeResult){recipe, matches, matched};
        else free(matched);
    }

    qsort(all, (size_t)found, sizeof(*all), compare_results);
    int keep = found < n ? found : n;
    for (int i = keep; i < found; i++) free(all[i].matched_ingredients);
    *out_count = keep;
    if (!keep) { free(all); return NULL; }
    return all;
}

static void run_test(const cJSON *database, const char *label,
                     const char *ingredients[], int ingredient_count, int n) {
    int count = 0;
    RecipeResult *results = find_recipes(database, ingredients, ingredient_count, n, &count);
    printf("\n%s (top %d, returned %d):\n", label, n, count);
    for (int i = 0; i < count; i++) {
        const cJSON *name = cJSON_GetObjectItemCaseSensitive(results[i].recipe, "name");
        printf("%d. %s | matches: %d | [", i + 1,
               cJSON_IsString(name) ? name->valuestring : "(unnamed)",
               results[i].match_count);
        for (int j = 0; j < results[i].match_count; j++)
            printf("%s%s", j ? ", " : "", results[i].matched_ingredients[j]);
        puts("]");
    }
    free_results(results, count);
}

int main(int argc, char **argv) {
    const char *example =
        "["
        "{\"name\":\"Pancakes\",\"ingredients\":[{\"name\":\"Egg\",\"amount\":2},{\"name\":\"Milk\",\"amount\":200},{\"name\":\"Flour\",\"amount\":150},{\"name\":\"Butter\",\"amount\":20}]},"
        "{\"name\":\"Omelette\",\"ingredients\":[{\"name\":\"Egg\",\"amount\":3},{\"name\":\"Milk\",\"amount\":30}]},"
        "{\"name\":\"Chocolate Cake\",\"ingredients\":[{\"name\":\"Flour\",\"amount\":200},{\"name\":\"Butter\",\"amount\":100},{\"name\":\"Chocolate\",\"amount\":200}]},"
        "{\"name\":\"Salad\",\"ingredients\":[{\"name\":\"Tomato\"},{\"name\":\"Lettuce\"}]}"
        "]";
    char *file_text = NULL;
    if (argc > 1) {
        FILE *f = fopen(argv[1], "rb");
        if (!f) { perror(argv[1]); return 1; }
        if (fseek(f, 0, SEEK_END) || ftell(f) < 0) { fclose(f); return 1; }
        long length = ftell(f);
        rewind(f);
        file_text = malloc((size_t)length + 1);
        if (!file_text) { fclose(f); return 1; }
        size_t read_bytes = fread(file_text, 1, (size_t)length, f);
        fclose(f);
        file_text[read_bytes] = '\0';
        if (read_bytes != (size_t)length) { free(file_text); return 1; }
    }
    cJSON *root = cJSON_Parse(file_text ? file_text : example);
    free(file_text);
    const cJSON *db = cJSON_IsArray(root)
        ? root : cJSON_GetObjectItemCaseSensitive(root, "meals");
    if (!cJSON_IsArray(db)) {
        fprintf(stderr, "Expected a JSON array or an object with a meals array.\n");
        cJSON_Delete(root);
        return 1;
    }
    const char *a[] = {"egg", "milk", "flour", "butter"};
    const char *b[] = {"tomato", "lettuce", "chocolate"};
    const char *c[] = {"  EGG ", "egg", "milk", "nothing"};
    run_test(db, "Test 1: basic sorting", a, 4, 3);
    run_test(db, "Test 2: different ingredients", b, 3, 3);
    run_test(db, "Test 3: duplicates, spaces and casing", c, 4, 2);
    cJSON_Delete(root);
    return 0;
}