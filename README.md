For our application to work we need a database that supports the functionality we plan to make. Here we will outline out process in aquiring that data so far as well as our future plans.

# Data collection:
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


# Data processing
In our initial process of looking for data sets for our P1 program we chose TheMealDB among other reasons because it had already seperated ingridients and amount/unit into two seperate strings making our data processing easier. We planned to write a simple parser program in C that could then split up the strings for amount/unit into amount and unit, with a few additional steps such that we would reach the following goals:

## Data processing goals:
- Amounts can be multiplied with a factor if a user want to make a larger portion of a recipe
- Amounts can be used to make a rough estimate of how many people there are portions for.
- Any units used are converted to units that danes understand rather than oz and pounds. (Ideally also in the instructions part of the recipe)

While the above goals sounds simple in theory it quickly became apparent that the dataset had much more diversity in the amount/unit-strings than we had originally anticipated and that any program converting amount/unit-strings to values that that achieved these goals would require more time to make than what would be realistic for the scope of P1. The fact that we have to write the project in C further adds to this point. 

In the following we will provide som examples of why complexity needed to reach our data processing goals ultimatly ends up so high that it would exceed the scope of our current abilities in the time we have to finish P1.
- The dataset not only contains fractions like 1/2 or ¾, ranges like 1-2, numbers as words, unit conversion strings like "175g/6oz", but also combinations of all these like "1 1/2", 2-1/2, 3-3 1/2, "3 400g Cans" and so forth. This kind of diversity isn't rare corner cases but something that most recipes will contain due to each recipe having multiple ingredients.
- The dataset has very large diversity in spelling and formatting of units as well as most often appending additional text to the end of any unit. For instance grams can be written both as "1g", "1 gram", "100grams", "1 kg", "1 kilograms cans", "1 g of flour disolved in water", and so on. And the same goes for all the non metric units of wheights used in the dataset that needs conversion as well. 

While no single corner case alone would be too complex to handle, the combination of corner cases throughout most recipes in the dataset means that we would not be able to reach our goals for the data processing in the required time without either:
1. Skipping most of the functionality that we wanted to add to our program 
2. Outsourcing the data processing to a vibe coded C program where we understand and document all the overall steps in the data proccesing but won't ourselves write and fully understand every single command used.

Ultimatly the whole point of doing this data processing step is for us to have a dataset that supports the functionality that we want to build and to use units that danes understand. With limited time to finish the coding for the project we wouldn't even get to work on this functionality if we had to impliment the entire data processing ourselves from scratch, thus making the processing kind of pointless in terms of our final product. For this reason after consulting our supervisers we decided to go with option 2 of outsourcing the data processing so we can focus our coding efforts on actually using the data for our project.

# Data organisation
We are currently in the process of deciding how to best organise our data for our program to access what it needs. While something like a SQL database would most likely be the ideal solution for a real production application, it would probably also take more time than we have, since we haven't yet worked with SQL. Since our data is relatively small (800 recipes total and 10000 ingredients total) it seems reasonable to asume that we can just use a few json files as out database and still make quires at sufficiently fast speeds. The exact json formats we will ultimately use are yet to be decided, though it would likely be close to what we are currently working with which will be presented at the status seminar.

