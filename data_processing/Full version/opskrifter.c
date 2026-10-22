/* gcc -std=c11 -Wall -Wextra opskrifter.c cJSON.c -lm -o opskrifter */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#define INPUT "opskrifter.json"
#define OUTPUT "opskrifter_opdelt.json"

typedef struct { const char *name, *aliases; double factor, grams; } Unit;
static const Unit units[] = {
    {"kg", "kilograms|kilogram|kg", 1, 1000},
    {"g", "grams|gram|g", 1, 1},
    {"ml", "milliliters|milliliter|ml", 1, 1},
    {"l", "liters|liter|litres|litre|l", 1, 1000},
    {"dl", "dl", 1, 100}, {"cl", "cl", 1, 10},
    {"tablespoons", "tablespoons|tablespoon|tblsp|tbls|tbsp|tbs", 1, 15},
    {"teaspoons", "teaspoons|teaspoon|tsp", 1, 5},
    {"ml", "cups|cup", 240, 1},
    {"ml", "fluid ounces|fluid ounce|fl. oz|fl oz", 29.5735295625, 1},
    {"g", "ounces|ounce|oz", 28.349523125, 1},
    {"g", "pounds|pound|lbs|lb", 453.59237, 1},
    {"ml", "pints|pint|pt", 473.176473, 1},
    {"ml", "quarts|quart|qt", 946.352946, 1},
    {"cm", "centimeters|centimeter|centimetres|centimetre|cm", 1, 0},
    {"cm", "inches|inch|\"|″", 2.54, 0}
};
typedef struct {
    double amount;
    const Unit *unit;
    const char *rest, *end;
} Measure;
static FILE *out;

