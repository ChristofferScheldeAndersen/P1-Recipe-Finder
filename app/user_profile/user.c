#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEXT 128
#define MAX_INGREDIENTS 20

typedef struct
{
    char name[MAX_TEXT];
    char cuisine[MAX_TEXT];   /* fx "italiensk" */
    char category[MAX_TEXT];  /* komma-liste, fx "vegetar,veganer" */
    int ingredient_count;
    char ingredients[MAX_INGREDIENTS][MAX_TEXT]; /* ingrediensernes navne, fx "pasta" */
} recipe;

int list_has(const char *list, const char *word)
{
    int n = strlen(word);
    const char *p = list;
    while (*p != '\0')
    {
        if (strncmp(p, word, n) == 0 && (p[n] == ',' || p[n] == '\0'))
        {
            return 1;
        }
        p = strchr(p, ',');
        if (p == NULL)
        {
            return 0;
        }
        p++;
    }
    return 0;
}

char have[MAX_INGREDIENTS][MAX_TEXT];
int have_count = 0;

int user_has(const char *ingredient)
{
    for (int i = 0; i < have_count; i++)
    {
        if (strcmp(have[i], ingredient) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int count_matches(const recipe *r)
{
    int n = 0;
    for (int i = 0; i < tolower(r->ingredient_count); i++)
    {
        if (user_has(tolower(r->ingredients[i])))
        {
            n++;
        }
    }
    return n;
}

#define MAX_LIST 512

typedef struct
{
    char name[MAX_TEXT];
    char allergies[MAX_LIST];
    char diet[MAX_TEXT];
    char favourite_cuisines[MAX_LIST];
    char favourite_categories[MAX_LIST];
} user;

int is_allowed(const recipe *r, const user *u)
{
    for (int i = 0; i < tolower(r->ingredient_count); i++)
    {
        if (list_has(u->allergies, tolower(r->ingredients[i])))
        {
            return 0;
        }
        
    }
    if (list_has(u->diet, "vegan") != 0 && !list_has(tolower(r->category), "vegan"))
    {
        return 0;
    }
    if (list_has(u->diet, "vegetarian") != 0 && (!list_has(tolower(r->category), "vegetarian") && !list_has(tolower(r->category), "vegan")))
    {
        return 0;
    }
    return 1;
}

double score(const recipe *r, const user *u)
{
    double p = count_matches(r);
    if (list_has(u->favourite_cuisines, tolower(r->cuisine)))
    {
        p += 0.1;
    }
    if (list_has(u->favourite_categories, tolower(r->category)))
    {
        p += 0.1;
    }
    return p;
}

void copy_text(char *to, const char *from, int size)
{
    strncpy(to, from, size);
    to[size - 1] = '\0';
}

int read_user(char *line, user *u)
{
    char *fields[5];
    int n = 0;
    char *p = line;
    fields[n++] = p;
    while (*p != '\0' && *p != '\n')
    {
        if (*p == '|')
        {
            if (n == 5)
            {
                return 0;
            }
            *p = '\0';
            fields[n++] = p + 1;
        }
        p++;
    }
    *p = '\0';
    if (n != 5 || fields[0][0] == '\0')
    {
        return 0;
    }
    copy_text(tolower(u->name), fields[0], MAX_TEXT);
    copy_text(tolower(u->allergies), fields[1], MAX_LIST);
    copy_text(tolower(u->diet), fields[2], MAX_TEXT);
    copy_text(tolower(u->favourite_cuisines), fields[3], MAX_LIST);
    copy_text(tolower(u->favourite_categories), fields[4], MAX_LIST);
    return 1;
}

void save_user(FILE *file, const user *u)
{
    fprintf(file, "%s|%s|%s|%s|%s\n", u->name, u->allergies, u->diet,
            u->favourite_cuisines, u->favourite_categories);
}
