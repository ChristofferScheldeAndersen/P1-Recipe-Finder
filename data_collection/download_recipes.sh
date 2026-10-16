#!/usr/bin/env bash
set -euo pipefail

base="https://www.themealdb.com/api/json/v1/1"
total=$(wc -l < ids.txt)
antal=0

while read -r id
do
    curl -fsS "$base/lookup.php?i=$id"
    echo
    antal=$((antal + 1))
    echo "Hentet $antal/$total" >&2
done < ids.txt > svar.jsonl

jq -s '{meals: [.[].meals[]]}' svar.jsonl > opskrifter.json