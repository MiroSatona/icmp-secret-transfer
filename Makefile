# Makefile for building the project for course ISA at BUT FIT
# Author: Martin Zůbek
# Date: 2025-09-26

# Compiler
CPP = g++
# Compiler flags
FLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter -g
# Flags for aes
CRYPTO_FLAGS =  -lssl -lcrypto

# Directory with source files
SRC_DIR = src
# Directory with header files
INC_DIR = inc
# Directory for object files
TEST_DIR = tests
# Directory for object files
OBJ_DIR = obj

# Files with implementation of the project
SRC_FILES = $(SRC_DIR)/main.cpp \
            $(SRC_DIR)/file_handle/abstract_file_handler.cpp $(SRC_DIR)/file_handle/file_writer.cpp $(SRC_DIR)/file_handle/file_reader.cpp \
            $(SRC_DIR)/network/client_socket_handler.cpp $(SRC_DIR)/network/server_socket_handler.cpp \
            $(SRC_DIR)/cipher/abstract_cipher.cpp $(SRC_DIR)/cipher/encryptor.cpp $(SRC_DIR)/cipher/decryptor.cpp \
            $(SRC_DIR)/protocol_message/message_creator.cpp $(SRC_DIR)/protocol_message/message_parser.cpp $(SRC_DIR)/protocol_message/message_handler.cpp\
            $(SRC_DIR)/program/program_context.cpp $(SRC_DIR)/program/client.cpp $(SRC_DIR)/program/server.cpp $(SRC_DIR)/program/help.cpp


# File with documentation of the project
MANUAL = manual.pdf
# License file of the project
LICENSE = LICENSE
# Readme file of the project
README = README.md
# This Makefile
MAKEFILE = Makefile
# Manual page for the project
MAN_PAGE = secret.1

# Test script
TEST_SCRIPT = $(TEST_DIR)/run_test.sh
# Binary file created after compilation
PROG = secret

# Derived object files from source files
OBJ = $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

# Link the object files
$(PROG): $(OBJ)
	@$(CPP) $(FLAGS) -I$(INC_DIR) $^ -o $@ $(CRYPTO_FLAGS)

# Compile the source files into object files
$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(CPP) $(FLAGS) -I$(INC_DIR) -c $< -o $@

# Build the program and run the test suite (uses sudo when not run as root)
test: $(PROG)
	@if [ "$$(id -u)" -eq 0 ]; then \
		BINARY="$(CURDIR)/$(PROG)" bash $(TEST_SCRIPT); \
	else \
		sudo BINARY="$(CURDIR)/$(PROG)" bash $(TEST_SCRIPT); \
	fi

# Target to create a zip archive of the project    
zip:
	@zip -r $(PROG).zip $(SRC_DIR) $(INC_DIR) $(TEST_DIR) $(LICENSE) $(MAKEFILE) $(MANUAL) $(README) $(MAN_PAGE)	

# Clean executable file, object files and temporary test files (uses sudo when not run as root)
clean:
	@sudo rm -rf $(OBJ_DIR) $(PROG) $(TEST_DIR)/_tmp

.PHONY: test clean zip
