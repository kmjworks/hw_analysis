#include <stdio.h>
#include <stdlib.h>

#include "bus_line_handler.h"
#include "file_handler.h"
#include "runtime_configuration_handler.h"

int main(int argc, char** argv) {
    FileSettings settings;
    cli_argument_handler(argc, argv, &settings);
    runtime_config_load_handler(&settings, "config.txt");

    BusLineProperties bus_lines_input_data_buffer[MAX_BUS_LINES];
    int line_count = read_handler(settings.input_file, bus_lines_input_data_buffer, MAX_BUS_LINES);
    if (line_count <= 0) {
        fprintf(stderr, "[!!] FATAL Error: No valid data found in input file '%s'.\n", settings.input_file);
        exit(EXIT_FAILURE);
    }
    

    calculate_profitability(bus_lines_input_data_buffer, line_count);
    sort_lines(bus_lines_input_data_buffer, line_count);

    if(settings.stdout_output_enabled) {
        printf("[*] Processing file: %s\n", settings.input_file);
        printf("[+] Found %d valid bus lines\n\n", line_count);
        display_result_handler(bus_lines_input_data_buffer, line_count);

        double total_pl = 0.0;
        for(int i = 0; i < line_count; ++i) total_pl += bus_lines_input_data_buffer[i].profitability;

        printf("\n------------------------------------------------------------------\n");
        printf("TOTAL P/L: %s %.2f€\n", 
               (total_pl < 0) ? "Loss of" : "Profit of", 
               total_pl);
        printf("------------------------------------------------------------------\n");

        if (settings.file_output_enabled) printf("\n[+] Bus Line Profitability Analysis Complete.\n[*] Savings Results to : %s\n\n", settings.output_file);
    }

    if(settings.file_output_enabled) write_handler(settings.output_file, bus_lines_input_data_buffer, line_count);
    printf("\n[+] Results saved to : %s\n[+] All done. Exiting...\n", settings.output_file);

    return 0;
}
