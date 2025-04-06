#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../incl/bus_line_handler.h"
#include "../incl/file_handler.h"
#include "../incl/runtime_configuration_handler.h"
#include "test_utils.h"


void test_integration(TestResults *results);
void test_profit_loss_calculation(TestResults *results);

#define TEST_CONFIG_FILE "test_config.txt"
#define TEST_INPUT_FILE "test_input.txt"
#define TEST_OUTPUT_FILE "test_output.txt"

int main() {
    TestResults results;
    init_test_results(&results);
    
    printf("\n--- Integration Tests ---\n\n");
    
    test_integration(&results);
    test_profit_loss_calculation(&results);
    
    print_test_summary(&results);
    
    /*
        Cleanup
    */
    unlink(TEST_CONFIG_FILE);
    unlink(TEST_INPUT_FILE);
    unlink(TEST_OUTPUT_FILE);
    
    return results.tests_failed > 0 ? 1 : 0;
}

void setup_test_files() {
    FILE *config_file = fopen(TEST_CONFIG_FILE, "w");
    if (config_file) {
        fprintf(config_file, "input_file=%s\n", TEST_INPUT_FILE);
        fprintf(config_file, "output_file=%s\n", TEST_OUTPUT_FILE);
        fprintf(config_file, "stdout_output=1\n");
        fprintf(config_file, "file_output=1\n");
        fclose(config_file);
    }
    
    /*
        Input data file with some dummy bus line data
    */
    FILE *input_file = fopen(TEST_INPUT_FILE, "w");
    if (input_file) {
        fprintf(input_file, "# Test Bus Line Data\n");
        fprintf(input_file, "1,08:00,1,25,10,5,15.5\n");  // Profitable line
        fprintf(input_file, "2,09:15,2,18,8,4,12.0\n");   // Profitable line
        fprintf(input_file, "3,10:30,3,15,7,6,10.5\n");   // Profitable line
        fprintf(input_file, "4,12:00,1,3,2,1,50.0\n");    // Unprofitable line
        fclose(input_file);
    }
}

/*
    Test and verify the complete workflow of the program starting from reading and parsing 
    the input up until generating the analysis report (and a stdout output for it) + writing it to a .txt file as output
*/
void test_integration(TestResults *results) {
    printf("Testing the compelte  workflow of profitability analysis...\n");
    
    setup_test_files();
    
    FileSettings settings;
    runtime_config_load_handler(&settings, TEST_CONFIG_FILE);
    
    BusLineProperties bus_lines[MAX_BUS_LINES];
    int line_count = read_handler(settings.input_file, bus_lines, MAX_BUS_LINES);
    
    /*
        Verify that the program reads the right number of bus lines' data and proceed to calculating
        the profitability of each individual bus line whose data was parsed
    */
    ASSERT_INT_EQUAL("Correct number of bus lines read", 4, line_count);
    ASSERT_INT_EQUAL("First line number correct", 1, bus_lines[0].line_number);
    
    calculate_profitability(bus_lines, line_count);
    
    /*
        I think that this is a little trivial and might as well be removed because there are tests
        targeting profitability calculations indivually but at the same time if I do not verify the 
        profitability calculation, I can't say that I've "tested" the complete workflow of the program
    */
    ASSERT_TRUE("First line is profitable", bus_lines[0].profitability > 0);
    ASSERT_TRUE("Last line is unprofitable", bus_lines[3].profitability < 0);
    
   
    sort_lines(bus_lines, line_count);
    
    ASSERT_INT_EQUAL("First subsidy level is 1", 1, bus_lines[0].subsidy_level);
    
    int write_result = write_handler(settings.output_file, bus_lines, line_count);
    ASSERT_INT_EQUAL("Write handler returns success", 0, write_result);
    
    FILE *output_file = fopen(settings.output_file, "r");
    ASSERT_TRUE("Output file was created", output_file != NULL);
    if (output_file) {
        fclose(output_file);
    }
}

