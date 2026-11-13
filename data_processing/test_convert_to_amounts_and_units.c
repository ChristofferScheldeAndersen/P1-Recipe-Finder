/* Build: gcc test_convert_to_amounts_and_units.c -o test_convert_to_amounts_and_units -lm
   Requires convert_to_amounts_and_units.c, cJSON.c and cJSON.h. */
#define main recipe_main
#include "convert_to_amounts_and_units.c"
#undef main

int main(void) {
    const struct {const char *input; int has_amount; double amount; const char *unit, *note;} tests[] = {
        {"1 (400g) tin",1,400,"g","1 tin"},
        {"2 (400g) tins",1,800,"g","2 tins"},
        {"3 400g cans",1,1200,"g","3 400g cans"},
        {"2 x 500g",1,1000,"g","2 x 500g"},
        {"1 can (400g)",1,400,"g","1 can (400g)"},
        {"2 tins of 400g",1,800,"g","2 tins of 400g"},
        {"1.5 kg",1,1500,"g",""},
        {"2 dl",1,200,"ml",""},
        {"1 l",1,1000,"ml",""},
        {"400 g drained weight",1,400,"g","drained weight"},
        {"200-300 g",1,250,"g","200-300 g"},
        {"1-2 eggs",1,1.5,"eggs","1-2 eggs"},
        {"12.5 g",1,13,"g",""},
        {"0.5-0.75 tsp",1,.625,"teaspoons","0.5-0.75 tsp"},
        {"1 ½ cups",1,360,"ml",""},
        {"2 tbsp",1,2,"tablespoons",""},
        {"1/2 oz",1,14,"g",""},
        {"5.4 g",1,5,"g",""},
        {"4.9 g",1,4.9,"g",""},
        {"5.5 g",1,6,"g",""},
        {"2.5 oz",1,71,"g",""},
        {"pinch",0,0,"pinch",""},
        {"to taste",0,0,"","to taste"}
    };
    const struct {const char *input,*expected;} instructions_tests[] = {
        {"Use 2 tbsp oil and 1/2 cup milk.","Use 2 tbsp oil and 120 ml milk."},
        {"Bake in a 9-inch pan. Add 3 oz cheese.","Bake in a 23 cm pan. Add 85 g cheese."}
    };
    int failed=0;
    const size_t n=sizeof(tests)/sizeof(tests[0]);
    puts("=== INGREDIENTS: INPUT AND PARSED OUTPUT ===");
    printf("%-3s %-24s %12s %-16s %-28s %s\n", "No", "INPUT", "AMOUNT", "UNIT", "NOTE", "TEST");
    puts("---------------------------------------------------------------------------------------------------");
    for(size_t i=0;i<n;i++){
        Parsed p=parse(tests[i].input);
        int a_ok=(p.has_amount==tests[i].has_amount) &&
            (!p.has_amount || (p.amount-tests[i].amount<1e-6 && tests[i].amount-p.amount<1e-6));
        int u_ok=!strcmp(p.unit,tests[i].unit);
        int no_ok=!strcmp(p.note,tests[i].note);
        int ok=a_ok && u_ok && no_ok;
        char amount[32];
        if(p.has_amount)snprintf(amount,sizeof amount,"%.10g",p.amount);
        else strcpy(amount,"-");
        printf("%02zu  %-24s %12s %-16s %-28s %s\n",i+1,tests[i].input,
            amount,p.unit[0]?p.unit:"-",p.note[0]?p.note:"-",ok?"PASS":"FAIL");
        if(!ok){
            printf("    EXPECTED: amount=%s",tests[i].has_amount?"":"-");
            if(tests[i].has_amount)printf("%.10g",tests[i].amount);
            printf(" | unit=%s | note=%s\n",tests[i].unit[0]?tests[i].unit:"-",
                tests[i].note[0]?tests[i].note:"-");
            printf("    MISMATCH: %s%s%s\n",a_ok?"":"amount ",u_ok?"":"unit ",no_ok?"":"note");
            failed++;
        }
    }
    const size_t m=sizeof(instructions_tests)/sizeof(instructions_tests[0]);
    puts("\n=== INSTRUCTIONS: INPUT OG OUTPUT ===");
    for(size_t i=0;i<m;i++){
        char *got=instructions(instructions_tests[i].input);
        int ok=!strcmp(got,instructions_tests[i].expected);
        printf("\n[%02zu] %s\n  INPUT : %s\n  OUTPUT: %s\n",i+1,ok?"PASS":"FAIL",instructions_tests[i].input,got);
        if(!ok){printf("  EXPECTED: %s\n",instructions_tests[i].expected);failed++;}
        free(got);
    }
    printf("\n=== RESULT: %zu tests, %d failures ===\n",n+m,failed);
    return failed?EXIT_FAILURE:EXIT_SUCCESS;
}