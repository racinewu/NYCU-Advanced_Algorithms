#!/bin/bash

EVAL=./evaluator-linux-x86_64-v3

# Check example folder exists
if [ ! -d example ]; then
  echo "Error: example directory not found."
  exit 1
fi

has_input=0
for in_file in $(ls example/*.in 2>/dev/null | sort -V); do
  [ -e "$in_file" ] || continue
  has_input=1

  base_name=$(basename "$in_file" .in)
  out_file="example/${base_name}.out"

  if [ -f "$out_file" ]; then
    echo "Evaluating $base_name..."
    "$EVAL" "$in_file" "$out_file"
    echo ""
  else
    echo "Warning: Missing output file for $base_name"
  fi
done

if [ "$has_input" -eq 0 ]; then
  echo "No input files (*.in) found in example directory."
fi
