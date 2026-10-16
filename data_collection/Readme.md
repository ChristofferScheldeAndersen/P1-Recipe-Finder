# Dokumentation af fremgangsmåde:
Vi er startet med at undersøge mulighederne for at hente data om opskrifter til vores projekt. Vi har anvendt AI ifm. at få overblik over mulighederne, som vi derfra selvstændigt har undersøgt videre. Vi har som udgangspunkt ledt efter et sted at hente data der opfylder følgende kriterier:
- Det skal være lovligt at hente og bruge data til vores formål
- Det skal være gratis eller højest involvere en lav engangsbetaling at hente den nødvendige data
- Det skal være muligt at hente data der er eller kan gøres kompatibel med vores core funktionalitet (R1) og ideelt set også med den funktionalitet, vi overvejer at tilføje efterfølgende (R2 og R3).
Med udgangspunkt i disse kriterier endte vi med at vælge https://www.themealdb.com/. Den kan bruges gratis og lovligt og indeholder en til projektet en passende mængde opskrifter (793) og ingredienser (992). Desuden har data herfra opdelt information om ingrediens og mængde i seperate strings, hvilket muliggør opslag ud fra ingredienslister, der ikke var muligt hos mange andre af de kandidater vi overvejede. Data indeholder desuden information om kategorier som hvorvidt en ret er vegetarisk og hvilket køkken den er inspireret af. 

Themealdb har en tilgængelig API med egen dokumentation: https://www.themealdb.com/documentation
Vi ønskede at hente en samlet json fil med alle 793 opskrifter og deres tilhørende data. Dokumentationen indeholdt ikke en direkte kommando til at hente alle opskrifter, men derimod mulighed for at slå enkelte opskrifter op ud fra deres ID. Dokumentationen angav desuden mulighed for at lave opslag i hver enkelt kategori for bl.a. alle opskrifts'id inden for kategorien. 

Fremgangsmåden blev derfor først at loope gennem alle kategorier og samle opskrifts-id'erne fra hver, fjerne evt. dubletter for vores id liste og tjekke at vi havde fundet alle 793 ID'er. Bash terminal-kommandoerne til dette er samlet i: *find_ids.sh*

Herefter loopede vi gennem hvert id og hentede alt relevant information for hvert ID. Bash terminal kommandoerne hertil er samlet i: *download_recipes.sh*

Den resulterende fil *opskrifter.json* indeholder den resulterende ufiltrerede data for alle 793 opskrifter.