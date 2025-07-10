#!/bin/bash

# Check executable file exists
if [ ! -x ./bin/Chip_Router ]; then
  echo "Error: ./bin/Chip_Router not found or not executable. Please compile the project first."
  exit 1
fi

# Check example folder exists
if [ ! -d example/ ]; then
  echo "Error: example/ directory not found."
  exit 1
fi

# run through all case
found_any=0
for file in $(ls example/*.in | sort -V); do
  [ -e "$file" ] || continue
  found_any=1
  base=$(basename "$file" .in)
  echo -e "\nRunning $base..."
  ./bin/Chip_Router "$file" "example/${base}.out"
done

if [ "$found_any" -eq 0 ]; then
  echo "No .in files found in example/"
fi

echo -e "\nFinished running examples."

