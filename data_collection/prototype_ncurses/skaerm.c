/* skaerm.c - alle skaermene, tegnet med ncurses.
 *
 * ncurses i korte traek:
 *   initscr()            tag over terminalen (kaldes i start_skaerm)
 *   clear()              visk skaermen ren (i hukommelsen)
 *   mvprintw(y, x, ...)  som printf, men paa linje y, kolonne x (0,0 er oeverst til venstre)
 *   attron/attroff(...)  slaa fx A_REVERSE (omvendte farver = markering) til og fra
 *   refresh()            vis det hele paa den rigtige skaerm
 *   getch()              vent paa een tast; piletaster kommer som KEY_UP, KEY_DOWN osv.
 *   endwin()             giv terminalen tilbage (kaldes i stop_skaerm)
 *   LINES, COLS          hvor mange linjer og kolonner terminalen har lige nu
 *
 * Hver skaerm er en loekke: tegn alt -> vent paa en tast -> reager -> tegn igen.
 */
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "madplan.h"

#define ESC 27

/* Farvepar: et nummer for hver kombination af tekstfarve og baggrund */
enum { FARVE_TITEL = 1, FARVE_HAR, FARVE_MANGLER };

void start_skaerm(void) {
    setenv("ESCDELAY", "25", 0);    /* ellers venter ncurses et helt sekund efter Esc */
    initscr();
    cbreak();                        /* faa tasterne med det samme, uden at vente paa Enter */
    noecho();                        /* vis ikke tasterne automatisk; vi tegner selv */
    keypad(stdscr, TRUE);            /* piletaster som KEY_UP osv. i stedet for koder */
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(FARVE_TITEL, COLOR_CYAN, -1);
        init_pair(FARVE_HAR, COLOR_GREEN, -1);
        init_pair(FARVE_MANGLER, COLOR_RED, -1);
    }
}

void stop_skaerm(void) {
    endwin();
}

/* Skriv tekst paa linje y fra kolonne x, men skaer den af ved skaermens hoejre kant */
void skriv_afkortet(int y, int x, const char *tekst) {
    int plads = COLS - x - 1;
    if (plads > 0) mvaddnstr(y, x, tekst, plads);
}

/* Ryd skaermen, skriv titlen oeverst og en hjaelpelinje med tasterne nederst */
void overskrift(const char *titel, const char *hjaelp) {
    clear();
    attron(A_BOLD | COLOR_PAIR(FARVE_TITEL));
    skriv_afkortet(0, 2, titel);
    attroff(A_BOLD | COLOR_PAIR(FARVE_TITEL));
    attron(A_DIM);
    skriv_afkortet(LINES - 1, 2, hjaelp);
    attroff(A_DIM);
}

/* Vis en besked nederst og vent paa en tast */
void besked(const char *tekst) {
    move(LINES - 2, 0);
    clrtoeol();
    attron(A_BOLD);
    skriv_afkortet(LINES - 2, 2, tekst);
    attroff(A_BOLD);
    refresh();
    getch();
}

/* Hjaelp til lister der er laengere end skaermen: giver den foerste raekke der skal vises,
 * saa den valgte raekke altid er synlig. */
static int rul(int valgt, int start, int hoejde) {
    if (valgt < start) start = valgt;
    if (valgt >= start + hoejde) start = valgt - hoejde + 1;
    return start < 0 ? 0 : start;
}

/* Faelles piletaster for alle lister: flyt valgt op/ned og hold den inden for 0..antal-1 */
static int flyt(int valgt, int tast, int antal, int hoejde) {
    if (tast == KEY_UP) valgt--;
    if (tast == KEY_DOWN) valgt++;
    if (tast == KEY_PPAGE) valgt -= hoejde;
    if (tast == KEY_NPAGE) valgt += hoejde;
    if (valgt >= antal) valgt = antal - 1;
    if (valgt < 0) valgt = 0;
    return valgt;
}

static int er_enter(int tast) { return tast == '\n' || tast == '\r' || tast == KEY_ENTER; }

