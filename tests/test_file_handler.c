#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../incl/file_handler.h"
#include "test_utils.h"

void test_read_handler(TestResults *results);
void test_write_handler(TestResults *results);
void test_invalid_inputs(TestResults *results);

#define TEST_INPUT_FILE "test_input.txt"
#define TEST_OUTPUT_FILE "test_output.txt"

int main() {
    TestResults results;
    init_test_results(&results);
    
    printf("\n--- Testing File Handler ---\n\n");
    
    test_read_handler(&results);
    test_write_handler(&results);
    test_invalid_inputs(&results);
    
    print_test_summary(&results);
    
    /*
        Cleanup
    */
    unlink(TEST_INPUT_FILE);
    unlink(TEST_OUTPUT_FILE);
    
    return results.tests_failed > 0 ? 1 : 0;
}

void create_test_input_file() {
    FILE *file = fopen(TEST_INPUT_FILE, "w");
    if (file) {
        fprintf(file, "# Test Bus Line Data\n");
        fprintf(file, "1,08:00,1,25,10,5,15.5\n");
        fprintf(file, "2,09:15,2,18,8,4,12.0\n");
        fprintf(file, "3,10:30,3,15,7,6,10.5\n");
        fprintf(file, "# Comment\n");
        fprintf(file, "\n");
        fprintf(file, "4,12:00,1,20,12,3,18.0\n");
        fclose(file);
    }
}


/*
    Test the read handler functionality that should parse the data from all valid
    bus lines and increment the count each time it has parsed a line's data from the input file
*/
void test_read_handler(TestResults *results) {
    printf("Testing read_handler...\n");
    
    create_test_input_file();
    
    /*
        Verify that valid bus lines are read 
    */
    BusLineProperties bus_lines[MAX_BUS_LINES];
    int count = read_handler(TEST_INPUT_FILE, bus_lines, MAX_BUS_LINES);
    
    /*
        Check if the count matches the expected number of lines that the function was supposed to parse from the
        input file
    */
    ASSERT_INT_EQUAL("Correct number of bus lines read", 4, count);
    
    /*
        Verify the data integrity (by integrity I do not mean a hash or a checksum lol, just the contents of the data) of the first bus line
    */
    ASSERT_INT_EQUAL("Line 1 number", 1, bus_lines[0].line_number);
    ASSERT_STRING_EQUAL("Line 1 departure time", "08:00", bus_lines[0].departure_time);
    ASSERT_INT_EQUAL("Line 1 subsidy level", 1, bus_lines[0].subsidy_level);
    ASSERT_INT_EQUAL("Line 1 adult passengers", 25, bus_lines[0].passengers.adult);
    ASSERT_INT_EQUAL("Line 1 student passengers", 10, bus_lines[0].passengers.student);
    ASSERT_INT_EQUAL("Line 1 senior passengers", 5, bus_lines[0].passengers.senior);
    ASSERT_DOUBLE_EQUAL("Line 1 route length", 15.5, bus_lines[0].route_length, 0.01);
    
    /*
        Perform the same verification for the fourth line as for the first (after the comment and empty line)
    */
    ASSERT_INT_EQUAL("Line 4 number", 4, bus_lines[3].line_number);
    ASSERT_STRING_EQUAL("Line 4 departure time", "12:00", bus_lines[3].departure_time);
    ASSERT_INT_EQUAL("Line 4 subsidy level", 1, bus_lines[3].subsidy_level);
    ASSERT_INT_EQUAL("Line 4 adult passengers", 20, bus_lines[3].passengers.adult);
    ASSERT_INT_EQUAL("Line 4 student passengers", 12, bus_lines[3].passengers.student);
    ASSERT_INT_EQUAL("Line 4 senior passengers", 3, bus_lines[3].passengers.senior);
    ASSERT_DOUBLE_EQUAL("Line 4 route length", 18.0, bus_lines[3].route_length, 0.01);
    
    /*
        Test an edge case - the limit of maximum bus lines allowed for analysis at a time
    */
    BusLineProperties limited_lines[2];
    int limited_count = read_handler(TEST_INPUT_FILE, limited_lines, 2);
    ASSERT_INT_EQUAL("Respect max_bus_lines limit", 2, limited_count);
}

