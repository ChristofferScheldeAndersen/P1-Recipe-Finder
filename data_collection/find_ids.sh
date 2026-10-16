#!/usr/bin/env bash
set -euo pipefail

base="https://www.themealdb.com/api/json/v1/1"

curl -fsS "$base/categories.php" |
    jq -r '.categories[].strCategory' |
    tr -d '\r' > kategorier.txt

while read -r kategori
do
    curl -fsS --get "$base/filter.php" --data-urlencode "c=$kategori" |
        jq -er '.meals[].idMeal'
done < kategorier.txt | tr -d '\r' | sort -u > ids.txt

echo "Antal fundne ID'er:"
wc -l < ids.txt