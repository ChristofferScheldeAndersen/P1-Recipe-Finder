In our initial process of looking for data sets for our P1 program we chose TheMealDB among other reasons because it had already seperated ingridients and amount/unit into two seperate strings making our data processing easier. We planned to write a simple parser program in C that could then split up the strings for amount/unit into amount and unit, with a few additional steps such that we would reach the following goals:

# Data processing goals:
- Amounts can be multiplied with a factor if a user want to make a larger portion of a recipe
- Amounts can be used to make a rough estimate of how many people there are portions for.
- Any units used are converted to units that danes understand rather than oz and pounds. (Ideally also in the instructions part of the recipe)

While the above goals sounds simple in theory it quickly became apparent that the dataset had much more diversity in the amount/unit-strings than we had originally anticipated and that any prograconverting amount/unit-strings to values that that achieved these goals would require more time to make than what would be realistic for the scope of P1. The fact that we have to write the project in C further adds to this point. 

In the following we will provide som examples of why complexity needed to reach our data processing goals ultimatly ends up so high that it would exceed the scope of our current abilities in the time we have to finish P1.
- The dataset not only contains fractions like 1/2 or ¾, ranges like 1-2, numbers as words, unit conversion strings like "175g/6oz", but also combinations of all these like "1 1/2", 2-1/2, 3-3 1/2, "3 400g Cans" and so forth. This kind of diversity isn't rare corner cases but something that most recipes will contain due to each recipe having multiple ingredients.
- The dataset has very large diversity in spelling and formatting of units as well as most often appending additional text to the end of any unit. For instance grams can be written both as "1g", "1 gram", "100grams", "1 kg", "1 kilograms cans", "1 g of flour disolved in water", and so on. And the same goes for all the non metric units of wheights used in the dataset that needs conversion as well. 

While no single corner case alone would be too complex to handle, the combination of corner cases throughout most recipes in the dataset means that we would not be able to reach our goals for the data processing in the required time without either:
1. Skipping most of the functionality that we wanted to add to our program 
2. Outsourcing the data processing to a vibe coded C program where we understand and document all the overall steps in the data proccesing but won't ourselves write and fully understand every single command used.

Ultimatly we propose a middle ground where we go with option 2 with the addition of us writing a much simplified version of the same program to ensure we get some hands on experience with coding such a program in C. Our simplified program starts by removing around 60% of the recipes, then proceeds to split the amount/unit-strings into 2 seperate values for amount and unit and finally saves the resulting json. Additionally we would also write our own tests for this program allowing us to get experience in that area as well.
We feel like this approach would reach a nice balance of getting hands on experience writing c code for data processing, while still allowing us to work on the features which are the reason we want to procces our data in the first place.

