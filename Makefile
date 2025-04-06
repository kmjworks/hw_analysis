SRC_DIR	:= src
OBJ_DIR := obj
BIN_DIR := bin
CC		:= gcc

# Files
BIN 	:= $(BIN_DIR)/bus_line_analysis
SRC 	:= $(wildcard $(SRC_DIR)/*.c)
OBJ 	:= $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
MAIN	:= main.c
OBJ_MAIN := $(OBJ_DIR)/main.o

CPPFLAGS	:= -Iincl -MMD -MP
CFLAGS		:= -std=c99 -Wall -Wextra -Werror -pedantic
DEBUG_FLAGS	:= -g -O0


.PHONY:	all clean

all:	$(BIN)


$(BIN): $(OBJ) $(OBJ_MAIN) | $(BIN_DIR)
	$(CC) $^ -o $@

$(OBJ_MAIN): $(MAIN) | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

-include $(OBJ:.o=.d)
