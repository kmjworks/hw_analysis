#include <stdio.h>
#include <stdlib.h>
#include "bus_line_handler.h"

#define COST_PER_KM 2.50
#define ADULT_TICKET 12.0
#define STUDENT_TICKET 8.0
#define SENIOR_TICKET 5.0
#define LEVEL1_SUBSIDY 0.50
#define LEVEL2_SUBSIDY 1.00
#define LEVEL3_SUBSIDY 1.50

void calculate_profitability(BusLineProperties *bus_lines, int count) {
    for(int i = 0; i < count; ++i) {
    
        BusLineProperties* line = &bus_lines[i];
        double cost_per_km = line->route_length * COST_PER_KM;
        double revenue_from_tickets = 0.0, subsidy_from_gov = 0.0;

        revenue_from_tickets += line->passengers.adult * ADULT_TICKET;
        revenue_from_tickets += line->passengers.student * STUDENT_TICKET;
        revenue_from_tickets += (line->subsidy_level == 1) ? line->passengers.senior * SENIOR_TICKET : 0;

        switch(line->subsidy_level) {
            case 1:
                subsidy_from_gov = line->route_length * LEVEL1_SUBSIDY;
                break;
            case 2:
                subsidy_from_gov = line->route_length * LEVEL2_SUBSIDY;
                break;
            case 3:
                subsidy_from_gov = line->route_length * LEVEL3_SUBSIDY;
                break;
        }

        line->profitability = revenue_from_tickets + subsidy_from_gov - cost_per_km;
    }
}

/*
    Quicksort implementation for comparing bus lines first by subsidy level and then by profitability
    (in descending order i.e. most profitable bus lines first)
*/
static int compare_bus_lines_handler(const void *comparator_a, const void* comparator_b) {
    const BusLineProperties *bus_line_a = (const BusLineProperties *)comparator_a;
    const BusLineProperties *bus_line_b = (const BusLineProperties *)comparator_b;

    if (bus_line_a->subsidy_level != bus_line_b->subsidy_level) return bus_line_a->subsidy_level - bus_line_b->subsidy_level;
    if (bus_line_b->profitability > bus_line_a->profitability) return 1;
    if (bus_line_b->profitability < bus_line_a->profitability) return -1;

    return 0;
}

void sort_lines(BusLineProperties *bus_lines, int count) {
    qsort(bus_lines, count, sizeof(BusLineProperties), compare_bus_lines_handler);
}

void display_result_handler(const BusLineProperties *bus_lines, int count) {
    printf("Bus Lines' Profitability Report\n");
    printf("--------------------------------\n\n");

    if(count == 0) {
        printf("[!] No bus lines to display. Exiting...\n");
        return;
    }

    int current_subsidy_level = -1;

    for(int i = 0; i < count; ++i) {
        const BusLineProperties *line = &bus_lines[i];

        if(line->subsidy_level != current_subsidy_level) {
            current_subsidy_level = line->subsidy_level;
            printf("\n[*] Subsidy Level %d:\n", current_subsidy_level);
            printf("------------------------------------------------------------------\n");
            printf("Line\tTime\tPassengers (A+S+Sr)\tLength(km)\tProfit(€)\n");
            printf("------------------------------------------------------------------\n");
        }
        
        printf("%d\t%s\t%d+%d+%d\t\t%.1f\t\t%.2f\n", 
            line->line_number,
            line->departure_time,
            line->passengers.adult,
            line->passengers.student,
            line->passengers.senior,
            line->route_length,
            line->profitability);

    }
}