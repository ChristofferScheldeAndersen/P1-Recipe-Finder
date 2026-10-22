# Opskriftsprogram

Startfilen er **opskrifter.c**. Den læser **opskrifter.json**, fjerner de 19 valgte opskrifter i hukommelsen og behandler de resterende 774 opskrifter. Slutresultatet gemmes som **opskrifter_opdelt.json**. Originalfilen ændres ikke, og der gemmes ingen filtreret mellemfil.

Programmet udskriver stadig 22 måleksempler og et instruktionseksempel. Det laver opdeling i mængde/enhed/note, omregning, afrunding og portionsestimat som den tidligere anbefalede version.

## Filer

- `opskrifter.c`: hele det opdaterede program med de seks filtreringslinjer.
- `cJSON.c` og `cJSON.h`: JSON-biblioteket, som kompileres sammen med programmet.
- `LICENSE-cJSON.txt`: bibliotekets licens.
- `opskrifter.json`: originalfilen med alle 793 opskrifter.
- `tests.py`: automatiske tests uden ekstra Python-pakker.
- `tests/forventet_output.json`: forventet resultat fra den tidligere validerede version på de 774 beholdte opskrifter.
- `TESTRESULTAT.txt`: testresultatet fra denne pakke.

## Kompilér og kør

Pak ZIP-filen ud. Åbn en terminal i den udpakkede mappe, hvor `opskrifter.c` og `opskrifter.json` ligger. GCC skal være installeret og tilgængelig i terminalen.

Linux:

```sh
gcc -std=c11 -Wall -Wextra opskrifter.c cJSON.c -lm -o opskrifter
./opskrifter
```

Windows med MSYS2 UCRT64-terminal:

```sh
gcc -std=c11 -Wall -Wextra opskrifter.c cJSON.c -lm -o opskrifter.exe
./opskrifter.exe
```

I PowerShell starter du i stedet programmet med `./opskrifter.exe`, efter at samme GCC-kommando er kørt i en terminal, der har GCC i PATH.

Der medfølger kildekode; du kompilerer programmet på din egen computer.

## Kør tests

Kræver Python 3 og GCC i samme terminals PATH. Ingen `pip install` er nødvendig.

Linux og MSYS2 med Python:

```sh
python3 tests.py
```

Windows med Python-launcheren og GCC i PATH:

```sh
py tests.py
```

Testene kompilerer selv programmet og arbejder i midlertidige mapper. De ændrer ikke dine input- eller outputfiler. Testene kontrollerer blandt andet 793→774 opskrifter, samtlige outputfelter mod det forventede resultat, præcise ID-match, gentagne ID'er, afrunding, decimalintervaller, noter, inches, portionsestimat, JSON og de udskrevne eksempler.

## Inputområde

Dette er versionen til den medfølgende opskriftsfil. Alle opskrifter skal have `idMeal` som en kort tekststreng, ligesom i originalfilen. De 19 ID'er står i `main()`.

Programmet er fortsat forenklet ud fra fravalget af disse opskrifter: det understøtter ikke sammenhængende dimensioner som `9x13-inch`, Unicode-brøkstregen `⁄` eller prioritering af et efterfølgende metrisk alternativ som i `6oz/180g`. Almindelige brøker, pint/quart og enkeltstående inches understøttes stadig. Hvis der senere tilføjes nye opskrifter, skal deres formater vurderes igen.

Pakken er testet på Linux med GCC. Windows-kommandoerne er medtaget, men Windows er ikke testet i denne kørsel.
