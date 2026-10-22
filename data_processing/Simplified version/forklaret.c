/*
PSEUDOKODE

Læs opskrifter.json og fjern_ids.txt som tekst.
Omdan opskriftsteksten til en JSON-struktur.
Find listen "meals" i JSON-strukturen.

Gå baglæns gennem listen med opskrifter:
    Find opskriftens ID.
    Hvis ID'et står i fjern_ids.txt:
        Fjern opskriften.

For hver tilbageværende opskrift:
    For hvert målefelt fra strMeasure1 til strMeasure20:
        Hent målefeltets tekst; brug tom tekst ved manglende værdi.
        Læs et eventuelt tal fra tekstens begyndelse.
        Lad resten af teksten være enheden, inklusive eventuelle noter.
        Spring mellemrum før enheden over.
        Gem tallet i amount, eller null hvis der ikke blev læst et tal.
        Gem resten af teksten i unit.
        Slet det oprindelige målefelt.

Omdan JSON-strukturen til tekst uden indrykning.
Gem teksten i opskrifter_opdelt.json.
Vis antallet af beholdte opskrifter.
Frigiv den reserverede hukommelse.

FORUDSÆTNINGER

Dette er et øvelsesprogram uden fejlhåndtering. Filerne findes, JSON er
gyldig, og hukommelsesreservationerne lykkes. ID-listen er forberedt til
det medfølgende datasæt: Den fjerner 454 opskrifter og beholder 339.

Efter filtreringen indeholder målefelterne enten ét heltal/decimaltal
efterfulgt af tekst uden flere tal eller tekst helt uden tal. Derfor
behøver programmet hverken normalisering eller kontrol af særtilfælde.
Brøker og intervaller understøttes ikke. Opskrifter med disse formater
er på forhånd identificeret og fjernes via ID-listen.

Originalinputtet skal bruges ved hver kørsel. Outputtet overskrives.
Ingen enheder konverteres; ingen portioner beregnes; instructions bevares.
*/

#include <ctype.h>   // isspace: undersøger, om et tegn er et mellemrumstegn.
#include <stdio.h>   // Filfunktioner, sprintf, snprintf og printf.
#include <stdlib.h>  // malloc, free og strtod.
#include <string.h>  // strstr: søger efter en tekst inde i en anden tekst.
#include "cJSON.h"  // Bibliotek til at læse og ændre JSON.

/*
Læs en hel fil og returner en pointer til tekstens første tegn.
Funktionen reserverer hukommelse, som kalderen senere skal frigive.
static betyder her, at funktionen kun er tilgængelig i denne C-fil.
*/
static char *læs(const char *navn) {
    // Åbn filen til læsning. "rb" giver samme bytebaserede læsning på Windows og Linux.
    FILE *fil = fopen(navn, "rb");

    // Flyt læsepositionen til slutningen. Positionen fortæller filens størrelse i bytes.
    fseek(fil, 0, SEEK_END);
    long størrelse = ftell(fil);

    // Flyt tilbage til begyndelsen, så hele indholdet kan læses.
    rewind(fil);

    // Reservér plads til indholdet og et afsluttende nul-tegn.
    // size_t er typen til størrelser i bytes; castet omdanner long til size_t.
    char *tekst = malloc((size_t)størrelse + 1);

    // Læs bytes ind i den reserverede hukommelse. fread returnerer det læste antal.
    size_t antal = fread(tekst, 1, (size_t)størrelse, fil);

    // C-strenge skal slutte med '\0', så funktioner ved, hvor teksten stopper.
    tekst[antal] = '\0';

    // Filen lukkes, men teksten findes fortsat i den reserverede hukommelse.
    fclose(fil);
    return tekst;
}

