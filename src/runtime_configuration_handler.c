#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "runtime_configuration_handler.h"

void runtime_config_load_handler(FileSettings *settings, const char* configuration_file) {
    strcpy(settings->input_file, "../data/bus_lines_data.txt");
    strcpy(settings->output_file, "../data/buslines_analysis_report.txt");
    settings->stdout_output_enabled = 1;
    settings->file_output_enabled = 1;

    FILE *file = fopen(configuration_file, "r");
    // assert(file != NULL && "[!] FATAL Error: Unable to load pre-set configuration from the configuration file.");
    if(!file) {
        fprintf(stderr, "[!] Warning : Configuration file '%s' not found - falling back to default runtime configuration.\n", configuration_file);
        return;
    }

    char line[256];
    char key[64];
    char val[192];

    while(fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '#' || line[0] == '\n') continue;

        if (sscanf(line, "%63[^=]=%191s", key, val) == 2) {
            char *key_end = key + strlen(key) - 1;
            while(key_end > key && isspace((unsigned char)*key_end)) {
                *--key_end = '\0';
            }

            /*
                I hate implementing stacked if and else-if branches and avoid them as much as possible, 
                but in this case I'm unable to use the 'switch' statement as it requires an argument of integral type ¯\_(ツ)_/¯
            */

            if(strcmp(key, "input_file") == 0) {
                strcpy(settings->input_file, val);
            } else if (strcmp(key, "output_file") == 0) {
                strcpy(settings->output_file, val);
            } else if (strcmp(key, "stdout_output") == 0) {
                settings->stdout_output_enabled = atoi(val);
            } else if (strcmp(key, "file_output") == 0) {
                settings->file_output_enabled = atoi(val);
            }
        }
    }

    fclose(file);
}

void cli_argument_handler(int argc, char **argv, FileSettings *settings) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (i + 1 < argc) {
                strcpy(settings->input_file, argv[++i]);
            } else {
                fprintf(stderr, "[!!] FATAL Error: Missing filename after %s\n", argv[i]);
                exit(1);
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                strcpy(settings->output_file, argv[++i]);
            } else {
                fprintf(stderr, "[!!] FATAL Error: Missing filename after %s\n", argv[i]);
                exit(1);
            }
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--no-screen") == 0) {
            settings->stdout_output_enabled = 0;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--no-file") == 0) {
            settings->file_output_enabled = 0;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            runtime_usage_print_handler(argv[0]);
            exit(0);
        } else if (i == 1 && argv[i][0] != '-') {
            /* First argument as input file (just for convenience >:DD) */
            strcpy(settings->input_file, argv[i]);
        } else {
            fprintf(stderr, "[!] Warning : Unknown option '%s'\n", argv[i]);
        }
    }
}

void runtime_usage_print_handler(const char* executable_name) {
    printf("Bus Line Profitability Analysis Tool\n");
    printf("---------------------------------------\n");
    printf("Usage: %s [options] [input_file]\n\n", executable_name);
    printf("Options:\n");
    printf("  -i, --input FILE    Specify input file (default: from config)\n");
    printf("  -o, --output FILE   Specify output file (default: from config)\n");
    printf("  -s, --no-screen     Disable output to screen\n");
    printf("  -f, --no-file       Disable output to file\n");
    printf("  -h, --help          Display this help message\n");
}