/* ------------------------------------------------------------------------------------
 * Ingrediensvaelgeren: man skriver, og listen filtreres for hvert bogstav.
 * Man kan kun vaelge navne der findes i opskrifterne, saa stavefejl er umulige.
 * ------------------------------------------------------------------------------------ */
void vaelg_ingredienser(Ordliste *liste, const char *titel, char navne[][KORT], int antal_navne) {
    static int fundne[MAX_NAVNE];    /* numrene paa de navne der passer til soegningen */
    char soeg[KORT] = "";
    int valgt = 0, start = 0;

    for (;;) {
        /* 1) Filtrer: hvilke navne har et ord der starter med det man har skrevet? */
        int n = 0;
        for (int i = 0; i < antal_navne; i++)
            if (ord_starter_med(navne[i], soeg)) fundne[n++] = i;
        int hoejde = LINES - 9;
        valgt = flyt(valgt, 0, n, hoejde);
        start = rul(valgt, start, hoejde);

        /* 2) Tegn */
        overskrift(titel, "Skriv for at søge   ↑↓ flyt   Enter vælg/fravælg   Esc færdig");
        mvprintw(2, 2, "Søg: ");
        attron(A_BOLD);
        printw("%s", soeg);
        attroff(A_BOLD);
        attron(A_DIM);
        mvprintw(3, 2, "%d af %d ingredienser passer", n, antal_navne);
        attroff(A_DIM);
        for (int r = 0; r < hoejde && start + r < n; r++) {
            const char *navn = navne[fundne[start + r]];
            if (start + r == valgt) attron(A_REVERSE);
            mvprintw(5 + r, 2, "%s %s", find_ord(liste, navn) >= 0 ? "[x]" : "[ ]", navn);
            if (start + r == valgt) attroff(A_REVERSE);
        }
        char valgte[1024] = "";       /* "Valgt: onion, rice, ..." nederst */
        for (int i = 0; i < liste->antal; i++) {
            strncat(valgte, i ? ", " : "", sizeof valgte - strlen(valgte) - 1);
            strncat(valgte, liste->ord[i], sizeof valgte - strlen(valgte) - 1);
        }
        mvprintw(LINES - 3, 2, "Valgt (%d): ", liste->antal);
        skriv_afkortet(LINES - 3, 15, liste->antal ? valgte : "(ingen)");
        move(2, 7 + (int)strlen(soeg));                  /* markoeren staar i soegefeltet */
        refresh();

        /* 3) Reager paa tasten */
        int tast = getch();
        size_t l = strlen(soeg);
        if (tast == ESC) return;
        else if (er_enter(tast)) {
            if (n > 0) skift_ord(liste, navne[fundne[valgt]]);
            soeg[0] = '\0';                              /* klar til at soege efter den naeste */
            valgt = start = 0;
        }
        else if (tast == KEY_BACKSPACE || tast == 127 || tast == 8) {
            /* slet sidste tegn; et UTF-8-tegn som oe fylder flere bytes (10xxxxxx er fortsaettelser) */
            while (l > 0 && ((unsigned char)soeg[l - 1] & 0xC0) == 0x80) soeg[--l] = '\0';
            if (l > 0) soeg[--l] = '\0';
            valgt = start = 0;
        }
        else if (tast >= ' ' && tast < 256 && l + 1 < KORT) {
            soeg[l] = (char)tast;                        /* et nyt bogstav i soegningen */
            soeg[l + 1] = '\0';
            valgt = start = 0;
        }
        else valgt = flyt(valgt, tast, n, hoejde);
    }
}

/* ------------------------------------------------------------------------------------
 * Praeferencer og allergier: en liste med flueben
 * ------------------------------------------------------------------------------------ */
void vaelg_praeferencer(Valg *v) {
    int valgt = 0, n = antal_praeferencer();
    for (;;) {
        overskrift("Præferencer og allergier", "↑↓ flyt   Enter/mellemrum slå til/fra   Esc færdig");
        for (int p = 0; p < n; p++) {
            if (p == valgt) attron(A_REVERSE);
            mvprintw(2 + p, 2, "[%s] %s", v->praeference[p] ? "x" : " ", praeference_navn(p));
            if (p == valgt) attroff(A_REVERSE);
        }
        refresh();
        int tast = getch();
        if (tast == ESC || tast == 'q') return;
        if (er_enter(tast) || tast == ' ') v->praeference[valgt] = !v->praeference[valgt];
        else valgt = flyt(valgt, tast, n, n);
    }
}

