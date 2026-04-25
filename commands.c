#include "commands.h"
#include "utils.h"
#include <stdio.h>

void execute_command(Sheet *sheet, ParsedInput *input) {
    if (!sheet || !input) return;

    switch (input->cmd) {

        case CMD_UP:
            if (sheet->scroll_row >= 10)
                sheet->scroll_row -= 10;
            else
                sheet->scroll_row = 0;
            break;

        case CMD_DOWN:
            if (sheet->scroll_row + 10 < sheet->rows)
                sheet->scroll_row += 10;
            break;

        case CMD_LEFT:
            if (sheet->scroll_col >= 10)
                sheet->scroll_col -= 10;
            else
                sheet->scroll_col = 0;
            break;

        case CMD_RIGHT:
            if (sheet->scroll_col + 10 < sheet->cols)
                sheet->scroll_col += 10;
            break;

        case CMD_SCROLL_TO: {
            int row, col;
            if (!cell_to_index(input->arg, &row, &col)) {
                printf("(Invalid cell)\n");
                return;
            }

            if (row < 0 || row >= sheet->rows ||
                col < 0 || col >= sheet->cols) {
                printf("(Invalid cell)\n");
                return;
            }

            sheet->scroll_row = row;
            sheet->scroll_col = col;
            break;
        }

        case CMD_ENABLE_OUTPUT:
            sheet->output_enabled = true;
            break;

        case CMD_DISABLE_OUTPUT:
            sheet->output_enabled = false;
            break;

        default:
            break;
    }
}
