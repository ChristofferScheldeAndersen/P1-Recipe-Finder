#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    FILE *file = fopen("ingredients.txt", "r");
    if (file == NULL){
        printf("Missing ingredient file\n");
        return 1;
    } 

    char *line = NULL;
    size_t size = 0;
    char *matching_ingredients[1000];
    int index = 0;

    while (getline(&line, &size, file) != -1) {
        if (strstr(line, "tomato")) {
            matching_ingredients[index] = strdup(line);
            index++;
        }
    }

    for (int i = 0; i < index; i++){
        printf("%s", matching_ingredients[i]);
    }

    free(line);
    fclose(file);
    return 0;
}