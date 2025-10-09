# Makefile for building the project for course ISA at BUT FIT
# Author: Martin Zůbek
# Date: 2025-09-26

# Compiler
CPP = g++

# Compiler flags
FLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter

# Files with source code
SRC_FILES = src/main.cpp \
			src/file_handle/file_reader.cpp \
			src/program/program_context.cpp src/program/client.cpp src/program/server.cpp 
	
# Directory with header files
INC_DIR = inc

# Directory for object files
OBJ_DIR = obj

# Binary file created after compilation
PROG = secret

# Derived object files from source files
OBJ = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Link the object files
$(PROG): $(OBJ)
	@$(CPP) $(FLAGS) -I$(INC_DIR) $^ -o $@

# Compile the source files into object files
$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(CPP) $(FLAGS) -I$(INC_DIR) -c $< -o $@

# Clean build artifacts
clean:
	@rm -rf $(OBJ_DIR) $(PROG)

.PHONY: clean
