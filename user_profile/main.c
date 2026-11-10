int main(void)
{
    user users[20];
    int count = read_users("brugere.txt", users, 20);

    printf("Read %d profiles\n", count);
    for (int i = 0; i < count; i++)
    {
        printf("%d. %s | allergies: %s | diet: %s | cuisines: %s | categories: %s\n",
               i + 1, users[i].name, users[i].allergies, users[i].diet,
               users[i].favourite_cuisines, users[i].favourite_categories);
    }
    return 0;
}