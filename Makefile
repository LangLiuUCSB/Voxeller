CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

SRCS = main.cpp Lattice.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./main

leaks: $(TARGET)
	leaks --atExit -- ./main

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGETS)

.PHONY:
	all clean run leaks