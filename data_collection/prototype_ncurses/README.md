# Madplan prototype (ncurses)

The same meal planner as `../prototype`, but menu driven with ncurses. You move with the arrow keys
and press Enter, and ingredients are picked from a list that filters as you type, so they can't be
misspelled. Comments and screen text are in Danish.

## Build and run

```bash
cd prototype_ncurses
cc -Wall -Wextra -o madplan *.c -lncurses -lm      # macOS
cc -Wall -Wextra -o madplan *.c -lncursesw -lm     # Linux (needs libncurses-dev)
./madplan                                          # reads ../opskrifter_normaliseret2.json
```

## Keys

| Screen | Keys |
|---|---|
| Main menu | ↑↓ move, Enter open, ←→ change number of people, q quit |
| Køleskab / Undgå | type to filter, ↑↓ move, Enter pick or unpick, Backspace delete a letter, Esc back |
| Præferencer | ↑↓ move, Enter or space toggle, Esc back |
| Forslag | ↑↓ move, Enter open recipe, Esc back |
| Opskrift | ↑↓ PgUp PgDn scroll, i add missing items to the shopping list, Esc back |
| Indkøbsliste | ↑↓ move, d remove a line, g save to `indkoebsliste.txt`, Esc back |

## Files

| File | What it does |
|---|---|
| `madplan.h` | Structs and the list of functions |
| `main.c` | Loads the data, draws the main menu, sends you to the other screens |
| `skaerm.c` | Every ncurses screen: ingredient picker, preferences, suggestions, recipe, shopping list |
| `indlaes.c` | Reads the JSON, and builds the sorted list of the 948 different ingredient names |
| `tekst.c` | Text helpers, including `ord_starter_med` for the live filter |
| `kost.c` | Diet and allergy rules (unchanged from the first prototype) |
| `planlaeg.c` | Ranking, scaling and the shopping list. It only calculates and never draws |

## Simplifications

- Every recipe counts as 4 servings (the data has no servings).
- Diet info is guessed from ingredient names.
- The ingredient list is the raw names from TheMealDB, so `onion`, `onions` and `red onions` are separate
  entries. Picking `onion` also matches recipes that use `Onions` or `Red Onions`.
