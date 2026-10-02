CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Iinclude
TARGET = JackAnalyzer
SRCS = $(wildcard src/*.cpp)

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
