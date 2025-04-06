#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include "bus_line_handler.h"

/*
    Param 1 - filename, pretty self explanatory i.e. the input file's name
    Param 2 - bus_lines is an array of bus lines that contains all the data about individual bus lines
    Param 3 - max_bus_lines is the maximum number of lines to read from the input file
*/
int read_handler(const char* filename, BusLineProperties *bus_lines, int max_bus_lines);

/*
    Handler for writing all the results out to a output file
    Param 1 - same as parameter 1 above, but for the output file
    Param 2 - bus_lines is an array of bus lines that contains all the data about individual bus lines
    Param 3 - count defines the number of valid bus lines in the first parameter
*/
int write_handler(const char* filename, BusLineProperties *bus_lines, int count);

#endif // FILE_HANDLER_H