static void fail(const char *message) {
    fprintf(stderr, "%s\n", message);
    if (out) { fclose(out); remove(OUTPUT ".tmp"); }
    exit(EXIT_FAILURE);
}
static void *allocate(size_t n) {
    void *p = malloc(n);
    if (!p) fail("Ikke nok hukommelse.");
    return p;
}
static const char *spaces(const char *p) {
    while (isspace((unsigned char)*p)) p++;
    return p;
}
static size_t match(const char *p, const char *aliases) {
    while (*aliases) {
        size_t n = strcspn(aliases, "|"), i = 0;
        while (i < n && tolower((unsigned char)p[i]) == tolower((unsigned char)aliases[i])) i++;
        if (i == n && !isalpha((unsigned char)p[i])) return n;
        aliases += n;
        if (*aliases) aliases++;
    }
    return 0;
}
/* Kun Unicode-broeker, som forekommer i originalfilen. */
static int fraction(const char **p, double *x) {
    const char *symbols[] = {"½", "¼", "¾", "⅓", "⅛", "⅕"};
    const double values[] = {.5, .25, .75, 1./3, .125, .2};
    for (int i = 0; i < 6; i++) {
        size_t n = strlen(symbols[i]);
        if (!strncmp(*p, symbols[i], n)) { *p += n; *x = values[i]; return 1; }
    }
    char *a, *b;
    double numerator = strtod(*p, &a);
    size_t n = match(a, "/");
    if (a == *p || !n) return 0;
    double denominator = strtod(a + n, &b);
    if (b == a + n || denominator <= 0 || !isfinite(numerator) ||
        !isfinite(denominator) || !isfinite(numerator / denominator)) return 0;
    *x = numerator / denominator;
    if (*x < 0) return 0;
    *p = b;
    return 1;
}
static int single(const char **p, double *x) {
    const char *start = spaces(*p), *q = start;
    const char *words[] = {"one and a half", "a couple of", "half", "one", "two", "an"};
    const double values[] = {1.5, 2, .5, 1, 2, 1};
    for (int i = 0; i < 6; i++) {
        size_t n = match(start, words[i]);
        if (n) { *p = start + n; *x = values[i]; return 1; }
    }
    if (fraction(&q, x)) { *p = q; return 1; }
    if (!isdigit((unsigned char)*start) && *start != '.') return 0;
    char *end;
    *x = strtod(start, &end);
    if (end == start || !isfinite(*x) || *x < 0 || *end == '/') return 0;
    q = spaces(end);
    if (*q == '-') q = spaces(q + 1);
    double f;
    if (*x >= 1 && floor(*x) == *x && fraction(&q, &f) && f < 1) {
        *x += f; *p = q;
    } else *p = end;
    return 1;
}
static int number(const char **p, double *x) {
    if (!single(p, x)) return 0;
    const char *q = spaces(*p);
    size_t n = match(q, "-|–|to");
    double y;
    if (n) {
        q += n;
        if (single(&q, &y)) { *x = *x / 2 + y / 2; *p = q; }
    }
    return 1;
}
static double rounded(double x, const Unit *u) {
    double scale = u && (!strcmp(u->name, "g") || !strcmp(u->name, "ml")) ? 1 : 10;
    return round(x * scale) / scale;
}
static const Unit *read_unit(const char **p) {
    const char *q = spaces(*p);
    size_t n = match(q, "of an");
    if (n) q = spaces(q + n);
    if (*q == '-') q = spaces(q + 1);
    for (size_t i = 0; i < sizeof units / sizeof *units; i++) {
        size_t n = match(q, units[i].aliases);
        if (n) { *p = q + n; return units + i; }
    }
    return NULL;
}
static Measure measure(const char *raw) {
    Measure m = {-1, NULL, raw, raw};
    const char *p = raw;
    double x;
    if (!number(&p, &x)) return m;
    m.rest = spaces(p);
    for (int packed = 0; packed < 2; packed++) {
        const char *q = m.rest;
        double size = 1;
        if (packed && !number(&q, &size)) break;
        const Unit *u = read_unit(&q);
        if (u && isfinite(x * size * u->factor)) {
            m.amount = rounded(x * size * u->factor, u);
            m.unit = u; m.rest = spaces(q); m.end = q;
            return m;
        }
    }
    m.amount = rounded(x, NULL); m.end = p;
    return m;
}
/* Konverter ogsaa laengder som 1/8 inch og ½\". */
static char *instructions(const char *text) {
    size_t capacity = strlen(text) * 32 + 64;
    char *result = allocate(capacity), *w = result;
    for (const char *p = text; *p;) {
        int boundary = p == text || (!isalnum((unsigned char)p[-1]) && p[-1] != '.');
        if (p > text + 1 && p[-1] == '/' && isdigit((unsigned char)p[-2])) boundary = 0;
        if (boundary && !isspace((unsigned char)*p)) {
            Measure m = measure(p);
            if (m.unit) {
                w += sprintf(w, "%g %s", m.amount, m.unit->name);
                p = m.end; continue;
            }
        }
        *w++ = *p++;
    }
    *w = 0;
    return result;
}