int main(void) {
    // 1. Læs de to inputfiler.
    char *tekst = læs("opskrifter.json");
    char *ids = læs("fjern_ids.txt");

    // Parse bygger en JSON-struktur med egne kopier af tekstværdierne.
    // Derfor kan den oprindelige filtekst frigives med det samme.
    cJSON *data = cJSON_Parse(tekst);
    free(tekst);

    // Rodobjektets felt "meals" indeholder listen med opskrifter.
    cJSON *opskrifter = cJSON_GetObjectItem(data, "meals");

    // 2. Fjern opskrifterne fra ID-listen.
    // Vi går baglæns, fordi sletning flytter de efterfølgende elementers indeks.
    // På den måde springer vi ikke en opskrift over efter en sletning.
    for (int i = cJSON_GetArraySize(opskrifter) - 1; i >= 0; i--) {
        cJSON *opskrift = cJSON_GetArrayItem(opskrifter, i);

        // GetObjectItem finder JSON-feltet; GetStringValue henter dets tekst.
        const char *id = cJSON_GetStringValue(
            cJSON_GetObjectItem(opskrift, "idMeal")
        );

        // ID-filen indeholder fx |52772|. Afgrænsningen sikrer et helt ID-match.
        // snprintf skriver højst så meget, som der er plads til i søg.
        char søg[40];
        snprintf(søg, sizeof søg, "|%s|", id);

        // strstr returnerer en pointer ved fund og NULL, hvis teksten ikke findes.
        if (strstr(ids, søg)) {
            cJSON_DeleteItemFromArray(opskrifter, i);
        }
    }

    // ID-listen skal ikke bruges mere.
    free(ids);

    // 3. Opdel målene i hver tilbageværende opskrift.
    // cJSON_ArrayForEach er bibliotekets måde at gennemløbe en JSON-liste på.
    cJSON *opskrift;
    cJSON_ArrayForEach(opskrift, opskrifter) {
        // Kildedata har højst 20 ingrediensmål pr. opskrift.
        for (int i = 1; i <= 20; i++) {
            char navn[40];
            char *enhed;

            // Byg feltnavnet, fx "strMeasure2", ud fra tælleren i.
            sprintf(navn, "strMeasure%d", i);
            const char *mål = cJSON_GetStringValue(
                cJSON_GetObjectItem(opskrift, navn)
            );

            // Tomme og null-felter er almindelige data, ikke programfejl.
            // Brug en tom streng, hvis feltet ikke indeholder en streng.
            if (!mål) {
                mål = "";
            }

            /*
            strtod læser et tal og sætter enhed til resten af teksten.
            &enhed giver funktionen adressen på vores pointer, så den kan ændres.

            "1tbsp"    giver mængde = 1 og enhed pegende på "tbsp".
            "2 cloves" giver mængde = 2 og enhed pegende på " cloves".
            "to taste" giver mængde = 0 og enhed pegende på samme sted som mål.

            Derfor skal vi sammenligne pointerne for at skelne mellem tallet 0
            og tekst, der slet ikke starter med et tal.
            */
            double mængde = strtod(mål, &enhed);
            int har_tal = enhed != mål;

            // Fjern mellemrum før enheden ved at flytte pointeren frem.
            // Cast til unsigned char sikrer et gyldigt argument til isspace.
            while (isspace((unsigned char)*enhed)) {
                enhed++;
            }

            // Gem en numerisk mængde, eller JSON-værdien null ved ren tekst.
            sprintf(navn, "strMeasure%damount", i);
            if (har_tal) {
                cJSON_AddNumberToObject(opskrift, navn, mængde);
            } else {
                cJSON_AddNullToObject(opskrift, navn);
            }

            // Hele resten er unit; vi skelner ikke mellem enhed og note.
            // AddStringToObject laver sin egen kopi af teksten.
            sprintf(navn, "strMeasure%dunit", i);
            cJSON_AddStringToObject(opskrift, navn, enhed);

            // Nu er værdierne kopieret, så originalfeltet kan slettes.
            // Sletning tidligere ville gøre mål og enhed til ugyldige pointere.
            sprintf(navn, "strMeasure%d", i);
            cJSON_DeleteItemFromObject(opskrift, navn);
        }
    }

    // 4. Omdan strukturen til JSON-tekst uden ekstra indrykning og linjeskift.
    // Det ændrer tekstens layout, men ikke dens data.
    char *resultat = cJSON_PrintUnformatted(data);

    // "w" opretter filen eller overskriver en eksisterende fil.
    FILE *fil = fopen("opskrifter_opdelt.json", "w");
    fputs(resultat, fil);
    fclose(fil);

    // Vis, hvor mange opskrifter der er tilbage efter filtreringen.
    printf("Beholdt: %d opskrifter.\n", cJSON_GetArraySize(opskrifter));

    // 5. Frigiv både den genererede tekst og hele JSON-strukturen.
    cJSON_free(resultat);
    cJSON_Delete(data);

    return 0;
}
