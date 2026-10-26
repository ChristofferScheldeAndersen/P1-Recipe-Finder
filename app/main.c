#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char ingredients[20][30]; 
    int i = 0;
    
    printf("\n=== RECIPE FINDER ===\n");
    printf("Input your ingredients and type 'D' when done:\n");
    
    while (i < 20) { //while loop, reads max 20 ingredients
        scanf(" %s", &ingredients[i]);
        if (strcmp(ingredients[i], "D") == 0) { //if statement, checks if user input is 'D'
            printf("\nYou entered:\n");
            int n = 0;
            while (n < i) { 
                //while loop, prints all parts of the array except 'D'
                //REMEMBER: use "< i" as to avoid including 'D' in the output
                printf("%s\n", ingredients[n]); 
                n++;
            }
            break;
        }
        i++;
    }

    return 0;
}
