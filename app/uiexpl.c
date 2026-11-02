
#include <stdio.h>               // Provides puts(), for normal text output
#include <ncurses/ncurses.h>     // Provides ncurses functions and types

int main(void) {

    // =====================================================
    // PART 1: INITIALIZE NCURSES
    // =====================================================

    // Start ncurses. This must happen before we use
    // ncurses windows, screen dimensions, or keyboard input.
    initscr();

    // Make keyboard input available immediately,
    // without needing to press Enter.
    cbreak();

    // Do not automatically print the keys we press.
    noecho();

    // Hide the blinking terminal cursor.
    // 0 = invisible, 1 = normal, 2 = highly visible.
    curs_set(0);


    // =====================================================
    // PART 2: CHECK TERMINAL SIZE
    // =====================================================

    // LINES and COLS are supplied by ncurses.
    // They tell us how many rows and columns
    // the terminal currently has.
    //
    // || means OR.
    //
    // If there are fewer than 16 rows OR
    // fewer than 65 columns, the program stops.

    if (LINES < 16 || COLS < 65) {

        // Restore normal terminal behavior.
        endwin();

        // puts() is a regular C function from stdio.h.
        puts("Terminal too small! Need at least 65x16.");

        // Return 1 to indicate that something went wrong.
        return 1;
    }


    // =====================================================
    // PART 3: SET UP COLORS
    // =====================================================

    // has_colors() returns true if the terminal
    // supports ncurses colors.
    if (has_colors()) {

        // Enable ncurses color functionality.
        start_color();

        // Define color pair number 1:
        // White text on a black background.
        init_pair(1, COLOR_WHITE, COLOR_BLACK);

        // Apply that color pair to the main screen.
        bkgd(COLOR_PAIR(1));
    }


    // =====================================================
    // PART 4: CREATE OUR WINDOWS
    // =====================================================

    // WINDOW is a type supplied by ncurses.
    //
    // The * means we are declaring a POINTER.
    // A pointer stores a reference to something.
    //
    // Here, menu points to an ncurses window.
    //
    // newwin() creates the window and returns
    // a pointer to it.
    //
    // newwin(height, width, y, x)
    //
    // Height and width specify its dimensions.
    // y and x specify its position on the terminal.

    WINDOW *menu = newwin(
        LINES - 4,  // Height: terminal height minus 4
        22,         // Width: 22 characters
        2,          // Y: 2 rows down
        2           // X: 2 columns right
    );

    // Our second window starts further to the right.
    //
    // COLS - 27 lets its width adapt to the
    // width of the terminal when the program starts.

    WINDOW *info = newwin(
        LINES - 4,  // Same height as menu
        COLS - 27,  // Fill the remaining horizontal space
        2,          // Same vertical position
        25          // Start at column 25
    );

    // Apply our white-on-black color pair to both
    // windows, if terminal colors are supported.
    if (has_colors()) {
        wbkgd(menu, COLOR_PAIR(1));
        wbkgd(info, COLOR_PAIR(1));
    }

    // Enable special keys (like arrow keys) when
    // reading keyboard input from the menu window.
    keypad(menu, TRUE);


    // =====================================================
    // PART 5: STORE THE TUTORIAL TEXT
    // =====================================================

    // These are ARRAYS of strings.
    //
    // An array can hold several values in one variable.
    //
    // Each array element has an INDEX.
    // IMPORTANT: C array indices start at ZERO!
    //
    // topics[0] is "newwin()"
    // topics[1] is "box()"
    // topics[2] is "mvwprintw()"
    //
    // const char * means a pointer to text that
    // we are not going to modify.

    const char *topics[] = {
        "newwin()",
        "box()",
        "mvwprintw()",
        "werase()",
        "wrefresh()",
        "wgetch()",
        "delwin()"
    };

    // These explanations correspond to the topics.
    //
    // For example:
    // topics[0] and descriptions[0] belong together.

    const char *descriptions[] = {
        "Creates a rectangular window.",
        "Draws a border around a window.",
        "Prints text inside a window.",
        "Clears a window's contents.",
        "Displays changes on the screen.",
        "Reads keyboard input from a window.",
        "Deletes a window when finished."
    };

    // Another array showing example syntax.

    const char *examples[] = {
        "newwin(height, width, y, x)",
        "box(window, 0, 0)",
        "mvwprintw(window, y, x, text)",
        "werase(window)",
        "wrefresh(window)",
        "wgetch(window)",
        "delwin(window)"
    };


    // =====================================================
    // PART 6: KEEP TRACK OF THE SELECTED ITEM
    // =====================================================

    // An ordinary integer variable.
    //
    // 0 = first menu item
    // 1 = second menu item
    // ...
    // 6 = seventh menu item

    int selected = 0;


    // =====================================================
    // PART 7: THE MAIN PROGRAM LOOP
    // =====================================================

    // while (1) creates a loop that runs continuously.
    //
    // The number 1 means TRUE in C.
    //
    // We will use "break" to exit the loop
    // when the user presses Q.

    while (1) {

        // ---------------------------------------------
        // 7A: DRAW THE MAIN SCREEN
        // ---------------------------------------------

        // erase() clears the standard screen (stdscr).
        // stdscr is the default window from initscr().
        erase();

        // mvprintw(y, x, text)
        //
        // Prints text at a position on stdscr.
        // First coordinate = row (y)
        // Second coordinate = column (x)

        mvprintw(0, 2, "NCURSES WINDOW TUTORIAL");

        // LINES - 1 is the last terminal row.
        // We subtract 1 because rows start at zero.
        mvprintw(LINES - 1, 2, "UP/DOWN: Navigate | Q: Quit");

        // Show our changes to the standard screen.
        refresh();


        // ---------------------------------------------
        // 7B: CLEAR AND DRAW BOTH WINDOWS
        // ---------------------------------------------

        // werase() is similar to erase(), but it
        // works on a specific window.
        //
        // We clear old content before redrawing.

        werase(menu);
        werase(info);

        // box() creates a border around a window.
        //
        // The two zeros tell ncurses to use its
        // default border characters.

        box(menu, 0, 0);
        box(info, 0, 0);

        // mvwprintw() is similar to mvprintw().
        //
        // The extra 'w' means we specify a WINDOW.
        //
        // mvwprintw(window, y, x, format, ...)
        //
        // Coordinates are RELATIVE to that window,
        // rather than the entire terminal.

        mvwprintw(menu, 0, 2, " FUNCTIONS ");
        mvwprintw(info, 0, 2, " EXPLANATION ");


        // ---------------------------------------------
        // 7C: DRAW THE MENU ITEMS
        // ---------------------------------------------

        // A for-loop repeats code a certain
        // number of times.
        //
        // int i = 0     Start counting at 0.
        // i < 7         Continue while i is below 7.
        // i++           Increase i by 1 each time.
        //
        // So i takes the values 0,1,2,3,4,5,6.

        for (int i = 0; i < 7; i++) {

            // == means "is equal to".
            //
            // If the current item is selected,
            // enable reverse-video highlighting.

            if (i == selected) {
                wattron(menu, A_REVERSE);
            }

            // Display the current array element.
            //
            // i + 2 sets its vertical position.
            //
            // "%s" is a placeholder for a string.
            //
            // "%-16s" means display a string in a
            // 16-character-wide, left-aligned field.

            mvwprintw(menu, i + 2, 2,
                      "%-16s", topics[i]);

            // Turn highlighting off again.
            //
            // Without this, later text could
            // also appear highlighted.

            if (i == selected) {
                wattroff(menu, A_REVERSE);
            }
        }


        // ---------------------------------------------
        // 7D: DRAW THE EXPLANATION WINDOW
        // ---------------------------------------------

        // Use selected as an array index.
        //
        // If selected is 2, these lines display
        // information about mvwprintw().

        mvwprintw(info, 2, 2,
                  "%s", topics[selected]);

        mvwprintw(info, 4, 2,
                  "%s", descriptions[selected]);

        mvwprintw(info, 6, 2,
                  "%s", examples[selected]);

        mvwprintw(info, 9, 2,
                  "This panel is a WINDOW!");


        // ---------------------------------------------
        // 7E: DISPLAY THE TWO WINDOWS
        // ---------------------------------------------

        // Changes made to windows aren't necessarily
        // visible immediately.
        //
        // wrefresh() updates the terminal with
        // the contents of the specified window.

        wrefresh(menu);
        wrefresh(info);


        // ---------------------------------------------
        // 7F: WAIT FOR KEYBOARD INPUT
        // ---------------------------------------------

        // wgetch(menu) waits for a keypress.
        //
        // It returns an integer representing
        // the key that was pressed.
        //
        // We store that value in "key".

        int key = wgetch(menu);

        // If the user pressed lowercase or
        // uppercase Q, exit the while-loop.
        //
        // || means OR.

        if (key == 'q' || key == 'Q') {
            break;
        }

        // KEY_UP is provided by ncurses.
        //
        // && means AND: both conditions must be true.
        //
        // We only move up if we're not already
        // at the first menu item.

        if (key == KEY_UP && selected > 0) {
            selected--;
        }

        // Similarly, move down only if we're
        // not at the last menu item.
        //
        // selected++ means selected = selected + 1.

        if (key == KEY_DOWN && selected < 6) {
            selected++;
        }

        // The while-loop now starts over.
        // Everything gets redrawn with the
        // new selected value.
    }


    // =====================================================
    // PART 8: CLEAN UP AND EXIT
    // =====================================================

    // Release the two windows we created.

    delwin(menu);
    delwin(info);

    // Exit ncurses mode and restore the terminal.
    endwin();

    // Return 0 to indicate successful completion.
    return 0;
}
