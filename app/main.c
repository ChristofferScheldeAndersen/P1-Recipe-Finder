#include <stdio.h>

int main(int argc, char **argv) {
    char ingredienser [1000];
    
    printf("=== RECIPE FINDER ===\n");
    printf("Skriv dine ingredienser: ");

    scanf("%998[^\n]", ingredienser);

    printf("Du har indtastet dette: %s\n", ingredienser);

    return 0;

    
}
