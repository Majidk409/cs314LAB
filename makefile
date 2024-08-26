# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# Executable name
TARGET = main

# Source files
SRCS = main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Rule to build the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to clean up generated files
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: clean
