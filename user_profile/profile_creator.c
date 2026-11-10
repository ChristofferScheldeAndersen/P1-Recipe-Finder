#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
