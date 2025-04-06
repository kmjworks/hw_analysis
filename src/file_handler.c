#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "file_handler.h"

#define UNUSED(x) (void)(x) // debug

int read_handler(const char* filename, BusLineProperties *bus_lines, int max_bus_lines) {
    FILE* file = fopen(filename, "r");

    /*
        Note to self: 
            I came to a conclusion that using assertions in this manner is problematic and probably
            considered a bad practice design-wise for several reasons:
                1. Asserts are meant for development-time invariant checking and not error handling.
                2. Assertions in this case can cause abrupt program termination with output that the end-user 
                might not understand
                3. Assertions should be called on expressions that do not have a side effect - e.g. 
                assigning a value to a variable.
                4. Assertions are used to check things that are NOT expected to fail - but calling fopen
                on a command line argument is something that is quite likely to fail which means that it should
                be handled in a more delicate way that is useful to the end-user of the program 
    */
    // assert(file != NULL && "[!!] FATAL Error: Colud not open the input file.");

    if(file == NULL) {
        fprintf(stderr, "[!!] FATAL Error : Could not open the input file '%s'.\n", filename);
        return -1;
    }

    char line_buffer[256];
    int count = 0, line_num = 0;

    while(count < max_bus_lines && fgets(line_buffer, sizeof(line_buffer), file) != NULL) {
        ++line_num;
        /*
            Skip empty lines and commented lines
        */
        if(line_buffer[0] == '\n' || line_buffer[0] == '#') continue;

        /*
            Remove the trailing newline
        */
        size_t trailing_len = strlen(line_buffer);
        if(trailing_len > 0 && (line_buffer[trailing_len-1] == '\n' || line_buffer[trailing_len-1] == '\r')) line_buffer[trailing_len-1] = '\0';

        /*
            Parse the current line being read into a BusLineProperties struct as defined in bus_line_handler.h
        */
        BusLineProperties* current = &bus_lines[count];

        char *token = strtok(line_buffer, ",");
        int field = 0, valid = 1;

        while (token != NULL && field < 7) {
            while(isspace((unsigned char)*token)) ++token;

            char *end = token + strlen(token) - 1;
            while(end > token && isspace((unsigned char)*end)) *--end = '\0';

            switch(field) {
                case 0:
                    if(sscanf(token, "%d", &current->line_number) != 1 ||
                        current->line_number <= 0) {
                            fprintf(stderr, "[!] Warning : Invalid line number (line %d\n).", line_num);
                            valid = 0;
                        }
                    break;

                case 1:
                    strncpy(current->departure_time, token, sizeof(current->departure_time) - 1);
                    current->departure_time[sizeof(current->departure_time) - 1] = '\0';
                    break;

                case 2:
                    if(sscanf(token, "%d", &current->subsidy_level) != 1 ||
                    current->subsidy_level < 1 || current->subsidy_level > 3) {
                        fprintf(stderr, "[!] Warning : Invalid subsidy level (line %d).\n", line_num);
                        valid = 0;
                    }
                    break;

                case 3:
                    if(sscanf(token, "%d", &current->passengers.adult) != 1 ||
                    current->passengers.adult < 0) {
                        fprintf(stderr, "[!] Warning : Invalid number of adult passengers (line %d).\n", line_num);
                        valid = 0;
                    }
                    break;

                case 4: 
                    if(sscanf(token, "%d", &current->passengers.student) != 1 || 
                    current->passengers.student < 0) {
                        fprintf(stderr, "[!] Warning : Invalid number of student passengers (line %d).\n", line_num);
                        valid = 0;
                    }
                    break;
                    
                case 5: 
                    if(sscanf(token, "%d", &current->passengers.senior) != 1 || 
                    current->passengers.senior < 0) {
                        fprintf(stderr, "[!] Warning : Invalid number of senior passengers (line %d).\n", line_num);
                        valid = 0;
                    }
                    break;
                        
                case 6: 
                    if(sscanf(token, "%lf", &current->route_length) != 1 ||
                        current->route_length <= 0) {
                            fprintf(stderr, "[!] Warning : Invalid route length (line %d).\n", line_num);
                            valid = 0;
                        }
                    break;
            }

            token = strtok(NULL, ",");
            ++field;
        }

        if(field < 7) {
            fprintf(stderr, "[!] Warning : Missing data fields (line %d).\n", line_num);
            valid = 0;
        }

        if(valid) ++count;
    }

    fclose(file);

    if (count == 0) fprintf(stderr, "[!] Warning : No valid data found in file '%s'.\n", filename);

    return count;
}

