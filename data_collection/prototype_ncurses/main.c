/* main.c - madplan-prototype med ncurses: starter programmet og viser hovedmenuen.
 *
 *   macOS:  cc -Wall -Wextra -o madplan *.c -lncurses -lm
 *   Linux:  cc -Wall -Wextra -o madplan *.c -lncursesw -lm
 *   ./madplan           laeser ../opskrifter_normaliseret2.json
 *   ./madplan fil.json  laeser en anden fil i samme format
 */
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include "madplan.h"

#define STANDARDFIL "../opskrifter_normaliseret2.json"

static Opskrift opskrifter[MAX_OPSKRIFTER];   /* ca. 3 MB, derfor global og ikke paa stakken */
static int antal_opskrifter;
static char navne[MAX_NAVNE][KORT];           /* alle forskellige ingrediensnavne, alfabetisk */
static int antal_navne;
static Valg valg;
static Indkoebsliste indkoeb;

/* Punkterne i hovedmenuen, i samme raekkefoelge som teksterne i tegn_menu() */
enum { KOELESKAB, UNDGAA, PRAEFERENCER, PERSONER, FORSLAG, INDKOEB, AFSLUT, ANTAL_PUNKTER };

/* Lav en linje som "Køleskab: onion, rice" ud fra en ordliste */
static void ordliste_tekst(const char *foran, const Ordliste *l, char *buf, int plads) {
    snprintf(buf, plads, "%s", foran);
    if (l->antal == 0) strncat(buf, "(ingen)", plads - strlen(buf) - 1);
    for (int i = 0; i < l->antal; i++) {
        if (i) strncat(buf, ", ", plads - strlen(buf) - 1);
        strncat(buf, l->ord[i], plads - strlen(buf) - 1);
    }
}

static void tegn_menu(int valgt) {
    char tekst[ANTAL_PUNKTER][128];
    snprintf(tekst[KOELESKAB], 128, "Køleskab: vælg hvad du har");
    snprintf(tekst[UNDGAA], 128, "Undgå: ingredienser du ikke vil have");
    snprintf(tekst[PRAEFERENCER], 128, "Præferencer og allergier");
    snprintf(tekst[PERSONER], 128, "Antal personer:  < %d >", valg.personer);
    snprintf(tekst[FORSLAG], 128, "Foreslå opskrifter");
    snprintf(tekst[INDKOEB], 128, "Indkøbsliste (%d linjer)", indkoeb.antal);
    snprintf(tekst[AFSLUT], 128, "Afslut");

    overskrift("MADPLAN", "↑↓ flyt   Enter vælg   ←→ antal personer   q afslut");
    for (int i = 0; i < ANTAL_PUNKTER; i++) {
        if (i == valgt) attron(A_REVERSE);
        mvprintw(2 + i, 2, " %-40s ", tekst[i]);
        if (i == valgt) attroff(A_REVERSE);
    }

    /* Status under menuen: hvad brugeren har valgt indtil nu */
    char linje[1024];
    int y = 3 + ANTAL_PUNKTER;
    ordliste_tekst("Køleskab:     ", &valg.koeleskab, linje, sizeof linje);
    skriv_afkortet(y, 2, linje);
    ordliste_tekst("Undgå:        ", &valg.undgaa, linje, sizeof linje);
    skriv_afkortet(y + 1, 2, linje);
    snprintf(linje, sizeof linje, "Præferencer:  ");
    int nogen = 0;
    for (int p = 0; p < antal_praeferencer(); p++) {
        if (!valg.praeference[p]) continue;
        if (nogen++) strncat(linje, ", ", sizeof linje - strlen(linje) - 1);
        strncat(linje, praeference_navn(p), sizeof linje - strlen(linje) - 1);
    }
    if (!nogen) strncat(linje, "(ingen)", sizeof linje - strlen(linje) - 1);
    skriv_afkortet(y + 2, 2, linje);
    refresh();
}

int main(int argc, char **argv) {
    const char *fil = argc > 1 ? argv[1] : STANDARDFIL;
    setlocale(LC_ALL, "");                      /* saa ncurses kan vise æ, ø og å */
    antal_opskrifter = indlaes_opskrifter(fil, opskrifter, MAX_OPSKRIFTER);
    if (antal_opskrifter <= 0) {
        fprintf(stderr, "madplan: kunne ikke læse opskrifter fra %s\n", fil);
        return 1;
    }
    antal_navne = lav_ingrediensliste(opskrifter, antal_opskrifter, navne, MAX_NAVNE);
    valg.personer = BASIS_PERSONER;

    start_skaerm();
    int valgt = 0;
    for (;;) {
        tegn_menu(valgt);
        int tast = getch();
        if (tast == 'q') break;
        if (tast == KEY_UP && valgt > 0) valgt--;
        if (tast == KEY_DOWN && valgt < ANTAL_PUNKTER - 1) valgt++;
        if (valgt == PERSONER && tast == KEY_LEFT && valg.personer > 1) valg.personer--;
        if (valgt == PERSONER && tast == KEY_RIGHT && valg.personer < 50) valg.personer++;
        if (tast != '\n' && tast != '\r' && tast != KEY_ENTER) continue;

        /* Enter: gaa ind i den valgte skaerm */
        if (valgt == KOELESKAB)
            vaelg_ingredienser(&valg.koeleskab, "Køleskab: hvad har du?", navne, antal_navne);
        else if (valgt == UNDGAA)
            vaelg_ingredienser(&valg.undgaa, "Undgå: hvad vil du ikke have?", navne, antal_navne);
        else if (valgt == PRAEFERENCER) vaelg_praeferencer(&valg);
        else if (valgt == FORSLAG) vis_forslag(opskrifter, antal_opskrifter, &valg, &indkoeb);
        else if (valgt == INDKOEB) vis_indkoebsliste(&indkoeb);
        else if (valgt == AFSLUT) break;
    }
    stop_skaerm();
    return 0;
}
