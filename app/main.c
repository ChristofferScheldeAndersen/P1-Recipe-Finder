#include <stdio.h>

int main(int argc, char **argv) {
    char ingredienser [1000];
    
    printf("=== RECIPE FINDER ===\n");
    printf("Skrive denne ingredienser: ");

    scanf("%999[^\n]", ingredienser);

    printf("Du har indtastet dette: %s\n", ingredienser);

    return 0;

    
}
