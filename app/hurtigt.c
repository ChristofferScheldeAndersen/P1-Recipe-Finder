#include <stdio.h>
#include <ctype.h>



/*funktion til at gøre alle bogstaver små inden man sammenligner*/
int small_letters(const char* s, const char *t)
{
    while (*s && *t){
        if (tolower((unsigned char)*s)!= tolower((unsigned char)*t)){
            return 0;
        }
    s++;
    t++;
    }
return *s == '\0' && *t == '\0';
}

int main (void) {
small_letters("APPLE", "apple");
printf("No problem");
return 0;
}
