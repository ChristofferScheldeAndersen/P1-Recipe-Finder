# Original plan and proces
In our initial process of looking for datasets for our P1 program, we chose TheMealDB, among other reasons, because it had already separated ingredients and amount/unit information into two separate strings, making our data processing easier.

We planned to write a simple parser program in C that could split the amount/unit strings into amounts and units, with a few additional steps, so that we would achieve the following goals:

Data Processing Goals:
• Amounts can be multiplied by a factor if a user wants to make a larger portion of a recipe.
• Amounts can be used to make a rough estimate of how many people a recipe serves.
• Any units used are converted to units that Danes understand, rather than oz and pounds (ideally also in the instructions part of the recipe).

While the above goals sound simple in theory, it quickly became apparent that the dataset had much more diversity in the amount/unit strings than we had originally anticipated, and that any program converting amount/unit strings into values that achieved these goals would require more time to develop than would be realistic with in the scope of P1. The fact that we have to write the project in C further adds to this point.

In the following, we will provide some examples of why the complexity required to reach our data processing goals ultimately becomes so high that it would exceed the scope of our current abilities within the time we have to finish P1.

• The dataset not only contains fractions like 1/2 or ¾, ranges like 1-2, numbers written as words, and unit conversion strings like 175g/6oz, but also combinations of all these, like 1 1/2, 2-1/2, 3-3 1/2, 3 400g Cans, and so forth.
This kind of diversity does not consist of rare corner cases, but is something that most recipes will contain because each recipe has multiple ingredients.
• The dataset has a very large diversity in the spelling and formatting of units, as well as often appending additional text to the end of a unit. For instance, grams can be written as 1g, 1 gram, 100grams, 1 kg, 1 kilograms cans, 1 g of flour dissolved in water, and so on. The same goes for all the non-metric units of weight used in the dataset that also need conversion.
While no single corner case alone would be too complex to handle, the combination of corner cases throughout most recipes in the dataset means that we would not be able to reach our data processing goals within the required time without either:
1. Skipping most of the functionality that we wanted to add to our program.
2. Outsourcing the data processing to a vibe-coded C program, where we understand and document all the overall steps in the data processing but do not ourselves write and fully understand every single command used.

Ultimately, the whole point of doing this data processing step is for us to have a dataset that supports the functionality we want to build and uses units that Danes understand.
With limited time to finish coding the project, we would not even get to work on this functionality if we had to implement the entire data processing ourselves from scratch, thus making the processing somewhat pointless in terms of our final product.
For this reason, after consulting our supervisors, we decided to go with option 2 and outsource the data processing so that we can focus our coding efforts on actually using the data for our project.



## Current programs


**`convert_to_amounts_and_units.c`**

Purpose: 
Convert the raw json file downloaded from TheMealDB into a new json file where the units and amounts can be used for our core R1 functionality such as having understandable units and adding support for portion size. Note that this program is vibe coded due to the the complexity nad sheer number of corner cases it needs to handle compared with the limited time we would have to make it from scratch.

- Converts `opskrifter.json` to a precessed `opskrifter_opdelt.json`.
- Excludes 19 predefined recipes.
- Converts weight and volume to `g` and `ml`, except teaspoons/tablespoons.
- Parses amounts/unit-strings into amount, units and notes.
- Rounds amounts above 5 to the nearest integer.
- Removes empty fields and simplifies names of columns and the structure of the json file. 
- Converts non-metric units in instructions.



**`test_convert_to_amounts_and_units.c`**

Purpose:
To quickly test that convert_tricky_units.c works for the majority of our corner cases even if we end up changing the program in the future.

- Runs the parser from convert_tricky_units.c on predefined cases of 23 ingredient strings and 2 instructions.
- Displays input, amount, unit, note and test results in the terminal.



**`calculate_portions.c`**

Purpose:
Reads `opskrifter_opdelt.json` and adds a `portions` field to each recipe that can be used for our portion size feature later. Note that the program uses very rough estimations that will be off in some cases. The aim is to have an roughly usefull estimate for >80% of recipes which would be in line with what one might ecpect for online estimates.

- Sums ingredient amounts measured in `g` or `ml`, treating 1 ml ≈ 1 g.
- Uses **300 g/ml per person** for regular recipes and **100 g/ml** for desserts.
- Rounds to the nearest whole person, with a minimum of 1.
- Uses `portions: 0` when no measurable amounts are available, indicating an uncertain estimate.



# Future plans:
`convert_to_amounts_and_units` is to complex for us to understand every single command for this project. However we should still understand the overall structure of what is going on. 

The other 2 programs are much simpler in nature and idealy we should impliment our own versions (with the same output) of these rather than the vibe coded versions I made so far. Unlike `convert_to_amounts_and_units` the other 2 programs were only vibe coded in order to have an actually have a finished database we could use so the rest of the project development depending on this would not have to be postponed.