/*
    Test the functionality of the write handling function that prints out the analysis' summary (profit or loss for a given bus line)
*/
void test_write_handler(TestResults *results) {
    printf("Testing write_handler...\n");
    
    /*
        Create some dummy-hard-coded data
    */
    BusLineProperties bus_lines[3] = {
        {
            .line_number = 1,
            .departure_time = "08:00",
            .subsidy_level = 1,
            .passengers = {.adult = 25, .student = 10, .senior = 5},
            .route_length = 15.5,
            .profitability = 100.0
        },
        {
            .line_number = 2,
            .departure_time = "09:15",
            .subsidy_level = 2,
            .passengers = {.adult = 18, .student = 8, .senior = 4},
            .route_length = 12.0,
            .profitability = 50.0
        },
        {
            .line_number = 3,
            .departure_time = "10:30",
            .subsidy_level = 3,
            .passengers = {.adult = 15, .student = 7, .senior = 6},
            .route_length = 10.5,
            .profitability = -25.0  /* NB! This should be an unprofitable amount resulting in a loss (negative)*/
        }
    };
    

    /*
        Write the results of the analysis to the output file created
    */
    int write_result = write_handler(TEST_OUTPUT_FILE, bus_lines, 3);
    ASSERT_INT_EQUAL("Write handler returns success", 0, write_result);
    

    /*
        Check if the file was created in the first place
    */
    FILE *file = fopen(TEST_OUTPUT_FILE, "r");
    ASSERT_TRUE("Output file was created", file != NULL);
    
    if (file) {
        char line[256];
        int found_header = 0;
        int found_subsidy_level_1 = 0;
        int found_subsidy_level_2 = 0;
        int found_subsidy_level_3 = 0;
        int found_summary = 0;
        
        while (fgets(line, sizeof(line), file)) {
            if (strstr(line, "BUS LINE PROFITABILITY REPORT")) {
                found_header = 1;
            } else if (strstr(line, "SUBSIDY LEVEL 1")) {
                found_subsidy_level_1 = 1;
            } else if (strstr(line, "SUBSIDY LEVEL 2")) {
                found_subsidy_level_2 = 1;
            } else if (strstr(line, "SUBSIDY LEVEL 3")) {
                found_subsidy_level_3 = 1;
            } else if (strstr(line, "SUMMARY REPORT")) {
                found_summary = 1;
            }
        }
        
        fclose(file);
        
        ASSERT_TRUE("Report header found", found_header);
        ASSERT_TRUE("Subsidy level 1 section found", found_subsidy_level_1);
        ASSERT_TRUE("Subsidy level 2 section found", found_subsidy_level_2);
        ASSERT_TRUE("Subsidy level 3 section found", found_subsidy_level_3);
        ASSERT_TRUE("Summary section found", found_summary);
    }

    /*
        Test writing to an empty array - this should succeed nevertheless
    */
    write_result = write_handler(TEST_OUTPUT_FILE, bus_lines, 0);
    ASSERT_INT_EQUAL("Write handler with empty array returns success", 0, write_result);
}

/*
    Test error handling - invalid inputs and file errors
*/
void test_invalid_inputs(TestResults *results) {
    printf("Testing error handling...\n");
    
    /*
        Verify that the program handles a non-existing input file error - reading a 
        non-existent input file should return an error as the program cannot retrieve a valid
        pointer to a file that does not exist in the first place
    */
    BusLineProperties bus_lines[MAX_BUS_LINES];
    int count = read_handler("nonexistent_file.txt", bus_lines, MAX_BUS_LINES);
    ASSERT_INT_EQUAL("Reading non-existent file returns error", -1, count);
    
    // Test invalid input in bus line data
    /*
        Test invalid input data for the bus lines - by testing against invalid input data I mean
        verifying that the program terminates (handles an error) if the source/input data file does not adhere to the 
        .csv formatting rules as defined:

        line_number,departure_time,subsidy_level,adults,students,seniors,route_length 
    */
    FILE *file = fopen(TEST_INPUT_FILE, "w");
    if (file) {
        /*
            Write some invalid dummy data in the input file
        */
        fprintf(file, "invalid,08:00,1,25,10,5,15.5\n");  /* Invalid line number */
        fprintf(file, "1,08:00,5,25,10,5,15.5\n");        /* Invalid subsidy level */
        fprintf(file, "2,08:00,1,-5,10,5,15.5\n");        /* Invalid passenger count */
        fprintf(file, "3,08:00,1,25,10,5,-10.5\n");       /* Invalid route length */
        fprintf(file, "4,08:00,1,25\n");                  /* Incomplete data - no passenger count for students, seniors/elderly and the route length is also not defined*/
        fclose(file);
        
        count = read_handler(TEST_INPUT_FILE, bus_lines, MAX_BUS_LINES);
        ASSERT_INT_EQUAL("All invalid lines rejected", 0, count);
    }
}
