CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Iinclude

SRC = src/main.cpp src/router.cpp src/solver.cpp
OBJ = $(patsubst src/%.cpp, build/%.o, $(SRC))
TARGET = bin/Chip_Router

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $@ $^

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build bin

.PHONY: all clean
