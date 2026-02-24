# Maximum Planar Subset
The Maximum Planar Subset Solver is an efficient computational geometry tool designed to solve the chord selection problem on a circular layout. It utilizes a dynamic programming approach to identify the largest possible subset of chords such that no two selected chords intersect within the circle.

## Problem Formulation
Given a set $C$ of $n$ chords in a circle, where endpoints are numbered from $0$ to $2n-1$ in a clockwise direction, the goal is to compute a planar subset of $C$ with the maximum number of chords. A subset is considered planar if no two chords in the subset overlap or intersect inside the circle.The system implements a recursive DP relation $M(i, j)$ to represent the maximum number of non-overlapping chords in the region formed by the chord $\overline{ij}$ and the arc between endpoints $i$ and $j$. The final objective is to calculate $M(0, 2n-1)$ for a given set of $n$ chords.

## Features
- **Maximum Planar Subset computation**: Uses dynamic programming to compute the maximum number of non-crossing pairs.
- **Efficient memoization**: A 2D DP table is used to avoid redundant subproblem computations.
- **Optional solution recovery (compile-time)**: When enabled via a compile-time flag, the program reconstructs and outputs the selected non-crossing pairs in sorted order.

## Processing Pipeline
1. **Input Parsing**: Read the number of nodes N and pair connections from the input file, storing them in an index mapping (pairIndex).
2. **Dynamic Programming Computation**: Recursively compute the maximum planar subset value for the interval [0, N-1], using a DP table to store intermediate results.
3. **Result Output**: Write the computed maximum number of non-crossing pairs to the output file.
4. **Optional Solution Recovery**: If enabled at compile time, reconstruct the selected pairs by traversing the DP table, sort them, and output the pairs in ascending order.

## Input / Output Format
## Input
**Input.in**
```
<2n_total_endpoints>
<u> <v>
# repeated n times
...
0
```

**Example**
```
10
3 7
2 8
5 6
4 9
0 1
0
```

### Output
**Output.out**
```
<max_subset_size>
```

**Example**
```
4
```

## Environment:
|  Operating System  |  Compiler Version  | C++ Standard |
|--------------------|--------------------|--------------|
| Ubuntu 22.04       |    gcc 9.5.0       |     C++17    |
| Windows 11 (MSYS2) |    gcc 15.1.0      |     C++17    |

## Directory Structure
```
PA1/
  ├── Makefile  // Build script to compile the project
  ├── dataset/  // testcase and golden answer
  ├── main.cpp
  ├── bin/      // Final executable, e.g., bin/MPS
  ├── run.sh    // Shell script to manage testcases
  └── README.md
```
## Usage Guide
### How to compile
To generate the executable `bin/MPS`, simply run
```
make
```
> [!TIP]
> Use `DETAIL=1` to enable detailed output.

### How to execute
Run the program with
```
./bin/MPS <input>.in <output>.out
```
### Utility Scripts
To quickly run, clean, or verify testcases, use `run.sh`.
```
./run.sh <case|all> [check|clean|valgrind]
```
