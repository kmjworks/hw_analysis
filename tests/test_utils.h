#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>


typedef struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
} TestResults;


void init_test_results(TestResults *results) {
    results->tests_run = 0;
    results->tests_passed = 0;
    results->tests_failed = 0;
}


/*
    My custom test assertion macros - you could also use assert.h but it offers less flexibility
    and a testing framework is too heavy in terms of file size for uploading it for grading
*/
#define ASSERT_TRUE(message, test) do { \
    results->tests_run++; \
    if (test) { \
        printf("+ PASS: %s\n", message); \
        results->tests_passed++; \
    } else { \
        printf("- FAIL: %s (Line %d)\n", message, __LINE__); \
        results->tests_failed++; \
    } \
} while (0)

#define ASSERT_INT_EQUAL(message, expected, actual) do { \
    results->tests_run++; \
    if ((expected) == (actual)) { \
        printf("+ PASS: %s\n", message); \
        results->tests_passed++; \
    } else { \
        printf("- FAIL: %s - Expected %d, got %d (Line %d)\n", \
               message, (expected), (actual), __LINE__); \
        results->tests_failed++; \
    } \
} while (0)

#define ASSERT_DOUBLE_EQUAL(message, expected, actual, epsilon) do { \
    results->tests_run++; \
    if (fabs((expected) - (actual)) < (epsilon)) { \
        printf("+ PASS: %s\n", message); \
        results->tests_passed++; \
    } else { \
        printf("- FAIL: %s - Expected %f, got %f (Line %d)\n", \
               message, (expected), (actual), __LINE__); \
        results->tests_failed++; \
    } \
} while (0)

#define ASSERT_STRING_EQUAL(message, expected, actual) do { \
    results->tests_run++; \
    if (strcmp((expected), (actual)) == 0) { \
        printf("+ PASS: %s\n", message); \
        results->tests_passed++; \
    } else { \
        printf("- FAIL: %s - Expected '%s', got '%s' (Line %d)\n", \
               message, (expected), (actual), __LINE__); \
        results->tests_failed++; \
    } \
} while (0)


void print_test_summary(TestResults *results) {
    printf("\n--- Test Summary ---\n");
    printf("Tests run: %d\n", results->tests_run);
    printf("Tests passed: %d\n", results->tests_passed);
    printf("Tests failed: %d\n", results->tests_failed);
    printf("Pass rate: %.1f%%\n", (float)results->tests_passed / results->tests_run * 100);
}

FILE* create_temp_file(const char* content) {
    FILE* tmp = tmpfile();
    if (tmp) {
        fputs(content, tmp);
        rewind(tmp);
    }
    return tmp;
}

#endif // TEST_UTILS_H
