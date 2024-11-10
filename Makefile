# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3

# Source and object files
SRCS = main.cpp VoxelGraph.cpp
OBJS = $(SRCS:.cpp=.o)

# Test files (0=info, 1=random, 2=specific)
TEST_SRCS = test_2.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGETS = $(TEST_SRCS:.cpp=)

# Target executable for main
TARGET = main

all: $(TARGET)# Default target

$(TARGET): $(OBJS) # Link the main object files to create the main executable
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

$(TEST_TARGETS): %: %.o VoxelGraph.o # Compile and link each test file separately
	$(CXX) $(CXXFLAGS) -o $@ $^ 

%.o: %.cpp # Compile .cpp files to .o files
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGETS) # Run the test executable
	./test_2

run: $(TARGET) # Run the main executable
	./main

leaks: $(TARGET) # Run leak check on main executable
	leaks --atExit -- ./main

clean:# Clean up object files and executables
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGETS)

.PHONY: # Phony targets
	all clean run leaks