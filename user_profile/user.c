#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEXT 128
#define MAX_INGREDIENTS 20

typedef struct
{
    char name[MAX_TEXT];
    char cuisine[MAX_TEXT];
    char category[MAX_TEXT];
    int ingredient_count;
    char ingredients[MAX_INGREDIENTS][MAX_TEXT];
} recipe;

void to_lower(char *s)
{
    for (char *p = s; *p != '\0'; p++)
    {
        *p = tolower((unsigned char)*p);
    }
}

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
    for (int i = 0; i < r->ingredient_count; i++)
    {
        if (user_has(r->ingredients[i]))
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
    for (int i = 0; i < r->ingredient_count; i++)
    {
        if (list_has(u->allergies, r->ingredients[i]))
        {
            return 0;
        }
    }
    if (list_has(u->diet, "vegan") && !list_has(r->category, "vegan"))
    {
        return 0;
    }
    if (list_has(u->diet, "vegetarian") && (!list_has(r->category, "vegetarian") && !list_has(r->category, "vegan")))
    {
        return 0;
    }
    return 1;
}

double score(const recipe *r, const user *u)
{
    double p = count_matches(r);
    if (list_has(u->favourite_cuisines, r->cuisine))
    {
        p += 0.1;
    }
    if (list_has(u->favourite_categories, r->category))
    {
        p += 0.1;
    }
    return p;
}

void copy_text(char *to, const char *from, int size)
{
    strncpy(to, from, size - 1);
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
    copy_text(u->name, fields[0], MAX_TEXT);
    copy_text(u->allergies, fields[1], MAX_LIST);
    copy_text(u->diet, fields[2], MAX_TEXT);
    copy_text(u->favourite_cuisines, fields[3], MAX_LIST);
    copy_text(u->favourite_categories, fields[4], MAX_LIST);
    to_lower(u->allergies);
    to_lower(u->diet);
    to_lower(u->favourite_cuisines);
    to_lower(u->favourite_categories);
    return 1;
}

void save_user(FILE *file, const user *u)
{
    fprintf(file, "%s|%s|%s|%s|%s\n", u->name, u->allergies, u->diet,
            u->favourite_cuisines, u->favourite_categories);
}

int read_users(const char *filename, user users[], int max)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        return 0;
    }
    char line[2000];
    int count = 0;
    while (count < max && fgets(line, sizeof(line), file) != NULL)
    {
        if (line[0] == '#' || line[0] == '\n')
        {
            continue;
        }
        if (read_user(line, &users[count]))
        {
            count++;
        }
    }
    fclose(file);
    return count;
}

void save_all_users(const char *filename, user users[], int count)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error: could not save profiles to '%s'\n", filename);
        return;
    }
    for (int i = 0; i < count; i++)
    {
        save_user(file, &users[i]);
    }
    fclose(file);
}

void ask(const char *question, char *answer, int size)
{
    printf("%s", question);
    if (fgets(answer, size, stdin) == NULL)
    {
        answer[0] = '\0';
        return;
    }
    answer[strcspn(answer, "\n")] = '\0';
}

void create_user(user *u)
{
    do
    {
        ask("Name: ", u->name, MAX_TEXT);
    } while (u->name[0] == '\0');
    
    ask("Allergies (comma separated, empty = none): ", u->allergies, MAX_LIST);
    ask("Diet (vegan / vegetarian / empty): ", u->diet, MAX_TEXT);
    ask("Favourite cuisines (comma separated) (no space): ", u->favourite_cuisines, MAX_LIST);
    ask("Favourite categories (comma separated) (no space): ", u->favourite_categories, MAX_LIST);
    to_lower(u->allergies);
    to_lower(u->diet);
    to_lower(u->favourite_cuisines);
    to_lower(u->favourite_categories);
}

void print_users(const user users[], int count)
{
    for (int i = 0; i < count; i++)
    {
        printf("%d. %s | allergies: %s | diet: %s | cuisines: %s | categories: %s\n",
               i + 1, users[i].name, users[i].allergies, users[i].diet,
               users[i].favourite_cuisines, users[i].favourite_categories);
    }
}

int main(void)
{
    user users[20];
    int count = read_users("brugere.txt", users, 20);
    printf("Read %d profiles from brugere.txt\n", count);
    print_users(users, count);

    printf("\nCreate a new profile:\n");
    create_user(&users[count]);
    count++;

    save_all_users("brugere.txt", users, count);
    printf("\nSaved %d profiles. Reading the file again:\n", count);

    user check[20];
    int check_count = read_users("brugere.txt", check, 20);
    printf("Read %d profiles from brugere.txt\n", check_count);
    print_users(check, check_count);
    return 0;
}