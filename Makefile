# Makefile for building the project for course ISA at BUT FIT
# Author: Martin Zůbek
# Date: 2025-09-26

# Compiler
CPP = g++

# Compiler flags
FLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter -g

# Flags for aes
CRYPTO_FLAGS =  -lssl -lcrypto

# Files with source code
SRC_FILES = src/main.cpp \
            src/file_handle/abstract_file_handler.cpp src/file_handle/file_writer.cpp src/file_handle/file_reader.cpp \
            src/network/client_socket_handler.cpp src/network/server_socket_handler.cpp \
            src/cipher/abstract_cipher.cpp src/cipher/encryptor.cpp src/cipher/decryptor.cpp \
            src/protocol_message/message_creator.cpp src/protocol_message/message_parser.cpp src/protocol_message/message_handler.cpp\
            src/program/program_context.cpp src/program/client.cpp src/program/server.cpp src/program/help.cpp

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
	@$(CPP) $(FLAGS) -I$(INC_DIR) $^ -o $@ $(CRYPTO_FLAGS)

# Compile the source files into object files
$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	@$(CPP) $(FLAGS) -I$(INC_DIR) -c $< -o $@

# Clean executable file and object files
clean:
	@rm -rf $(OBJ_DIR) $(PROG)

.PHONY: clean