/* ------------------------------------------------------------------------------------
 * Visning af een opskrift. Teksten bygges foerst op som linjer, og saa kan man rulle i dem.
 * ------------------------------------------------------------------------------------ */
typedef struct { char tekst[256]; int farve; } Linje;   /* farve 0 = almindelig */
static Linje linjer[600];
static int antal_linjer;

static void ny_linje(int farve, const char *tekst) {
    if (antal_linjer == 600) return;
    snprintf(linjer[antal_linjer].tekst, sizeof linjer[0].tekst, "%s", tekst);
    linjer[antal_linjer++].farve = farve;
}

/* Del en lang tekst i linjer paa hoejst bredde tegn, helst ved et mellemrum */
static void ny_ombrudt(const char *tekst, int bredde) {
    char stykke[256];
    while (*tekst) {
        int n = 0;
        while (tekst[n] && tekst[n] != '\n' && n < bredde) n++;
        if (tekst[n] && tekst[n] != '\n') {            /* for lang: gaa tilbage til et mellemrum */
            int m = n;
            while (m > 0 && tekst[m] != ' ') m--;
            if (m > 0) n = m;
        }
        snprintf(stykke, sizeof stykke, "%.*s", n, tekst);
        ny_linje(0, stykke);
        tekst += n;
        if (*tekst == ' ' || *tekst == '\n') tekst++;
    }
}

static void vis_opskrift(const Opskrift *o, const Valg *v, Indkoebsliste *indkoeb) {
    char tmp[256], maengde[128];
    int bredde = COLS - 6 < 250 ? COLS - 6 : 250;

    antal_linjer = 0;
    snprintf(tmp, sizeof tmp, "%s%s%s. Til %d personer (opskriften er regnet som %d).",
             o->kategori, o->land[0] ? ", " : "", o->land, v->personer, BASIS_PERSONER);
    ny_linje(0, tmp);
    beskriv_indhold(o, maengde, sizeof maengde);
    snprintf(tmp, sizeof tmp, "Indeholder: %s", maengde);
    ny_linje(0, tmp);
    ny_linje(0, "");
    ny_linje(FARVE_TITEL, "Ingredienser:");
    for (int i = 0; i < o->antal; i++) {
        int har = har_ingrediens(&v->koeleskab, o->ing[i].navn);
        maengde_tekst(&o->ing[i], v->personer, maengde, sizeof maengde);
        snprintf(tmp, sizeof tmp, "%s %-28s %s", har ? "[har]    " : "[mangler]", o->ing[i].navn, maengde);
        ny_linje(har ? FARVE_HAR : FARVE_MANGLER, tmp);
    }
    ny_linje(0, "");
    ny_linje(FARVE_TITEL, "Fremgangsmåde:");
    ny_ombrudt(o->fremgangsmaade ? o->fremgangsmaade : "(ingen)", bredde);

    int start = 0;
    for (;;) {
        int hoejde = LINES - 4;
        overskrift(o->navn, "↑↓ rul   i læg det manglende på indkøbslisten   Esc tilbage");
        for (int r = 0; r < hoejde && start + r < antal_linjer; r++) {
            int f = linjer[start + r].farve;
            if (f) attron(COLOR_PAIR(f));
            skriv_afkortet(2 + r, 2, linjer[start + r].tekst);
            if (f) attroff(COLOR_PAIR(f));
        }
        refresh();
        int tast = getch();
        if (tast == ESC || tast == 'q') return;
        if (tast == 'i') {
            int foer = indkoeb->antal;
            tilfoej_manglende(indkoeb, o, v);
            snprintf(tmp, sizeof tmp, "Lagt på indkøbslisten (%d nye linjer). Tryk en tast.", indkoeb->antal - foer);
            besked(tmp);
        }
        int sidste = antal_linjer > hoejde ? antal_linjer - hoejde : 0;
        start = flyt(start, tast, sidste + 1, hoejde);
    }
}

