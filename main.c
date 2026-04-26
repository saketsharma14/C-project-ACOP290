#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parser.h"
#include "sheet.h"
#include "utils.h"
#include "commands.h"
#include "evaluator.h"
#include "deps.h"

#define INPUT_SIZE 256

int cell_to_id(int row, int col, int cols)
{
    return row * cols + col;
}
int id_to_row(int id, int cols)
{
    return id / cols;
}

int id_to_col(int id, int cols)
{
    return id % cols;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./sheet R C\n");
        return 1;
    }

    int R = atoi(argv[1]);
    int C = atoi(argv[2]);

    if (R < 1 || R > 999 || C < 1 || C > 18278)
    {
        printf("Invalid sheet size\n");
        return 1;
    }

    Sheet *sheet = create_sheet(R, C);
    init_deps(R * C);

    char input[INPUT_SIZE];
    char status[64] = "ok";

    print_sheet(sheet);
    printf("[0.0] (%s) > ", status);

    while (fgets(input, sizeof(input), stdin))
    {
        input[strcspn(input, "\n")] = '\0';

        clock_t start = clock();

        ParsedInput p = parse_input(input);

        if (p.type == INPUT_INVALID)
        {
            strcpy(status, "unrecognized cmd");
        }

        else if (p.type == INPUT_COMMAND)
        {
            if (p.cmd == CMD_QUIT)
                break;

            execute_command(sheet, &p);
            strcpy(status, "ok");
        }

        else if (p.type == INPUT_FORMULA)
        {
            int row, col;

            if (!cell_to_index(p.target, &row, &col) ||
                row < 0 || row >= sheet->rows ||
                col < 0 || col >= sheet->cols)
            {
                strcpy(status, "invalid cell");
            }
            else
            {
                int target = cell_to_id(row, col, sheet->cols);

                // 1. Extract dependencies
                char deps_list[64][16];
                int dep_count = extract_dependencies(p.expression, deps_list);

                // 2. Add dependencies FIRST (temporary)
                int added[64];
                int added_count = 0;

                for (int i = 0; i < dep_count; i++)
                {
                    int r, c;
                    if (!cell_to_index(deps_list[i], &r, &c))
                        continue;

                    if (r < 0 || r >= sheet->rows ||
                        c < 0 || c >= sheet->cols)
                    {
                        continue;
                    }

                    int from = cell_to_id(r, c, sheet->cols);
                    add_dependency(from, target);
                    added[added_count++] = from;
                }

                // 3. Detect cycle
                if (detect_cycle(target))
                {
                    strcpy(status, "circular ref");

                    // rollback newly added edges
                    for (int i = 0; i < added_count; i++)
                    {
                        remove_dependency(added[i], target);
                    }
                }
                else
                {
                    // 4. Remove OLD dependencies (safe now)
                    remove_dependencies(target);

                    // 5. Re-add dependencies (final)
                    for (int i = 0; i < added_count; i++)
                    {
                        add_dependency(added[i], target);
                    }

                    // 6. Store formula
                    Cell *cell = get_cell(sheet, row, col);
                    strcpy(cell->formula, p.expression);

                    // 7. Get recalculation order
                    int order[R * C];
                    int count = get_recalc_order(target, order);

                    // 8. Recalculate ONLY affected cells
                    for (int i = 0; i < count; i++)
                    {
                        int id = order[i];
                        int r = id_to_row(id, sheet->cols);
                        int c = id_to_col(id, sheet->cols);

                        Cell *cur = get_cell(sheet, r, c);

                        if (strlen(cur->formula) > 0)
                        {
                            EvalResult res = evaluate_expression(sheet, cur->formula);
                            if (res.is_err)
                            {
                                set_cell_value(sheet, r, c, 0, true);
                                if (i == 0)
                                {
                                    if (res.err_type == EVAL_ERR_INVALID_RANGE)
                                        strcpy(status, "Invalid range");
                                    else if (res.err_type == EVAL_ERR_DIV_ZERO)
                                        strcpy(status, "div by 0");
                                    else
                                        strcpy(status, "err");
                                }
                            }
                            else
                            {
                                set_cell_value(sheet, r, c, res.result, false);
                            }
                        }
                    }

                    // Set ok only if target cell has no error
                    if (!get_cell(sheet, row, col)->is_err)
                        strcpy(status, "ok");
                }
            }
        }

        clock_t end = clock();
        double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

        if (sheet->output_enabled)
        {
            print_sheet(sheet);
        }

        printf("[%.1f] (%s) > ", time_taken, status);
    }

    free_sheet(sheet);
    return 0;
}