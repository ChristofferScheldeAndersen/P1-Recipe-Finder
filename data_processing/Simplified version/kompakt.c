#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

static char *læs(const char *navn) {
    FILE *fil = fopen(navn, "rb");
    fseek(fil, 0, SEEK_END);
    long n = ftell(fil);
    rewind(fil);
    char *tekst = malloc((size_t)n + 1);
    size_t antal = fread(tekst, 1, (size_t)n, fil);
    tekst[antal] = '\0';
    fclose(fil);
    return tekst;
}

int main(void) {
    char *tekst = læs("opskrifter.json");
    char *ids = læs("fjern_ids.txt");
    cJSON *data = cJSON_Parse(tekst);
    free(tekst);
    cJSON *opskrifter = cJSON_GetObjectItem(data, "meals");

    for (int i = cJSON_GetArraySize(opskrifter) - 1; i >= 0; i--) {
        cJSON *opskrift = cJSON_GetArrayItem(opskrifter, i);
        const char *id = cJSON_GetStringValue(cJSON_GetObjectItem(opskrift, "idMeal"));
        char søg[40];
        snprintf(søg, sizeof søg, "|%s|", id);
        if (strstr(ids, søg)) {
            cJSON_DeleteItemFromArray(opskrifter, i);
        }
    }
    free(ids);

    cJSON *opskrift;
    cJSON_ArrayForEach(opskrift, opskrifter) {
        for (int i = 1; i <= 20; i++) {
            char navn[40];
            char *enhed;
            sprintf(navn, "strMeasure%d", i);
            const char *mål = cJSON_GetStringValue(cJSON_GetObjectItem(opskrift, navn));
            if (!mål) {
                mål = "";
            }
            double mængde = strtod(mål, &enhed);
            int har_tal = enhed != mål;
            while (isspace((unsigned char)*enhed)) {
                enhed++;
            }
            sprintf(navn, "strMeasure%damount", i);
            if (har_tal) {
                cJSON_AddNumberToObject(opskrift, navn, mængde);
            } else {
                cJSON_AddNullToObject(opskrift, navn);
            }
            sprintf(navn, "strMeasure%dunit", i);
            cJSON_AddStringToObject(opskrift, navn, enhed);

            sprintf(navn, "strMeasure%d", i);
            cJSON_DeleteItemFromObject(opskrift, navn);
        }
    }
    char *resultat = cJSON_PrintUnformatted(data);
    FILE *fil = fopen("opskrifter_opdelt.json", "w");
    fputs(resultat, fil);
    fclose(fil);
    printf("Beholdt: %d opskrifter.\n", cJSON_GetArraySize(opskrifter));
    cJSON_free(resultat);
    cJSON_Delete(data);
    return 0;
}
