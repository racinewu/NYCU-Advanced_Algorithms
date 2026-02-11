# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
CXXFLAGS += -DDETAIL=0

TARGET = bin/MPS
SRC = main.cpp

all: $(TARGET)

$(TARGET):
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean
clean:
	rm -rf bin

.PHONY: all clean
