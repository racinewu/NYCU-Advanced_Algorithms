# Single Layer ChipRouter


## Directory Structure
```
PA2/
  ├── Makefile                  # Build script to compile the project
  ├── visualizer.py             # Python script to visualize routing result
  ├── verifier-linux            # Executable to verify output correctness
  ├── testcase/                 # Input (.in) and expected output (.out) files
  │
  ├── shell_run_all.sh          # Shell script to run all testcases
  ├── shell_plot_all.sh         # Shell script to plot all results
  │
  ├── include/                  # Header files
  │   ├── router.h
  │   ├── solver.h
  │   └── types.h
  │
  ├── src/                      # C++ source files
  │   ├── main.cpp              # Main entry point
  │   ├── router.cpp            # Routing algorithm implementation
  │   └── solver.cpp            # Problem solver and result handling
  │
  ├── build/                    # Object files (.o), created after make
  └── bin/                      # Final executable, e.g., `bin/Chip_Router.exe`
```
## How to compile
To generate the executable `bin/Chip_Router`, simply run
```
make
```
## How to execute
Run the program with
```
./bin/Chip_Router <input.in> <output.out>
```
## How to verify
To verify the output with provided verifier
```
./evaluator-linux-x86_64-v3 <input.in> <output.out>
```
## How to plot
To visualize the result (requires Python and matplotlib)
```
python3 visualizer.py
```
## Utility Scripts
To solve/verify all testcase
```
./run.sh
./evaluator.sh
```
