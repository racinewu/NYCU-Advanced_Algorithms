#!/bin/bash

# Config
TESTCASE_DIR="testcase"
TARGET="./bin/Chip_Router"
CHECKER="python3 evaluator_pure_v1.py"

# Colors
BLUE="\033[34m"
PURPLE="\033[35m"
GREEN="\033[32m"
RED="\033[31m"
YELLOW="\033[33m"
RESET="\033[0m"

# Logging functions
log_info()    { printf "[${BLUE}INFO${RESET}] %s\n" "$*"; }
log_error()   { printf "\n[${RED}ERROR${RESET}] %s\n" "$*"; }
log_missing() { printf "\n[${PURPLE}MISSING${RESET}] %s\n" "$1"; }

usage() {
    cat <<EOF
Usage: $0 <case|all> [check|clean|valgrind]

Examples:
  $0 case1           # run a single case
  $0 all             # run all cases
  $0 case1 check     # check a single case
  $0 all clean       # clean all outputs
  $0 case1 valgrind  # run with valgrind
EOF
    exit 1
}

run_case() {
    local CASE=$1
    local IN_FILE="$TESTCASE_DIR/${CASE}.in"
    local OUT_FILE="$TESTCASE_DIR/${CASE}.out"

    if [[ ! -f "$IN_FILE" ]]; then
        log_error "$IN_FILE: No such case"
        return
    fi
    printf "\n"
    log_info "Running case: $CASE ..."
    if [[ "$MODE" == "valgrind" ]]; then
        valgrind --leak-check=full --show-leak-kinds=all "$TARGET" "$IN_FILE" "$OUT_FILE"
    else
        /usr/bin/time -f "Real: %e s, User: %U s, Sys: %S s, CPU%%: %P, MaxMem: %M KB" "$TARGET" "$IN_FILE" "$OUT_FILE"
    fi
    log_info "Finished running case: $CASE."
}

check_case() {
    local CASE=$1
    local IN_FILE="$TESTCASE_DIR/${CASE}.in"
    local OUT_FILE="$TESTCASE_DIR/${CASE}.out"

    if [[ ! -f "$IN_FILE" ]]; then
        log_error "$IN_FILE: No such case"
        return
    fi

    if [[ ! -f "$OUT_FILE" ]]; then
        log_missing "$CASE.out"
        return
    fi

    printf "\n"
    log_info "Checking case: $CASE ..."
    $CHECKER "$IN_FILE" "$OUT_FILE"
    log_info "Finished checking case: $CASE."
}

clean_case() {
    printf "\n"
    if [[ "$1" == "all" ]]; then
        log_info "Cleaning all .out files in $TESTCASE_DIR ..."
        rm -f "$TESTCASE_DIR"/*.out
        log_info "Clean complete."
    else
        local FILE="$TESTCASE_DIR/${1}.out"
        if [[ -f "$FILE" ]]; then
            log_info "Cleaning $FILE ..."
            rm -f "$FILE"
            log_info "Clean complete."
        else
            log_info "$FILE does not exist."
        fi
    fi
}

# Check target existence unless cleaning
if [[ "$2" != "clean" && ! -x "$TARGET" ]]; then
    log_error "$TARGET not found or not executable."
    printf "Please build it first (e.g. make).\n"
    exit 1
fi

# Parameter check
if [[ $# -lt 1 || $# -gt 2 ]]; then
    usage
fi

CASE=$1
MODE=$2

if [[ "$MODE" == "clean" ]]; then
    clean_case "$CASE"
    exit 0
fi

if [[ "$CASE" == "all" ]]; then
    shopt -s nullglob
    CASES=("$TESTCASE_DIR"/*.in)
    shopt -u nullglob

    if [[ ${#CASES[@]} -eq 0 ]]; then
        log_error "No .in files found in $TESTCASE_DIR"
        exit 1
    fi

    for file in $(printf "%s\n" "${CASES[@]}" | sort -V); do
        casename=$(basename "$file" .in)
        if [[ "$MODE" == "check" ]]; then
            check_case "$casename"
        else
            run_case "$casename"
        fi
    done
    printf "\n"
    log_info "Finished all cases."
else
    if [[ "$MODE" == "check" ]]; then
        check_case "$CASE"
    else
        run_case "$CASE"
    fi
fi
