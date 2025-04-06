CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -pedantic -g
CPPFLAGS = -I../incl -MMD -MP

TEST_SRC = test_bus_line_handler.c test_file_handler.c test_runtime_config.c test_main.c
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_BINS = $(TEST_SRC:.c=)

SRC_DIR = ../src
SRC_FILES = $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))
SRC_OBJ = $(SRC_FILES:$(SRC_DIR)/%.c=%.o)

all: $(TEST_BINS)

test_%: test_%.o $(SRC_OBJ)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run_tests: $(TEST_BINS)
	@echo "Running all tests..."
	@for test in $(TEST_BINS); do \
		echo "\n--- Running $$test ---"; \
		./$$test; \
	done


clean:
	rm -f $(TEST_BINS) $(TEST_OBJ) $(SRC_OBJ) *.d

-include $(TEST_OBJ:.o=.d) $(SRC_OBJ:.o=.d)

.PHONY: all clean run_tests
