#include <stdio.h>

int main(int argc, char *argv) {
    char ingredins [1000];
    
    printf("=== RECIPE FINDER ===\n");
    printf("Skrive denne ingredienser: ");

    scanf("%999[^\n]", ingredins);

    printf("Du har indtastet: %s\n", ingredins);

    return 0;

    
}