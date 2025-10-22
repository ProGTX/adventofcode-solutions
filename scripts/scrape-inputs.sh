#!/usr/bin/env bash
set -euo pipefail

# Requires aoc-cli Rust crate to be installed

# Important: Set ADVENT_OF_CODE_SESSION env var to the session cookie
#   from your browser before running this script.
#   You need to be logged into Advent of Code in the browser.
#   https://www.cookieyes.com/blog/how-to-check-cookies-on-your-website-manually/

# Determine the directory where this script resides
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.."

years=(2021 2022 2023 2024)
days=$(seq 1 25)

# Generate (year, day) pairs
for y in "${years[@]}"; do
  for d in $days; do
    printf "%s %02d\n" "$y" "$d"
  done
done | xargs -n2 -P8 bash -c '
  year="$0"
  day="$1"

  # Make sure output folder exists
  mkdir -p "inputs/$year"

  outfile="inputs/$year/day${day}.input"

  # Skip if file already exists
  if [[ -f "$outfile" ]]; then
    echo "Skipping $outfile (already exists)"
    exit 0
  fi

  aoc download -I -y "$year" -d "$day" -i "$outfile"
'