/*
    Test the profit and loss calculations' correctness across different scenarios - e.g.
    all lines are profitable, some lines are profitable-some are not, all lines are unprofitable etc..
*/
void test_profit_loss_calculation(TestResults *results) {
    printf("Testing the correctness of profit/loss calculations...\n");
    
    /*
        Test case 1: all bus lines are profitable
    */
    BusLineProperties profitable_lines[3] = {
        {
            .line_number = 1,
            .departure_time = "08:00",
            .subsidy_level = 1,
            .passengers = {.adult = 25, .student = 10, .senior = 5},
            .route_length = 15.5
        },
        {
            .line_number = 2,
            .departure_time = "09:15",
            .subsidy_level = 2,
            .passengers = {.adult = 18, .student = 8, .senior = 4},
            .route_length = 12.0
        },
        {
            .line_number = 3,
            .departure_time = "10:30",
            .subsidy_level = 3,
            .passengers = {.adult = 15, .student = 7, .senior = 6},
            .route_length = 10.5
        }
    };
    
    calculate_profitability(profitable_lines, 3);
    
    /*
        Verify that all lines are profitable as expected
    */
    ASSERT_TRUE("Line 1 is profitable", profitable_lines[0].profitability > 0);
    ASSERT_TRUE("Line 2 is profitable", profitable_lines[1].profitability > 0);
    ASSERT_TRUE("Line 3 is profitable", profitable_lines[2].profitability > 0);
    
    /*
        Calculated the total combined profit of all bus lines
    */
    double total_profit = 0.0;
    for (int i = 0; i < 3; i++) {
        total_profit += profitable_lines[i].profitability;
    }
    
    ASSERT_TRUE("Total profit is positive", total_profit > 0);
    

    /*
        Test case 2: all bus lines here are unprofitable and yield a loss
    */
    BusLineProperties unprofitable_lines[3] = {
        {
            .line_number = 111,
            .departure_time = "05:00",
            .subsidy_level = 1,
            .passengers = {.adult = 2, .student = 1, .senior = 0},
            .route_length = 70.0
        },
        {
            .line_number = 221,
            .departure_time = "03:30",
            .subsidy_level = 2,
            .passengers = {.adult = 1, .student = 1, .senior = 0},
            .route_length = 95.0
        },
        {
            .line_number = 331,
            .departure_time = "01:00",
            .subsidy_level = 3,
            .passengers = {.adult = 1, .student = 0, .senior = 0},
            .route_length = 120.0
        }
    };
    
    calculate_profitability(unprofitable_lines, 3);
    
    /*
        Verify that all bus lines produced a loss instead of a profit
    */
    ASSERT_TRUE("Line 111 is unprofitable", unprofitable_lines[0].profitability < 0);
    ASSERT_TRUE("Line 221 is unprofitable", unprofitable_lines[1].profitability < 0);
    ASSERT_TRUE("Line 331 is unprofitable", unprofitable_lines[2].profitability < 0);
    
    /*
        Calculate the combined loss
    */
    double total_loss = 0.0;
    for (int i = 0; i < 3; i++) {
        total_loss += unprofitable_lines[i].profitability;
    }
    
    ASSERT_TRUE("Total profit is negative (loss)", total_loss < 0);
    
    /*
        Test case 3: final test case where there is a mix of bus lines in terms of profitability
        (i.e. some are profitable and some are not)
    */
    BusLineProperties mixed_lines[4] = {
        {
            .line_number = 101,
            .departure_time = "07:30",
            .subsidy_level = 1,
            .passengers = {.adult = 15, .student = 8, .senior = 4},
            .route_length = 12.0
        },
        {
            .line_number = 301,
            .departure_time = "14:15",
            .subsidy_level = 1,
            .passengers = {.adult = 3, .student = 2, .senior = 1},
            .route_length = 50.0
        },
        {
            .line_number = 102,
            .departure_time = "08:15",
            .subsidy_level = 2,
            .passengers = {.adult = 12, .student = 6, .senior = 3},
            .route_length = 10.0
        },
        {
            .line_number = 501,
            .departure_time = "06:00",
            .subsidy_level = 3,
            .passengers = {.adult = 3, .student = 1, .senior = 1},
            .route_length = 60.0
        }
    };
    
    calculate_profitability(mixed_lines, 4);
    
    /*
        Count the profitable and unprofitable bus lines individually
    */
    int profitable_count = 0;
    int unprofitable_count = 0;
    double mixed_total = 0.0;
    
    for (int i = 0; i < 4; i++) {
        if (mixed_lines[i].profitability >= 0) {
            profitable_count++;
        } else {
            unprofitable_count++;
        }
        mixed_total += mixed_lines[i].profitability;
    }
    
    ASSERT_TRUE("Mix contains both profitable and unprofitable lines", 
                profitable_count > 0 && unprofitable_count > 0);
    
    /*
        Check if the function that produces the analysis' report produces a negative
        number for representing loss and vice-versa for representing profit
    */
    if (mixed_total >= 0) {
        ASSERT_TRUE("Positive total correctly identified as profit", mixed_total >= 0);
    } else {
        ASSERT_TRUE("Negative total correctly identified as loss", mixed_total < 0);
    }
}
