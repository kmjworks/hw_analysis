#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../incl/runtime_configuration_handler.h"
#include "test_utils.h"

void test_config_load(TestResults *results);
void test_cli_arguments(TestResults *results);

#define TEST_CONFIG_FILE "test_config.txt"

int main() {
    TestResults results;
    init_test_results(&results);
    
    printf("\n--- Testing Runtime Configuration Handler ---\n\n");
    
    test_config_load(&results);
    test_cli_arguments(&results);
    
    print_test_summary(&results);
    
    /*
        Cleanupppppppppppppp
    */
    unlink(TEST_CONFIG_FILE);
    
    return results.tests_failed > 0 ? 1 : 0;
}

void create_test_config_file() {
    FILE *file = fopen(TEST_CONFIG_FILE, "w");
    if (file) {
        fprintf(file, "# Test Configuration File\n");
        fprintf(file, "input_file=test_input.txt\n");
        fprintf(file, "output_file=test_output.txt\n");
        fprintf(file, "stdout_output=1\n");
        fprintf(file, "file_output=1\n");
        fprintf(file, "# Comment line\n");
        fprintf(file, "\n");
        fclose(file);
    }
}

/*
    Tests for the function that is responsible for loading the runtime configuration of the program
    (i.e. if file output is enabled, does the user want the results to be logged into the console etc..)
*/
void test_config_load(TestResults *results) {
    printf("Testing configuration loading...\n");
    
    create_test_config_file();
    
    /*
        Make sure that a valid runtime configuration gets loaded as expected
    */
    FileSettings settings;
    runtime_config_load_handler(&settings, TEST_CONFIG_FILE);
    
    ASSERT_STRING_EQUAL("Input file path correctly loaded", "test_input.txt", settings.input_file);
    ASSERT_STRING_EQUAL("Output file path correctly loaded", "test_output.txt", settings.output_file);
    ASSERT_INT_EQUAL("Stdout output enabled flag correctly loaded", 1, settings.stdout_output_enabled);
    ASSERT_INT_EQUAL("File output enabled flag correctly loaded", 1, settings.file_output_enabled);
    
    /*
        Test the configuration loading with a non-existent file - in case there is no explicit file provided for
        configuration, the program should fall back to using default parameters for runtime 
    */
    FileSettings default_settings;
    runtime_config_load_handler(&default_settings, "nonexistent_file.txt");
    
    /*
        Verify that the program falls back to defaults:
            (logging the results out to a .txt file is enabled, logging the analysis result to stdout i.e. to console
            is enabled)
    */
    ASSERT_STRING_EQUAL("Default input file used for non-existent config", "bus_lines_data.txt", default_settings.input_file);
    ASSERT_TRUE("Default output file set for non-existent config", strlen(default_settings.output_file) > 0);
    ASSERT_INT_EQUAL("Default stdout output enabled", 1, default_settings.stdout_output_enabled);
    ASSERT_INT_EQUAL("Default file output enabled", 1, default_settings.file_output_enabled);
    
    /*
        Check error handling and fallback for a configuration file that contains invalid runtime configuration arguments,
        e.g. no filename provided for a input data file, and invalid or non-existent values for result logging
    */
    FILE *file = fopen(TEST_CONFIG_FILE, "w");
    if (file) {
        fprintf(file, "input_file=\n");  
        fprintf(file, "stdout_output=invalid\n");
        fprintf(file, "unknown_key=value\n"); 
        fclose(file);
        
        FileSettings invalid_settings;
        strcpy(invalid_settings.input_file, "");
        runtime_config_load_handler(&invalid_settings, TEST_CONFIG_FILE);
        

        /*
            The load handler should handle all the invalid parameters gracefully and default
        */
        ASSERT_TRUE("Handler handles empty input file value", strlen(invalid_settings.input_file) > 0);
    }
}

/*
    Tests for verifying the implementation of the function that is responsible
    for parsing arguments from the command line
*/
void test_cli_arguments(TestResults *results) {
    printf("Testing CLI argument handling...\n");
    
    /*
        Verify the parser with different filename arguments for both the input and output file
    */
    FileSettings settings;
    strcpy(settings.input_file, "default_input.txt");
    strcpy(settings.output_file, "default_output.txt");
    settings.stdout_output_enabled = 1;
    settings.file_output_enabled = 1;
    
    /*
        Test case 1: validate that the input file (name) is accepted as a valid command line argument
    */
    char *argv1[] = {"program", "-i", "cli_input.txt"};
    cli_argument_handler(3, argv1, &settings);
    ASSERT_STRING_EQUAL("Input file from CLI argument", "cli_input.txt", settings.input_file);
    
    /*
        Test case 2: verify that the output file (name) is accepted and parsed as a command line argument
    */
    char *argv2[] = {"program", "-o", "cli_output.txt"};
    cli_argument_handler(3, argv2, &settings);
    ASSERT_STRING_EQUAL("Output file from CLI argument", "cli_output.txt", settings.output_file);
    
    
    /*
        Test case 3: if the user does not want the output to be logged to stdout, this functionality should be
        disabled before the analysis starts execution - the analysis should be conducted in "quiet" mode i.e. 
        any kind of output must be abscent from the console
    */
    char *argv3[] = {"program", "-s"};
    cli_argument_handler(2, argv3, &settings);
    ASSERT_INT_EQUAL("Stdout disabled by CLI argument", 0, settings.stdout_output_enabled);
    
    /*
        Test case 4: if the user disabled logging the results to an output file, the program should not create a 
        .txt file that it would otherwise use for logging the results of the analysis - after taking this option
        into account, it should proceed to conducting analysis as usual
    */
    char *argv4[] = {"program", "-f"};
    cli_argument_handler(2, argv4, &settings);
    ASSERT_INT_EQUAL("File output disabled by CLI argument", 0, settings.file_output_enabled);
    
    /*
        Test case 5: make sure that the parser can handle multiple command line arguments at once
    */
    settings.stdout_output_enabled = 1;
    settings.file_output_enabled = 1;
    char *argv5[] = {"program", "-i", "multi_input.txt", "-o", "multi_output.txt", "-s", "-f"};
    cli_argument_handler(7, argv5, &settings);
    ASSERT_STRING_EQUAL("Input file from multiple CLI arguments", "multi_input.txt", settings.input_file);
    ASSERT_STRING_EQUAL("Output file from multiple CLI arguments", "multi_output.txt", settings.output_file);
    ASSERT_INT_EQUAL("Stdout disabled from multiple CLI arguments", 0, settings.stdout_output_enabled);
    ASSERT_INT_EQUAL("File output disabled from multiple CLI arguments", 0, settings.file_output_enabled);
    
    /*
        Test case 6: make sure the parser accepts the command line arguments independent of their ordering
        and interprets them correctly in terms of the command line argument itself - e.g. if the input file is
        passed either as a first or second command line argument, it should: 
        1) be accepted as a valid command line argument (if the appropriate flag was used)
        2) be interpreted (or understood) as a input file from which data for analysis is parsed from
    */
    char *argv6[] = {"program", "positional_input.txt"};
    strcpy(settings.input_file, "default_input.txt");
    cli_argument_handler(2, argv6, &settings);
    ASSERT_STRING_EQUAL("Input file from positional argument", "positional_input.txt", settings.input_file);
    
    /*
        Test 7: just in case make sure that an invalid or unknown command line argument does not just 
        downright crash the program without any indication of what the user's error might have been
    */
    char *argv7[] = {"program", "--unknown"};
    cli_argument_handler(2, argv7, &settings);
    ASSERT_TRUE("Unknown option handled gracefully", 1); // Just check we didn't crash
}
