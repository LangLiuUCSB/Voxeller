# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Source and object files
SRCS = main.cpp VoxelGraph.cpp
OBJS = $(SRCS:.cpp=.o)

# Test files (0=info, 1=random, 2=specific)
TEST_SRCS = test_2.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_TARGETS = $(TEST_SRCS:.cpp=)

# Target executable for main
TARGET = main

# Default target
all: $(TARGET)

# Link the main object files to create the main executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Compile and link each test file separately
$(TEST_TARGETS): %: %.o VoxelGraph.o
	$(CXX) $(CXXFLAGS) -o $@ $^ 

# Compile .cpp files to .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the test executable
test: $(TEST_TARGETS)
	./test_2

# Run the main executable
run: $(TARGET)
	./main

# Run leak check on main executable
leaks: $(TARGET)
	leaks --atExit -- ./main

# Clean up object files and executables
clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGETS)

# Phony targets
.PHONY: all clean run leaks