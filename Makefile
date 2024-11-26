CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

SRCS = test.cpp Lattice.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./test

leaks: $(TARGET)
	leaks --atExit -- ./main

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGETS)

.PHONY:
	all clean run leaks