/* tekst.c - smaa hjaelpefunktioner til tekst. */
#include <ctype.h>
#include <string.h>
#include "madplan.h"

/* Fjern mellemrum og linjeskift i begge ender af s */
void trim(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
    size_t start = 0;
    while (isspace((unsigned char)s[start])) start++;
    memmove(s, s + start, n - start + 1);
}

/* Er a og b den samme tekst, naar man ser bort fra store og smaa bogstaver? */
int ens(const char *a, const char *b) {
    while (*a && tolower((unsigned char)*a) == tolower((unsigned char)*b)) { a++; b++; }
    return tolower((unsigned char)*a) == tolower((unsigned char)*b);
}

/* Er tegnet c et bogstav? (UTF-8-tegn som e i "creme" med accent taeller ogsaa som bogstav) */
static int bogstav(char c) {
    return isalpha((unsigned char)c) || (unsigned char)c >= 128;
}

/* Indeholder tekst ordet ord som et helt ord (ignorer store/smaa bogstaver)?
 * Flertal med -s og -es er tilladt, saa "onion" findes i "Red Onions".
 * Men "egg" findes IKKE i "eggplant", og "nut" findes ikke i "nutmeg" eller "coconut".
 * Ord kan godt bestaa af flere ord, fx "peanut butter". */
int indeholder_ord(const char *tekst, const char *ord) {
    size_t l = strlen(ord);
    if (l == 0) return 0;
    for (const char *p = tekst; *p; p++) {
        if (p > tekst && bogstav(p[-1])) continue;       /* skal starte et ord */
        size_t i = 0;
        while (i < l && p[i] && tolower((unsigned char)p[i]) == tolower((unsigned char)ord[i])) i++;
        if (i < l) continue;                            /* ikke ens */
        const char *efter = p + l;
        if (tolower((unsigned char)efter[0]) == 's') efter += 1;              /* onions */
        else if (tolower((unsigned char)efter[0]) == 'e' &&
                 tolower((unsigned char)efter[1]) == 's') efter += 2;          /* tomatoes */
        if (!bogstav(*efter)) return 1;                 /* skal ogsaa slutte et ord */
    }
    return 0;
}

/* Starter et af ordene i tekst med del? ("Red Onions" passer paa "oni", men "Seasoning" goer ikke)
 * Bruges til at filtrere ingredienslisten mens man skriver. */
int ord_starter_med(const char *tekst, const char *del) {
    size_t l = strlen(del);
    for (const char *p = tekst; *p; p++) {
        if (p > tekst && bogstav(p[-1])) continue;       /* kun ved starten af et ord */
        size_t i = 0;
        while (i < l && p[i] && tolower((unsigned char)p[i]) == tolower((unsigned char)del[i])) i++;
        if (i == l) return 1;
    }
    return l == 0;                     /* tom soegning passer paa alt */
}

/* Er ordet allerede i listen? Giver dets plads (0, 1, 2 ...) eller -1 hvis ikke. */
int find_ord(const Ordliste *liste, const char *ord) {
    for (int i = 0; i < liste->antal; i++)
        if (ens(liste->ord[i], ord)) return i;
    return -1;
}

/* Tilfoej ordet hvis det ikke er der, ellers fjern det (som at saette og fjerne et flueben) */
void skift_ord(Ordliste *liste, const char *ord) {
    int i = find_ord(liste, ord);
    if (i >= 0) {
        /* ryk resten af listen en plads op, saa raekkefoelgen bevares */
        memmove(liste->ord[i], liste->ord[i + 1], (liste->antal - i - 1) * sizeof liste->ord[0]);
        liste->antal--;
    } else if (liste->antal < MAX_VARER) {
        snprintf(liste->ord[liste->antal++], KORT, "%s", ord);
    }
}
