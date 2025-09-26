# Makefile for building the project for course ISA at BUT FIT
# Author: Martin Zůbek
# Date: 2025-09-26

# Compiler
CPP = g++
# Compiler flags
FLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter -g

# Directory with source code
SRC_DIR = src
# Directory with header files
INC_DIR = inc
# Directory for object files
OBJ_DIR = obj
# Binary file created after compilation
PROG = secret

# Get all .cpp files -> files with source code
SRC = $(wildcard $(SRC_DIR)/*.cpp)
# Create list of object files from source files
OBJ = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC))

# Compilation 
$(PROG): $(OBJ)
	@$(CPP) $(FLAGS) -o $(PROG) $(OBJ)
 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@$(CPP) $(FLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)