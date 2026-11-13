/* indlaes.c - laeser opskrifterne fra opskrifter_normaliseret2.json.
 *
 * Vi bruger ikke et rigtigt JSON-bibliotek. Filen har eet felt per linje, fx
 *       "strIngredient1": "Raw king prawns",
 *       "amount1": 500,
 * saa vi laeser linje for linje, finder navnet (noeglen) og vaerdien, og gemmer
 * de felter vi skal bruge. Et nyt "idMeal" betyder at en ny opskrift starter.
 */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "madplan.h"

#define LINJE 20000   /* den laengste fremgangsmaade er ca. 10.000 tegn */

/* Kopier en JSON-tekst (startende efter det foerste ") over i ud og oversaet \n, \" osv.
 * Data indeholder ingen \uXXXX, saa dem springer vi over. */
static void laes_tekst(const char *p, char *ud, size_t plads) {
    size_t n = 0;
    while (*p && *p != '"' && n + 1 < plads) {
        if (*p == '\\' && p[1]) {
            p++;
            if (*p == 'n') ud[n++] = '\n';
            else if (*p == 't') ud[n++] = ' ';
            else if (*p == 'r') { /* \r springes over */ }
            else ud[n++] = *p;                     /* \" \\ \/ */
        } else {
            ud[n++] = *p;
        }
        p++;
    }
    ud[n] = '\0';
}

/* Kopier fra over i til, men hoejst plads-1 tegn (lange tekster skaeres af) */
static void kopier(char *til, size_t plads, const char *fra) {
    size_t n = strlen(fra);
    if (n >= plads) n = plads - 1;
    memcpy(til, fra, n);
    til[n] = '\0';
}

/* Hvis noeglen starter med prefix og slutter med et tal (fx "amount12"), returner tallet, ellers 0 */
static int felt_nr(const char *noegle, const char *prefix) {
    size_t l = strlen(prefix);
    if (strncmp(noegle, prefix, l) != 0) return 0;
    int nr = atoi(noegle + l);
    return (nr >= 1 && nr <= MAX_INGREDIENSER) ? nr : 0;
}

/* Naar en opskrift er faerdiglaest: fjern tomme ingredienser (TheMealDB har "" og null i slutningen) */
static void ryd_op(Opskrift *o) {
    int n = 0;
    for (int i = 0; i < MAX_INGREDIENSER; i++)
        if (o->ing[i].navn[0] != '\0')
            o->ing[n++] = o->ing[i];
    o->antal = n;
}

int indlaes_opskrifter(const char *filnavn, Opskrift *opskrifter, int max) {
    FILE *f = fopen(filnavn, "r");
    if (!f) return -1;

    static char linje[LINJE], vaerdi[LINJE];
    int n = 0;                       /* antal opskrifter indtil nu */
    Opskrift *o = NULL;              /* den opskrift vi er i gang med */

    while (fgets(linje, sizeof linje, f)) {
        /* Find  "noegle": vaerdi */
        char *p = linje;
        while (*p == ' ') p++;
        if (*p != '"') continue;                 /* linjer som {  }  ] springes over */
        char noegle[KORT];
        char *slut = strchr(p + 1, '"');
        if (!slut || slut - p - 1 >= KORT) continue;
        memcpy(noegle, p + 1, slut - p - 1);
        noegle[slut - p - 1] = '\0';
        p = slut + 1;
        while (*p == ':' || *p == ' ') p++;      /* nu peger p paa vaerdien */

        int er_tekst = (*p == '"');
        int er_null  = (strncmp(p, "null", 4) == 0);
        if (er_tekst) laes_tekst(p + 1, vaerdi, sizeof vaerdi);
        else vaerdi[0] = '\0';

        if (strcmp(noegle, "idMeal") == 0) {     /* ny opskrift */
            if (o) ryd_op(o);
            if (n == max) break;
            o = &opskrifter[n++];
            memset(o, 0, sizeof *o);
            continue;
        }
        if (!o || er_null) continue;

        int nr;
        if (strcmp(noegle, "strMeal") == 0)
            kopier(o->navn, sizeof o->navn, vaerdi);
        else if (strcmp(noegle, "strCategory") == 0)
            kopier(o->kategori, KORT, vaerdi);
        else if (strcmp(noegle, "strArea") == 0)
            kopier(o->land, KORT, vaerdi);
        else if (strcmp(noegle, "strInstructions") == 0)
            o->fremgangsmaade = strdup(vaerdi);
        else if ((nr = felt_nr(noegle, "strIngredient"))) {
            kopier(o->ing[nr - 1].navn, KORT, vaerdi);
            trim(o->ing[nr - 1].navn);
        }
        else if ((nr = felt_nr(noegle, "strMeasure")))
            kopier(o->ing[nr - 1].maal, KORT, vaerdi);
        else if ((nr = felt_nr(noegle, "amount"))) {
            o->ing[nr - 1].har_maengde = 1;
            o->ing[nr - 1].maengde = atof(p);
        }
        else if ((nr = felt_nr(noegle, "unit")))
            kopier(o->ing[nr - 1].enhed, KORT, vaerdi);
        else if ((nr = felt_nr(noegle, "note")))
            kopier(o->ing[nr - 1].note, KORT, vaerdi);
    }
    if (o) ryd_op(o);
    fclose(f);
    return n;
}

/* qsort-sammenligning: alfabetisk, uden at skelne store og smaa bogstaver */
static int alfabetisk(const void *a, const void *b) {
    return strcasecmp((const char *)a, (const char *)b);
}

/* Saml alle forskellige ingrediensnavne fra opskrifterne i navne[], sorteret alfabetisk.
 * Det er listen man vaelger fra, naar man skriver hvad man har i koeleskabet. */
int lav_ingrediensliste(const Opskrift *opskrifter, int n, char navne[][KORT], int max) {
    int antal = 0;
    for (int r = 0; r < n; r++) {
        for (int i = 0; i < opskrifter[r].antal; i++) {
            const char *navn = opskrifter[r].ing[i].navn;
            int findes = 0;
            for (int k = 0; k < antal && !findes; k++)
                if (ens(navne[k], navn)) findes = 1;
            if (!findes && antal < max)
                snprintf(navne[antal++], KORT, "%s", navn);
        }
    }
    qsort(navne, antal, KORT, alfabetisk);
    return antal;
}
