/* madplan.h - faelles typer og funktioner for madplan-prototypen (ncurses-udgaven).
 *
 * Programmet er delt i seks filer:
 *   main.c      starter programmet og viser hovedmenuen
 *   skaerm.c    alle ncurses-skaermene (ingrediensvaelger, praeferencer, forslag, opskrift, indkoebsliste)
 *   indlaes.c   laeser opskrifterne fra opskrifter_normaliseret2.json
 *   tekst.c     smaa tekst-hjaelpere (sammenlign uden store/smaa bogstaver, find et ord)
 *   kost.c      praeferencer og allergier (vegansk, glutenfri, noeddeallergi ...)
 *   planlaeg.c  forslag ud fra koeleskabet, skalering til antal personer, indkoebsliste
 */
#ifndef MADPLAN_H
#define MADPLAN_H

#include <stdio.h>

#define MAX_OPSKRIFTER   1000
#define MAX_INGREDIENSER 20     /* TheMealDB har strIngredient1..20 */
#define MAX_VARER        50     /* hvor mange ting man kan skrive i koeleskab / undgaa-listen */
#define MAX_INDKOEB      200    /* linjer paa indkoebslisten */
#define MAX_NAVNE        1500   /* forskellige ingrediensnavne i data (der er 948) */
#define KORT             64     /* laengde paa korte tekster (navne, enheder) */

/* Data har ikke antal portioner, saa vi ANTAGER at alle opskrifter er til 4 personer. */
#define BASIS_PERSONER   4

/* En ingrediens i en opskrift, fx  2 | tbs | (ingen note) | Olive oil */
typedef struct {
    char navn[KORT];          /* strIngredientN, fx "Olive oil" */
    char maal[KORT];          /* strMeasureN som den staar, fx "3 tablespoons" (bruges hvis intet tal) */
    int  har_maengde;         /* 1 hvis amountN var et tal */
    double maengde;           /* amountN, fx 3 */
    char enhed[KORT];         /* unitN, fx "tablespoons" (tom hvis ingen) */
    char note[KORT];          /* noteN, fx "cloves" eller "to taste" (tom hvis ingen) */
} Ingrediens;

typedef struct {
    char navn[128];           /* strMeal */
    char kategori[KORT];      /* strCategory, fx "Dessert" */
    char land[KORT];          /* strArea, fx "Greek" */
    char *fremgangsmaade;     /* strInstructions (kan vaere lang, saa den ligger i heap) */
    int  antal;               /* antal ingredienser i listen herunder */
    Ingrediens ing[MAX_INGREDIENSER];
} Opskrift;

/* En liste af ord som brugeren har skrevet, fx koeleskabet: "onion", "garlic", "rice" */
typedef struct {
    int antal;
    char ord[MAX_VARER][KORT];
} Ordliste;

/* Alt det brugeren har valgt i menuen */
typedef struct {
    Ordliste koeleskab;       /* det man har derhjemme */
    Ordliste undgaa;          /* ting man ikke vil have, fx "mushroom" */
    int praeference[16];      /* 1/0 for hver praeference i kost.c */
    int personer;             /* hvor mange der skal spise */
} Valg;

/* Een linje paa indkoebslisten */
typedef struct {
    char navn[KORT];
    int  har_maengde;
    double maengde;
    char enhed[2 * KORT];     /* enhed + note, fx "g" eller "cloves" */
    char tekst[KORT];         /* bruges naar der ikke er et tal, fx "pinch" */
} Vare;

typedef struct {
    int antal;
    Vare vare[MAX_INDKOEB];
} Indkoebsliste;

/* Et forslag: opskriftens nummer, hvor mange ingredienser der mangler, og hvor mange man har */
typedef struct { int nr, mangler, har; } Forslag;

/* indlaes.c */
int indlaes_opskrifter(const char *filnavn, Opskrift *opskrifter, int max);
int lav_ingrediensliste(const Opskrift *opskrifter, int n, char navne[][KORT], int max);

/* tekst.c */
void trim(char *s);
int  ens(const char *a, const char *b);                    /* samme tekst, ignorer store/smaa */
int  indeholder_ord(const char *tekst, const char *ord);    /* "Chopped onions" indeholder ordet "onion" */
int  ord_starter_med(const char *tekst, const char *del);   /* et ord i "Red Onions" starter med "oni" */
int  find_ord(const Ordliste *liste, const char *ord);      /* plads i listen, eller -1 */
void skift_ord(Ordliste *liste, const char *ord);          /* tilfoej, eller fjern hvis den er der */

/* kost.c */
int  antal_praeferencer(void);
const char *praeference_navn(int nr);
int  opskrift_tilladt(const Opskrift *o, const Valg *v);
void beskriv_indhold(const Opskrift *o, char *buf, int plads);  /* "mælk, gluten" */

/* planlaeg.c */
int  er_basisvare(const char *ingrediens);              /* salt, peber, vand antages at vaere hjemme */
int  har_ingrediens(const Ordliste *koeleskab, const char *ingrediens);
int  foreslaa(const Opskrift *opskrifter, int n, const Valg *v, Forslag *resultat, int max);
void maengde_tekst(const Ingrediens *ing, int personer, char *buf, int plads);
void tilfoej_manglende(Indkoebsliste *liste, const Opskrift *o, const Valg *v);
void vare_tekst(const Vare *va, char *buf, int plads);
int  gem_indkoebsliste(const Indkoebsliste *liste, const char *filnavn);

/* skaerm.c */
void start_skaerm(void);                                /* goer ncurses klar */
void stop_skaerm(void);                                 /* giver terminalen tilbage */
void overskrift(const char *titel, const char *hjaelp); /* ryd skaermen, titel oeverst, taster nederst */
void skriv_afkortet(int y, int x, const char *tekst);   /* skriv, men stop ved hoejre kant */
void besked(const char *tekst);                         /* besked nederst, vent paa en tast */
void vaelg_ingredienser(Ordliste *liste, const char *titel, char navne[][KORT], int antal_navne);
void vaelg_praeferencer(Valg *v);
void vis_forslag(const Opskrift *opskrifter, int n, const Valg *v, Indkoebsliste *indkoeb);
void vis_indkoebsliste(Indkoebsliste *indkoeb);

#endif
