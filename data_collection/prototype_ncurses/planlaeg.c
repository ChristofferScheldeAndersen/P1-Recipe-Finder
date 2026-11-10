/* planlaeg.c - forslag ud fra koeleskabet, skalering og indkoebsliste.
 * Filen skriver intet paa skaermen; den regner kun. Det er skaerm.c der viser resultaterne. */
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "madplan.h"

/* Ting vi antager at alle har derhjemme, saa de ikke kommer paa indkoebslisten */
static const char *basisvarer[] = {"salt", "sea salt", "pepper", "black pepper", "water",
                                   "salt and pepper", "ice", 0};

int er_basisvare(const char *ingrediens) {
    for (int i = 0; basisvarer[i]; i++)
        if (ens(ingrediens, basisvarer[i])) return 1;
    return 0;
}

/* Har brugeren ingrediensen? "onion" i koeleskabet daekker "Chopped onion" og "Red Onions". */
int har_ingrediens(const Ordliste *koeleskab, const char *ingrediens) {
    if (er_basisvare(ingrediens)) return 1;
    for (int i = 0; i < koeleskab->antal; i++)
        if (indeholder_ord(ingrediens, koeleskab->ord[i])) return 1;
    return 0;
}

/* Hvor mange af opskriftens ingredienser har brugeren (basisvarer taeller ikke med her)? */
static int antal_fra_koeleskab(const Opskrift *o, const Ordliste *koeleskab) {
    int n = 0;
    for (int i = 0; i < o->antal; i++)
        if (!er_basisvare(o->ing[i].navn) && har_ingrediens(koeleskab, o->ing[i].navn)) n++;
    return n;
}

static int antal_mangler(const Opskrift *o, const Ordliste *koeleskab) {
    int n = 0;
    for (int i = 0; i < o->antal; i++)
        if (!har_ingrediens(koeleskab, o->ing[i].navn)) n++;
    return n;
}

/* Bruges af qsort: faerrest manglende foerst, og ved lighed flest ting fra koeleskabet foerst */
static int sammenlign(const void *a, const void *b) {
    const Forslag *x = a, *y = b;
    if (x->mangler != y->mangler) return x->mangler - y->mangler;
    return y->har - x->har;
}

/* Find de bedste opskrifter: tilladt efter praeferencerne og bruger mindst een ting fra
 * koeleskabet. De bedste max forslag laegges i resultat[]; returnerer hvor mange. */
int foreslaa(const Opskrift *opskrifter, int n, const Valg *v, Forslag *resultat, int max) {
    static Forslag alle[MAX_OPSKRIFTER];
    int antal = 0;
    for (int i = 0; i < n; i++) {
        const Opskrift *o = &opskrifter[i];
        int har = antal_fra_koeleskab(o, &v->koeleskab);
        if (har == 0 || !opskrift_tilladt(o, v)) continue;
        alle[antal].nr = i;
        alle[antal].har = har;
        alle[antal].mangler = antal_mangler(o, &v->koeleskab);
        antal++;
    }
    qsort(alle, antal, sizeof alle[0], sammenlign);
    if (antal > max) antal = max;
    memcpy(resultat, alle, antal * sizeof alle[0]);
    return antal;
}

/* Skaler en maengde og rund den paent af: 1 decimal under 10, ellers hele tal */
static double skaler(double maengde, int personer) {
    double x = maengde * personer / BASIS_PERSONER;
    return x < 10 ? round(x * 10) / 10 : round(x);
}

/* Lav maengden for ingrediensen til personer personer som tekst, fx "750 g" eller "1.5 cloves" */
void maengde_tekst(const Ingrediens *ing, int personer, char *buf, int plads) {
    if (ing->har_maengde)
        snprintf(buf, plads, "%g%s%s%s%s", skaler(ing->maengde, personer),
                 ing->enhed[0] ? " " : "", ing->enhed, ing->note[0] ? " " : "", ing->note);
    else
        snprintf(buf, plads, "%s", ing->maal);   /* fx "pinch" eller "to taste": kan ikke skaleres */
}

/* Laeg alt det der mangler til opskriften paa indkoebslisten.
 * Staar varen der allerede med samme enhed, laegges maengderne sammen. */
void tilfoej_manglende(Indkoebsliste *liste, const Opskrift *o, const Valg *v) {
    for (int i = 0; i < o->antal; i++) {
        const Ingrediens *ing = &o->ing[i];
        if (har_ingrediens(&v->koeleskab, ing->navn)) continue;

        double maengde = ing->har_maengde ? skaler(ing->maengde, v->personer) : 0;
        char enhed[2 * KORT];                 /* enhed + note, fx "g" eller "cloves" */
        snprintf(enhed, sizeof enhed, "%s%s%s", ing->enhed, ing->enhed[0] && ing->note[0] ? " " : "", ing->note);

        int fundet = 0;
        for (int k = 0; k < liste->antal && !fundet; k++) {
            Vare *va = &liste->vare[k];
            if (ens(va->navn, ing->navn) && va->har_maengde && ing->har_maengde && ens(va->enhed, enhed)) {
                va->maengde += maengde;
                fundet = 1;
            }
        }
        if (fundet || liste->antal == MAX_INDKOEB) continue;

        Vare *ny = &liste->vare[liste->antal++];
        snprintf(ny->navn, KORT, "%s", ing->navn);
        ny->har_maengde = ing->har_maengde;
        ny->maengde = maengde;
        snprintf(ny->enhed, sizeof ny->enhed, "%s", enhed);
        snprintf(ny->tekst, KORT, "%s", ing->maal);
    }
}

/* Lav een linje af indkoebslisten som tekst, fx "Butter  2 tbs" */
void vare_tekst(const Vare *va, char *buf, int plads) {
    if (va->har_maengde) snprintf(buf, plads, "%-28s %g %s", va->navn, va->maengde, va->enhed);
    else snprintf(buf, plads, "%-28s %s", va->navn, va->tekst);
}

/* Gem indkoebslisten i en tekstfil. Returnerer 1 hvis det gik godt. */
int gem_indkoebsliste(const Indkoebsliste *liste, const char *filnavn) {
    FILE *f = fopen(filnavn, "w");
    if (!f) return 0;
    char linje[256];
    fprintf(f, "Indkøbsliste:\n");
    for (int k = 0; k < liste->antal; k++) {
        vare_tekst(&liste->vare[k], linje, sizeof linje);
        fprintf(f, "  [ ] %s\n", linje);
    }
    fclose(f);
    return 1;
}
