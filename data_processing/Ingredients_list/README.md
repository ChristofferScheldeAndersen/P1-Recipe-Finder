# ingredients

A small C program that reads the recipe file `opskrifter.json` (TheMealDB format) and writes a sorted list of every **unique ingredient** in it, one per line.

The list is used by the meal planner to compare the user's ingredients against the recipes and to power a live ingredient search.

## Build

```
gcc -Wall -Wextra -o ingredients ingredients.c
```

While developing, add the AddressSanitizer to catch memory bugs (slower, so leave it out of the final build):

```
gcc -Wall -Wextra -g -fsanitize=address -o ingredients ingredients.c
```

## Usage

The program reads from standard input and writes to standard output:

```
./ingredients < opskrifter.json > ingredients.txt
```

Two status lines are printed to standard error (so they don't end up in the output file):

```
8179 names stored
<N> unique names
```

## Output format

A plain text file with one ingredient per line, all lowercase, sorted alphabetically, no duplicates:

```
bacon
basil
olive oil
salt
```

A text file was chosen over JSON because the meal planner is also written in C: each line can be read with `fgets`, so no JSON parser is needed.

## How names are cleaned

Two ingredient names count as the same if they only differ in letter case or in spaces at the start or end. Every name is therefore:

1. **Trimmed** of leading and trailing whitespace (`" Salt "` → `"Salt"`)
2. **Lowercased** (`"Salt"` → `"salt"`)

Empty values (`""`), whitespace-only values and `null` values are skipped.

The meal planner should clean user input with the same two rules, so a plain `strcmp` is enough to compare.

## Input validation

If the input does not contain both `"meals"` and `"strIngredient1"`, the program prints an error to standard error and exits with code `1` without writing any output.

## How it works (short version)

1. Read all of standard input into one growing buffer (`malloc` / `realloc`).
2. Check that the input looks like the recipe file.
3. Walk through the buffer with a pointer, using `strstr` to find each `"strIngredientN"` key and `strchr` to find its value.
4. Trim and lowercase each value into its own string and store it in a growing array of strings.
5. Sort the array with `qsort`.
6. Print each name only if it differs from the one before it (duplicates sit next to each other after sorting).
7. Free all memory.

## Testing

```
# output should contain no duplicates and be alphabetical
./ingredients < opskrifter.json > ingredients.txt
head -30 ingredients.txt

# line count should match the "unique names" number
wc -l ingredients.txt

# a common ingredient should appear exactly once
grep -x "olive oil" ingredients.txt

# wrong input should give an error and exit code 1
echo "hello" > fake.json
./ingredients < fake.json
echo $?
```

The growth logic was also stress-tested by temporarily setting the starting capacities very low (`list_cap = 2`) and checking with `diff` that the output was unchanged.

## Known limitations

- **Escaped quotes** (`\"`) inside an ingredient name are not handled; the name would be cut at the escaped quote. The current dataset does not contain any.
- **Near-duplicates** are kept as separate entries, e.g. `egg` / `eggs` or `onion` / `chopped onion`. They are genuinely different strings in the data; how smart the matching should be is a decision for the meal planner.
- **Lowercasing** uses `tolower`, which only changes plain ASCII letters. Letters like `Ø` or `É` keep their case.
- If memory runs out in the middle of reading, names already stored in the list are not freed before exiting (the operating system reclaims them). A `free_list` helper would fix this.
