#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parser.h"
#include "sheet.h"
#include "utils.h"
#include "commands.h"
#include "evaluator.h"

#define INPUT_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: ./sheet R C\n");
        return 1;
    }

    int R = atoi(argv[1]);
    int C = atoi(argv[2]);

    if (R < 1 || R > 999 || C < 1 || C > 18278) {
        printf("Invalid sheet size\n");
        return 1;
    }

    Sheet *sheet = create_sheet(R, C);

    char input[INPUT_SIZE];
    char status[64] = "ok";

    print_sheet(sheet);
    printf("[0.0] (%s) > ", status);

    while (fgets(input, sizeof(input), stdin)) {
        input[strcspn(input, "\n")] = '\0';

        clock_t start = clock();

        ParsedInput p = parse_input(input);

        if (p.type == INPUT_INVALID) {
            strcpy(status, "unrecognized cmd");
        }

        else if (p.type == INPUT_COMMAND) {
            if (p.cmd == CMD_QUIT) break;

            execute_command(sheet, &p);
            strcpy(status, "ok");
        }

        else if (p.type == INPUT_FORMULA) {
            int row, col;

            if (!cell_to_index(p.target, &row, &col)) {
                strcpy(status, "invalid cell");
            } else {
                EvalResult res = evaluate_expression(sheet, p.expression);

                if (res.is_err) {
                    set_cell_value(sheet, row, col, 0, true);
                } else {
                    set_cell_value(sheet, row, col, res.result, false);
                }

                strcpy(get_cell(sheet, row, col)->formula, p.expression);
                strcpy(status, "ok");
            }
        }

        clock_t end = clock();
        double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

        if (sheet->output_enabled) {
            print_sheet(sheet);
        }

        printf("[%.1f] (%s) > ", time_taken, status);
    }

    free_sheet(sheet);
    return 0;
}