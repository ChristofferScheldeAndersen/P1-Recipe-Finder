#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int compare_names(const void *a, const void *b)
{
    const char *name_a = *(const char **)a;
    const char *name_b = *(const char **)b;
    return strcmp(name_a, name_b);
}

int main(void)
{

    int c;
    size_t len = 0;          /* how many books are on the shelf right now */
    size_t cap = 4096;       /* how many books the shelf has room for */
    char *buf = malloc(cap); /* buf is the shelf itself (where the memory is) */

    if (buf == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }

    while ((c = getchar()) != EOF)
    {
        if (len + 1 >= cap)
        {
            size_t new_cap = cap * 2;
            char *tmp = realloc(buf, new_cap); /*realloc: in this case makes a new bigger block of memory*/

            if (tmp == NULL)
            {
                fprintf(stderr, "Out of memory\n");
                free(buf); /*frees the memory previously assigned by 'malloc'*/
                return 1;
            }
            buf = tmp;
            cap = new_cap;
        } /*The *tmp variable points to the new bigger memory, without it the old memory would be overwritten with NULL*/

        buf[len] = (char)c; /* put the character in the next empty box */
        len++;              /* move "next empty box" one step forward */
    }

    buf[len] = '\0';

    if (strstr(buf, "\"strCategory\"") == NULL || strstr(buf, "\"meals\"") == NULL)
    {
        fprintf(stderr, "Could not find what you are looking for\n");
        free(buf);
        return 1;
    }
    /* Sanity check: the input must contain a "meals" array and at least one
    "strCategory" field. If either is missing, the file is not in the
    expected format, so clean up and exit with an error. */
    char *p = buf;

    size_t count = 0;                                /* how many categories are stored */
    size_t list_cap = 256;                           /* how many categories there's room for */
    char **list = malloc(list_cap * sizeof(char *)); /*char **list is a pointer to boxes that each hold a char *, i.e. a pointer to a string.*/

    if (list == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        free(buf);
        return 1;
    }

    /* find the next category key; stop when there are no more.
       The closing quote is part of the search, so only the exact key
       "strCategory" matches (each meal has exactly one). */
    while ((p = strstr(p, "\"strCategory\"")) != NULL)
    {
        p = strchr(p, ':'); /* move p to the colon after the key */
        if (p == NULL)      /* no colon found, so there's nothing more to read */
        {
            break;
        }
        p++; /* step past the colon */

        while (*p == ' ') /* skip spaces between the colon and the value */
        {
            p++;
        }
        if (*p == 'n') /* value is null, so there's no category here */
        {
            continue;
        }
        if (*p == '"') /* value is a string; step past the opening quote */
        {
            p++;
        }
        else /* unexpected value, so skip it */
        {
            continue;
        }

        char *start = p;                /* start points at the first character of the category */
        char *end = strchr(start, '"'); /* end points at the closing quote, just after the last character */

        if (end == NULL) /* no closing quote found, so stop reading */
        {
            break;
        }

        p = end + 1; /* continue searching after this value */

        while (start < end && isspace((unsigned char)*start)) /*Checks if there is a space written in front of the category e.g " Beef"*/
        {
            start++; /*Moves start one spot to the right if the character is a space*/
        }
        while (start < end && isspace((unsigned char)*(end - 1))) /*Checks if there is a space written at the end of the category e.g "Beef "*/
        {
            end--; /*Moves end one spot to the left if the character is a space*/
        }

        int length = end - start; /* number of characters in the category */
        if (length == 0)          /* the category is empty, e.g. "" */
        {
            continue;
        }

        char *copy = malloc(length + 1);

        if (copy == NULL)
        {
            fprintf(stderr, "Out of memory\n");
            free(list);
            free(buf);
            return 1;
        }

        for (int i = 0; i < length; i++)
        {
            copy[i] = (char)tolower((unsigned char)start[i]);
        }
        
        copy[length] = '\0';
        list[count] = copy;
        count++;

        if (count == list_cap)
        {
            size_t new_cap = list_cap * 2;
            char **tmp = realloc(list, new_cap * sizeof(char *)); /*realloc: in this case makes a new bigger block of memory*/

            if (tmp == NULL)
            {
                fprintf(stderr, "Out of memory\n");
                free(list);
                free(buf); /*frees the memory previously assigned by 'malloc'*/
                return 1;
            }
            list = tmp;
            list_cap = new_cap;
        } 
    }

    qsort(list, count, sizeof(char *), compare_names);

    fprintf(stderr, "%zu categories stored\n", count);

    size_t unique = 0;

    for (size_t i = 0; i < count; i++)
    {
        if (i == 0 || strcmp(list[i], list[i-1]) != 0)
        {
            printf("%s\n", list[i]);
            unique++;
        }
    }

    fprintf(stderr, "%zu unique categories\n", unique);

    for (size_t i = 0; i < count; i++)
    {
        free(list[i]);
    }
    free(list);
    free(buf);

    return 0;
}