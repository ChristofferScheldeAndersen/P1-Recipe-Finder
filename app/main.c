#include <stdio.h>
#include "backend.h"
#include "ui.h"

// Main

int main(void) {
    cJSON *root = load_recipes("recipes_with_portions.json");
    cJSON *meals = cJSON_GetObjectItem(root, "meals");
    StringList categories = recipe_options(meals, "category");
    StringList areas = recipe_options(meals, "area");

    User users[20] = {0};
    int count = read_users("brugere.txt", users, 20);
    int profile = ui_profile(users, count);
    if (profile < 0 || profile > count + 1) return 1;

    UserPreferences current = {0};
    User *selected_user = NULL;
    if (profile == 1) {
        if (count >= 20) return 1;
        create_user(&users[count]);
        selected_user = &users[count];
        selected_user->preferences = ui_choose_preferences(current, categories, areas);
        current = preferences_copy(selected_user->preferences);
        count++;
        save_all_users("brugere.txt", users, count);
    } else if (profile >= 2) {
        selected_user = &users[profile - 2];
        current = preferences_copy(selected_user->preferences);
    }

    StringList all_chosen = ui_choose_ingredients();
    UserPreferences preferences = current;
    char answer[20];
    ask("Do you want to enter preferences? (y/n): ", answer, sizeof(answer));
    if (answer[0] == 'y' || answer[0] == 'Y') {
        preferences = ui_choose_preferences(current, categories, areas);
        preferences_free(current);

        if (selected_user != NULL) {
            ask("Save preferences to your profile? (y/n): ", answer, sizeof(answer));
            if (answer[0] == 'y' || answer[0] == 'Y') {
                preferences_free(selected_user->preferences);
                selected_user->preferences = preferences_copy(preferences);
                save_all_users("brugere.txt", users, count);
            }
        }
    }

    if (root != NULL) {
        cJSON *allowed_recipes = filter_allowed_recipes(meals, preferences);

        int number = 3;
        RecipeResult best[number];
        choose_best_recipes(allowed_recipes, all_chosen, preferences, number, best);
        ui_show_best_recipes(best, number);

        for (int i = 0; i < number; i++) {
            string_free(best[i].matching);
        }
        cJSON_Delete(allowed_recipes);
        cJSON_Delete(root);
    }

    string_free(all_chosen);
    preferences_free(preferences);
    for (int i = 0; i < count; i++) preferences_free(users[i].preferences);
    string_free(categories);
    string_free(areas);
    return 0;
}
