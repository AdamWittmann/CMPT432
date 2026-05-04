CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

TARGET = compiler
SOURCES = main.cpp token.cpp lexer.cpp parser.cpp cst_node.cpp symbol_table.cpp semantic_analyzer.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build the compiler
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run tests
test: $(TARGET)
	@echo "Running test suite..."
	@./tests/run_tests.sh

# Run a single test
test-one: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make test-one FILE=tests/test-valid-simple.txt"; \
	else \
		./$(TARGET) $(FILE); \
	fi

# Run tests with verbose output
test-verbose: $(TARGET)
	@echo "Running tests with verbose output..."
	@for file in tests/test-*.txt; do \
		echo ""; \
		echo "=== Testing $$file ==="; \
		./$(TARGET) $$file; \
		echo ""; \
	done

# Clean build files
clean:
	rm -f $(TARGET) $(OBJECTS)

# Clean everything including test outputs
clean-all: clean
	rm -f tests/*.log tests/*.out

.PHONY: all test test-one test-verbose clean clean-all