/*
    Write handler can be a little confusing in terms of naming - whoever reads this or sees this definition in
    the header file might assume that this is a "general-purpose" function that handles different kinds of
    writing operations to whatever file one deems relevant depending on the use case.

    I might or might not rename this later on, on second thought the parameters' data types indicate that it's a specific kind
    of write handler so on the other hand, it can stay the way it is
*/
int write_handler(const char* filename, BusLineProperties *bus_lines, int count) {
    FILE *file;
    file = fopen(filename, "w");
    if(file == NULL) {
        fprintf(stderr, "[!!] FATAL Error: Could not open the output file '%s'.\n", filename);
        return -1;
    }

    /* Write report header */
    fprintf(file, "--------------------------------------------------------------------\n");
    fprintf(file, "                   BUS LINES' PROFITABILITY REPORT                   \n");
    fprintf(file, "--------------------------------------------------------------------\n\n");

    if(count == 0) {
        fprintf(file, "No bus lines to display - closing the file and exiting.\n");
        fclose(file); 
        return 0;
    }

    int current_subsidy_level = -1;
    double total_profit = 0.0;
    int profitable_lines = 0;
    int unprofitable_lines = 0;

    /* Process each bus line */
    for(int i = 0; i < count; ++i) {
        const BusLineProperties *line = &bus_lines[i];
        total_profit += line->profitability;
        
        /* Track profitable vs unprofitable lines */
        if(line->profitability >= 0) {
            profitable_lines++;
        } else {
            unprofitable_lines++;
        }

        /* Print subsidy level headers when changing levels */
        if(line->subsidy_level != current_subsidy_level) {
            current_subsidy_level = line->subsidy_level;
            fprintf(file, "\n------------------------------------------------------------------\n");
            fprintf(file, "                      SUBSIDY LEVEL %d                              \n", current_subsidy_level);
            fprintf(file, "------------------------------------------------------------------\n");
            fprintf(file, "| Line | Time   | Passengers (Adults+Students+Seniors/Elderly) | Route length(km) | Result (P/L) (€)   |\n");
            fprintf(file, "------------------------------------------------------------------\n");
        }

        /* Format profit/loss indicators */
        char profit_indicator[20];
        if(line->profitability >= 0) {
            snprintf(profit_indicator, sizeof(profit_indicator), "+%.2f", line->profitability);
        } else {
            snprintf(profit_indicator, sizeof(profit_indicator), "%.2f", line->profitability);
        }

        fprintf(file, "| %-4d | %-6s | %3d+%-3d+%-3d      | %-9.1f | %-12s |\n", 
            line->line_number,
            line->departure_time,
            line->passengers.adult,
            line->passengers.student,
            line->passengers.senior,
            line->route_length,
            profit_indicator);
    }

    fprintf(file, "\n--------------------------------------------------------------------\n");
    fprintf(file, "                          PROFITABILITY ANALYSIS REPORT                          \n");
    fprintf(file, "-----------------------------------------------------------------------\n");
    fprintf(file, "Total bus lines analyzed: %d\n", count);
    fprintf(file, "Profitable lines: %d\n", profitable_lines);
    fprintf(file, "Unprofitable lines: %d\n", unprofitable_lines);
    
    /* Display final profit/loss with appropriate formatting */
    if(total_profit >= 0) {
        fprintf(file, "RESULT: PROFIT of %.2f€\n", total_profit);
    } else {
        fprintf(file, "RESULT: LOSS of %.2f€\n", -total_profit);
    }
    fprintf(file, "--------------------------------------------------------------------\n");

    fclose(file);
    return 0;
}
