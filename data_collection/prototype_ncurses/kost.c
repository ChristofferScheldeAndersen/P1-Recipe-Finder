/* kost.c - praeferencer og allergier.
 *
 * Data siger ikke om en ret er vegansk eller indeholder noedder. Derfor goetter vi
 * ud fra ingrediensernes navne med simple ordlister. Det er en prototype: listerne
 * er ikke komplette, og et ord som "stock" kan baade vaere grontsags- og kyllingefond.
 *
 * Ideen:
 *  1) Hver GRUPPE (koed, fisk, maelk ...) har en liste af ord, og en liste af undtagelser
 *     ("peanut butter" er ikke smoer, "coconut milk" er ikke maelk).
 *  2) Hver PRAEFERENCE forbyder en eller flere grupper. Vegansk forbyder fx
 *     koed + fisk + maelk + aeg + honning.
 */
#include <string.h>
#include "madplan.h"

enum { KOED, FISK, MAELK, AEG, GLUTEN, NOEDDER, HONNING, ANTAL_GRUPPER };

typedef struct {
    const char *navn;
    const char *ord[48];         /* ingrediensen tilhoerer gruppen hvis den indeholder et af disse */
    const char *undtagelser[12]; /* ... medmindre den ogsaa indeholder et af disse */
} Gruppe;

static const Gruppe grupper[ANTAL_GRUPPER] = {
    [KOED] = {"kød",
        {"chicken", "beef", "pork", "lamb", "mutton", "veal", "goat", "duck", "turkey", "bacon",
         "ham", "sausage", "chorizo", "salami", "pancetta", "prosciutto", "mince", "steak",
         "kidney", "liver", "oxtail", "venison", "rabbit", "lard", "suet", "gelatine", "gelatin",
         "meat", "meatballs", "brisket", "ribs", "pheasant", "partridge", "goose", "pepperoni"},
        {"kidney beans", "vegetable stock", "vegetarian"}},
    [FISK] = {"fisk/skaldyr",
        {"fish", "salmon", "tuna", "cod", "haddock", "mackerel", "sardine", "anchovy",
         "anchovies", "trout", "tilapia", "monkfish", "bass", "herring", "prawn", "prawns",
         "shrimp", "crab", "lobster", "mussels", "clams", "squid", "oyster", "scallops",
         "white fish", "kipper", "pilchards", "plaice", "sole"},
        {0}},
    [MAELK] = {"mælk",
        {"milk", "butter", "cream", "cheese", "yogurt", "yoghurt", "ghee", "parmesan",
         "mozzarella", "cheddar", "feta", "ricotta", "mascarpone", "creme fraiche",
         "crème fraîche", "buttermilk", "custard", "paneer", "brie", "gruyere", "gruyère",
         "stilton", "halloumi", "fromage frais", "quark", "condensed milk", "ice cream"},
        {"coconut milk", "coconut cream", "almond milk", "soy milk", "oat milk",
         "peanut butter", "butter beans", "cocoa butter", "vegan", "cream of tartar"}},
    [AEG] = {"æg",
        {"egg", "yolk", "yolks", "mayonnaise", "meringue"},
        {"vegan"}},
    [GLUTEN] = {"gluten",
        {"flour", "bread", "breadcrumbs", "pasta", "spaghetti", "noodles", "wheat", "barley",
         "rye", "couscous", "bulgur", "semolina", "soy sauce", "beer", "pastry", "tortilla",
         "tortillas", "biscuits", "digestive", "macaroni", "lasagne", "penne", "fettuccine",
         "tagliatelle", "linguine", "rigatoni", "farfalle", "filo", "baguette", "ciabatta",
         "pitta", "naan", "croissant", "oats", "bun", "buns", "brioche", "cake", "crackers"},
        {"rice flour", "gluten-free", "gluten free", "rice noodles", "corn tortillas",
         "almond flour", "chickpea flour", "gram flour"}},
    [NOEDDER] = {"nødder",
        {"nut", "nuts", "almond", "almonds", "walnut", "walnuts", "cashew", "cashews", "peanut",
         "peanuts", "pecan", "pecans", "hazelnut", "hazelnuts", "pistachio", "pistachios",
         "pine nuts", "macadamia", "praline", "marzipan", "nutella", "frangipane"},
        {0}},
    [HONNING] = {"honning", {"honey"}, {0}},
};

/* Praeferencerne man kan slaa til og fra i menuen, og hvilke grupper de forbyder */
static const struct { const char *navn; int forbudt[ANTAL_GRUPPER]; } praef[] = {
    {"Vegetar",                {[KOED] = 1, [FISK] = 1}},
    {"Vegansk",                {[KOED] = 1, [FISK] = 1, [MAELK] = 1, [AEG] = 1, [HONNING] = 1}},
    {"Glutenfri",              {[GLUTEN] = 1}},
    {"Laktosefri / mælkeallergi", {[MAELK] = 1}},
    {"Ægallergi",              {[AEG] = 1}},
    {"Nøddeallergi",           {[NOEDDER] = 1}},
    {"Fisk- og skaldyrsallergi", {[FISK] = 1}},
};

int antal_praeferencer(void) { return sizeof praef / sizeof praef[0]; }
const char *praeference_navn(int nr) { return praef[nr].navn; }

/* Hoerer ingrediensen til gruppen g? */
static int i_gruppe(const char *ingrediens, int g) {
    for (int i = 0; grupper[g].undtagelser[i]; i++)
        if (indeholder_ord(ingrediens, grupper[g].undtagelser[i])) return 0;
    for (int i = 0; grupper[g].ord[i]; i++)
        if (indeholder_ord(ingrediens, grupper[g].ord[i])) return 1;
    return 0;
}

/* Maa brugeren faa denne opskrift med de valgte praeferencer og undgaa-ord? */
int opskrift_tilladt(const Opskrift *o, const Valg *v) {
    for (int i = 0; i < o->antal; i++) {
        const char *navn = o->ing[i].navn;
        for (int p = 0; p < antal_praeferencer(); p++) {
            if (!v->praeference[p]) continue;
            for (int g = 0; g < ANTAL_GRUPPER; g++)
                if (praef[p].forbudt[g] && i_gruppe(navn, g)) return 0;
        }
        for (int u = 0; u < v->undgaa.antal; u++)
            if (indeholder_ord(navn, v->undgaa.ord[u])) return 0;
    }
    return 1;
}

/* Lav en tekst med de grupper retten indeholder, fx "mælk, æg, gluten", i buf */
void beskriv_indhold(const Opskrift *o, char *buf, int plads) {
    buf[0] = '\0';
    for (int g = 0; g < ANTAL_GRUPPER; g++) {
        int fundet = 0;
        for (int i = 0; i < o->antal && !fundet; i++)
            fundet = i_gruppe(o->ing[i].navn, g);
        if (fundet) {
            if (buf[0]) strncat(buf, ", ", plads - strlen(buf) - 1);
            strncat(buf, grupper[g].navn, plads - strlen(buf) - 1);
        }
    }
    if (!buf[0]) snprintf(buf, plads, "intet af det vi kigger efter");
}
