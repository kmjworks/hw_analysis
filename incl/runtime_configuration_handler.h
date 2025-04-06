#ifndef RUNTIME_CONFIGURATION_HANDLER_H
#define RUNTIME_CONFIGURATION_HANDLER_H

typedef struct {
    char input_file[256];
    char output_file[256];
    int stdout_output_enabled;
    int file_output_enabled;
} FileSettings;

void runtime_config_load_handler(FileSettings *settings, const char* configuration_file);

void cli_argument_handler(int argc, char **argv, FileSettings *settings);

void runtime_usage_print_handler(const char* executable_name);

#endif // RUNTIME_CONFIGURATION_HANDLER_H