/* ------------------------------------------------------------------------------------
 * Forslag: de bedste opskrifter ud fra koeleskabet og praeferencerne
 * ------------------------------------------------------------------------------------ */
#define MAX_FORSLAG 30

void vis_forslag(const Opskrift *opskrifter, int n, const Valg *v, Indkoebsliste *indkoeb) {
    Forslag forslag[MAX_FORSLAG];
    if (v->koeleskab.antal == 0) {
        besked("Vælg først hvad du har i køleskabet. Tryk en tast.");
        return;
    }
    int antal = foreslaa(opskrifter, n, v, forslag, MAX_FORSLAG);
    if (antal == 0) {
        besked("Ingen opskrifter passer. Prøv flere ingredienser eller færre præferencer.");
        return;
    }
    int valgt = 0, start = 0;
    for (;;) {
        int hoejde = LINES - 5;
        start = rul(valgt, start, hoejde);
        overskrift("Forslag (færrest manglende ingredienser først)", "↑↓ flyt   Enter se opskrift   Esc tilbage");
        attron(A_DIM);
        mvprintw(2, 2, "%-42s %-14s %s", "Opskrift", "Kategori", "Mangler");
        attroff(A_DIM);
        for (int r = 0; r < hoejde && start + r < antal; r++) {
            const Forslag *f = &forslag[start + r];
            const Opskrift *o = &opskrifter[f->nr];
            char raekke[256];
            snprintf(raekke, sizeof raekke, "%-42.42s %-14s %d af %d", o->navn, o->kategori, f->mangler, o->antal);
            if (start + r == valgt) attron(A_REVERSE);
            skriv_afkortet(3 + r, 2, raekke);
            if (start + r == valgt) attroff(A_REVERSE);
        }
        refresh();
        int tast = getch();
        if (tast == ESC || tast == 'q') return;
        if (er_enter(tast)) vis_opskrift(&opskrifter[forslag[valgt].nr], v, indkoeb);
        else valgt = flyt(valgt, tast, antal, hoejde);
    }
}

/* ------------------------------------------------------------------------------------
 * Indkoebslisten: se den, fjern det du har koebt, gem den i en fil
 * ------------------------------------------------------------------------------------ */
void vis_indkoebsliste(Indkoebsliste *indkoeb) {
    int valgt = 0, start = 0;
    for (;;) {
        int hoejde = LINES - 4;
        start = rul(valgt, start, hoejde);
        overskrift("Indkøbsliste", "↑↓ flyt   d fjern linje   g gem i indkoebsliste.txt   Esc tilbage");
        if (indkoeb->antal == 0) mvprintw(2, 2, "Indkøbslisten er tom.");
        for (int r = 0; r < hoejde && start + r < indkoeb->antal; r++) {
            char raekke[256];
            vare_tekst(&indkoeb->vare[start + r], raekke, sizeof raekke);
            if (start + r == valgt) attron(A_REVERSE);
            mvprintw(2 + r, 2, "[ ] ");
            skriv_afkortet(2 + r, 6, raekke);
            if (start + r == valgt) attroff(A_REVERSE);
        }
        refresh();
        int tast = getch();
        if (tast == ESC || tast == 'q') return;
        if ((tast == 'd' || tast == KEY_DC) && indkoeb->antal > 0) {
            /* fjern linjen ved at rykke resten en plads op */
            memmove(&indkoeb->vare[valgt], &indkoeb->vare[valgt + 1],
                    (indkoeb->antal - valgt - 1) * sizeof indkoeb->vare[0]);
            indkoeb->antal--;
        }
        if (tast == 'g')
            besked(gem_indkoebsliste(indkoeb, "indkoebsliste.txt")
                   ? "Gemt i indkoebsliste.txt. Tryk en tast." : "Kunne ikke gemme filen. Tryk en tast.");
        valgt = flyt(valgt, tast, indkoeb->antal, hoejde);
    }
}
