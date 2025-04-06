#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../incl/bus_line_handler.h"
#include "test_utils.h"


void test_calculate_profitability(TestResults *results);
void test_sort_lines(TestResults *results);
void test_edge_cases(TestResults *results);

int main() {
    TestResults results;
    init_test_results(&results);
    
    printf("\n--- Testing Bus Line Handler ---\n\n");
    
    test_calculate_profitability(&results);
    test_sort_lines(&results);
    test_edge_cases(&results);
    
    print_test_summary(&results);
    
    return results.tests_failed > 0 ? 1 : 0;
}


/*
    Test the logic of the bus line's profitability calculation
*/
void test_calculate_profitability(TestResults *results) {
    printf("Testing profitability calculation...\n");

    /*
        Test case 1: Regular profitable route with subsidy level 1
    */
    BusLineProperties line1 = {
        .line_number = 1,
        .departure_time = "08:00",
        .subsidy_level = 1,
        .passengers = {.adult = 25, .student = 10, .senior = 5},
        .route_length = 15.5
    };
    
    calculate_profitability(&line1, 1);
    
    /*
        Expected result of the profitability calcuation: 
        Cost = 15.5 * 2.50 = 38.75
        Revenue = (25 * 12.0) + (10 * 8.0) + (5 * 5.0) = 300 + 80 + 25 = 405
        Subsidy = 15.5 * 0.50 = 7.75
        Profit = 405 + 7.75 - 38.75 = 374.0
    */

    ASSERT_DOUBLE_EQUAL("Profitable route with subsidy level 1", 374.0, line1.profitability, 0.01);

    /*
        Test case 2: an unprofitable route with a subsidy level of 3
    */
    BusLineProperties line2 = {
        .line_number = 331,
        .departure_time = "01:00",
        .subsidy_level = 3,
        .passengers = {.adult = 1, .student = 0, .senior = 0},
        .route_length = 120.0
    };
    
    calculate_profitability(&line2, 1);

    /*
        Expected result of the profitability calcuation: 
        Cost = 120.0 * 2.50 = 300.0
        Revenue = (1 * 12.0) + (0 * 8.0) + (0 * 5.0) = 12.0
        Subsidy = 120.0 * 1.50 = 180.0
        Profit = 12.0 + 180.0 - 300.0 = -108.0
    */

    ASSERT_DOUBLE_EQUAL("Unprofitable route with subsidy level 3", -108.0, line2.profitability, 0.01);
    
    /*
        Test case 3: This is an edge case i.e. a bus line with no passengers
    */
    BusLineProperties line3 = {
        .line_number = 999,
        .departure_time = "23:59",
        .subsidy_level = 2,
        .passengers = {.adult = 0, .student = 0, .senior = 0},
        .route_length = 50.0
    };
    
    calculate_profitability(&line3, 1);
    /*
        Expected result of the profitability calcuation: 
        Cost = 50.0 * 2.50 = 125.0
        Revenue = (0 * 12.0) + (0 * 8.0) + (0 * 5.0) = 0.0
        Subsidy = 50.0 * 1.00 = 50.0
        Profit = 0.0 + 50.0 - 125.0 = -75.0
    */

    ASSERT_DOUBLE_EQUAL("Zero-passenger route", -75.0, line3.profitability, 0.01);
}

/*
    Test the quick-sort functionality of the function that sorts the bus lines first by the level of 
    subsidy followed by sorting each line by profitability in a descending order
*/
void test_sort_lines(TestResults *results) {
    printf("Testing bus line sorting...\n");
    
    BusLineProperties bus_lines[5] = {
        {
            .line_number = 1,
            .departure_time = "08:00",
            .subsidy_level = 2,
            .passengers = {.adult = 20, .student = 10, .senior = 5},
            .route_length = 15.0,
            .profitability = 250.0
        },
        {
            .line_number = 2,
            .departure_time = "09:00",
            .subsidy_level = 1,
            .passengers = {.adult = 25, .student = 15, .senior = 10},
            .route_length = 20.0,
            .profitability = 300.0
        },
        {
            .line_number = 3,
            .departure_time = "10:00",
            .subsidy_level = 3,
            .passengers = {.adult = 15, .student = 5, .senior = 3},
            .route_length = 10.0,
            .profitability = 150.0
        },
        {
            .line_number = 4,
            .departure_time = "11:00",
            .subsidy_level = 2,
            .passengers = {.adult = 10, .student = 5, .senior = 2},
            .route_length = 25.0,
            .profitability = 100.0
        },
        {
            .line_number = 5,
            .departure_time = "12:00",
            .subsidy_level = 1,
            .passengers = {.adult = 5, .student = 2, .senior = 1},
            .route_length = 30.0,
            .profitability = 50.0
        }
    };
    
    sort_lines(bus_lines, 5);
    

    

    /*
        Verify that the subsidy levels are in a monotonously-growing order as they should be (1,2,3)
    */
    ASSERT_INT_EQUAL("First element has subsidy level 1", 1, bus_lines[0].subsidy_level);
    ASSERT_INT_EQUAL("Second element has subsidy level 1", 1, bus_lines[1].subsidy_level);
    ASSERT_INT_EQUAL("Third element has subsidy level 2", 2, bus_lines[2].subsidy_level);
    ASSERT_INT_EQUAL("Fourth element has subsidy level 2", 2, bus_lines[3].subsidy_level);
    ASSERT_INT_EQUAL("Fifth element has subsidy level 3", 3, bus_lines[4].subsidy_level);
    
    /*
        Check if profitability is sorted and ordered within each subsidy level
    */
    ASSERT_DOUBLE_EQUAL("Higher profit route is first within subsidy level 1", 300.0, bus_lines[0].profitability, 0.01);
    ASSERT_DOUBLE_EQUAL("Lower profit route is second within subsidy level 1", 50.0, bus_lines[1].profitability, 0.01);
    ASSERT_DOUBLE_EQUAL("Higher profit route is first within subsidy level 2", 250.0, bus_lines[2].profitability, 0.01);
    ASSERT_DOUBLE_EQUAL("Lower profit route is second within subsidy level 2", 100.0, bus_lines[3].profitability, 0.01);
}


/*
    Test the analysis program's edge cases and boundary conditions (e.g. a single route, a data source array with no bus lines....)
*/
void test_edge_cases(TestResults *results) {
    printf("Testing edge cases...\n");
    
    /*
        Test case 1: A bus line with a single route
    */
    BusLineProperties single_line = {
        .line_number = 1,
        .departure_time = "08:00",
        .subsidy_level = 1,
        .passengers = {.adult = 10, .student = 5, .senior = 2},
        .route_length = 10.0
    };
    
    calculate_profitability(&single_line, 1);
    sort_lines(&single_line, 1);
    
    /*
        Make sure that the profitability calculation's and the sorting logic's implementation
        did not break with a single bus line
    */
    ASSERT_TRUE("Single line profitability calculation completed", single_line.profitability != 0);
    
    /*
        Test case 2: Empty array without any bus lines - this should not crash despite there being no bus lines in the array
    */
    BusLineProperties empty_lines[1];
    sort_lines(empty_lines, 0);
    
    /*
        If the test reaches this line without crashing, we are good and the test passed without any errors
    */
    ASSERT_TRUE("Empty array sorting completed without crashes", true);
}
