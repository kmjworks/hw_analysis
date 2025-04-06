#ifndef BUS_LINE_HANDLER_H
#define BUS_LINE_HANDLER_H

#define MAX_BUS_LINES 100

typedef struct {
    int student;
    int adult;
    int senior;
} Passengers;

typedef struct {
    int line_number;
    char departure_time[10];
    int subsidy_level;
    Passengers passengers;
    double route_length;
    double profitability;
} BusLineProperties;

/*
    This function calculates the profitability for all buslines
    Param 1 - bus_lines is an array of bus lines that contains all the data about individual bus lines
    Param 2 - count defines the number of valid bus lines in the first parameter
*/
void calculate_profitability(BusLineProperties *bus_lines, int count);

/*
    Sorts the bus lines by subsidy level followed by sorting by profitability
    Param 1 - bus_lines is an array of bus lines that contains all the data about individual bus lines
    Param 2 - count defines the number of valid bus lines in the first parameter

    (same as the above)
*/
void sort_lines(BusLineProperties *bus_lines, int count);

/*
    This function displays the results in stdout (console)
    Parameters are same as the above
*/
void display_result_handler(const BusLineProperties *bus_lines, int count);

#endif // BUS_LINE_HANDLER_H