static void examples(void) {
    const char *tests[] = {"2 tbsp", "½ tsp", "1 ½ cups", "2-3 tbsp", "2-1/2 cups",
        "8 oz", "2 kg", "2 cloves", "2 small packs", "3 400g Cans", "1 finely chopped",
        "2", "pinch", "to taste", "12.5 g", "24.6 ml", "1.25 kg", "0.5-0.75 tsp",
        "2 pint", "1 quart", "2 tbsp chopped", "⅛ inch"};
    for (size_t i = 0; i < sizeof tests / sizeof *tests; i++) {
        Measure m = measure(tests[i]);
        printf("%-20s -> amount: ", tests[i]);
        if (m.amount < 0) printf("null"); else printf("%g", m.amount);
        printf(", unit: %s, note: %s\n", m.unit ? m.unit->name : m.rest,
               m.unit && *m.rest ? m.rest : "-");
    }
    const char *text = "Use 1 oz tomato and 1/2 cup milk. Cut into ⅛-inch pieces. Use a 9-inch pan.";
    char *converted = instructions(text);
    printf("\nBefore: %s\nAfter:  %s\n\n", text, converted); free(converted);
}
/* Bevar originalmaalet; tilfoej amount, unit og note. */
static void field(cJSON *meal, const char *key, cJSON *item) {
    if (!item) fail("Ikke nok hukommelse.");
    cJSON_DeleteItemFromObjectCaseSensitive(meal, key);
    if (!cJSON_AddItemToObject(meal, key, item)) { cJSON_Delete(item); fail("JSON-fejl."); }
}
static void convert(cJSON *root) {
    cJSON *meals = cJSON_GetObjectItemCaseSensitive(root, "meals"), *meal;
    if (!cJSON_IsArray(meals)) fail("JSON mangler et meals-array.");
    cJSON_ArrayForEach(meal, meals) {
        if (!cJSON_IsObject(meal)) fail("En opskrift skal vaere et objekt.");
        double total = 0;
        for (int i = 1; i <= 20; i++) {
            char key[40]; snprintf(key, sizeof key, "strMeasure%d", i);
            cJSON *item = cJSON_GetObjectItemCaseSensitive(meal, key);
            if (!item) continue;
            if (!cJSON_IsString(item) && !cJSON_IsNull(item)) fail("Ugyldigt maalfelt.");
            Measure m = cJSON_IsString(item) ? measure(item->valuestring) : (Measure){-1, NULL, NULL, NULL};
            snprintf(key, sizeof key, "strMeasure%damount", i);
            field(meal, key, m.amount < 0 ? cJSON_CreateNull() : cJSON_CreateNumber(m.amount));
            snprintf(key, sizeof key, "strMeasure%dunit", i);
            const char *unit = m.unit ? m.unit->name : m.rest;
            field(meal, key, unit && *unit ? cJSON_CreateString(unit) : cJSON_CreateNull());
            snprintf(key, sizeof key, "strMeasure%dnote", i);
            char *note = m.unit && *m.rest ? instructions(m.rest) : NULL;
            field(meal, key, note ? cJSON_CreateString(note) : cJSON_CreateNull());
            free(note);
            if (m.unit) total += m.amount * m.unit->grams;
        }
        if (!isfinite(total)) fail("Portionsestimatet er for stort.");
        field(meal, "portions", cJSON_CreateNumber(round(total / 400)));
        cJSON *item = cJSON_GetObjectItemCaseSensitive(meal, "strInstructions");
        if (cJSON_IsString(item)) {
            char *text = instructions(item->valuestring);
            if (!cJSON_SetValuestring(item, text)) fail("Ikke nok hukommelse.");
            free(text);
        }
    }
}
int main(void) {
    examples();
    FILE *f = fopen(INPUT, "rb");
    if (!f) fail("Kunne ikke aabne " INPUT);
    if (fseek(f, 0, SEEK_END)) fail("Kunne ikke laese input.");
    long size = ftell(f); if (size < 0) fail("Ukendt filstoerrelse.");
    rewind(f); char *text = allocate((size_t)size + 1);
    if (fread(text, 1, (size_t)size, f) != (size_t)size) fail("Laesefejl.");
    text[size] = 0; fclose(f);
    cJSON *root = cJSON_ParseWithOpts(text, NULL, 1); free(text);
    if (!root) fail("Ugyldig JSON.");
    const char *ids = "|52772|52774|52829|52879|52933|52935|53017|53057|53067|53083|53093|53116|53308|53336|53342|53454|53490|53494|53535|";
    cJSON *meals = cJSON_GetObjectItem(root, "meals");
    for (int i = cJSON_GetArraySize(meals) - 1; i >= 0; i--) {
        char id[32]; snprintf(id, sizeof id, "|%s|", cJSON_GetStringValue(cJSON_GetObjectItem(cJSON_GetArrayItem(meals, i), "idMeal")));
        if (strstr(ids, id)) cJSON_DeleteItemFromArray(meals, i);
    }
    convert(root);
    char *result = cJSON_Print(root);
    if (!result) fail("Ikke nok hukommelse.");
    out = fopen(OUTPUT ".tmp", "wb"); if (!out) fail("Kunne ikke oprette output.");
    if (fputs(result, out) == EOF) fail("Skrivefejl.");
    if (fclose(out)) { out = NULL; remove(OUTPUT ".tmp"); fail("Skrivefejl."); }
    out = NULL; cJSON_free(result); cJSON_Delete(root);
#ifdef _WIN32
    remove(OUTPUT);
#endif
    if (rename(OUTPUT ".tmp", OUTPUT)) fail("Kunne ikke gemme outputfilen.");
    printf("Gemt som %s\n", OUTPUT);
    return 0;
}
