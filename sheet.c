#include "sheet.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <ctype.h>

Sheet* create_sheet(int rows, int cols) {
    Sheet *sheet = malloc(sizeof(Sheet));
    if (!sheet) return NULL;

    sheet->rows = rows;
    sheet->cols = cols;

    sheet->scroll_row = 0;
    sheet->scroll_col = 0;
    sheet->output_enabled = true;

    sheet->grid = malloc(rows * sizeof(Cell*));
    if (!sheet->grid) return NULL;

    for (int i = 0; i < rows; i++) {
        sheet->grid[i] = malloc(cols * sizeof(Cell));
        if (!sheet->grid[i]) return NULL;

        for (int j = 0; j < cols; j++) {
            sheet->grid[i][j].value = 0;
            sheet->grid[i][j].is_err = false;
            sheet->grid[i][j].formula[0] = '\0';
        }
    }

    return sheet;
}

void free_sheet(Sheet *sheet) {
    if (!sheet) return;

    for (int i = 0; i < sheet->rows; i++) {
        free(sheet->grid[i]);
    }

    free(sheet->grid);
    free(sheet);
}

Cell* get_cell(Sheet *sheet, int row, int col) {
    if (!sheet) return NULL;

    if (row < 0 || row >= sheet->rows ||
        col < 0 || col >= sheet->cols) {
        return NULL;
    }

    return &sheet->grid[row][col];
}

void set_cell_value(Sheet *sheet, int row, int col, int value, bool is_err) {
    Cell *cell = get_cell(sheet, row, col);
    if (!cell) return;

    cell->value = value;
    cell->is_err = is_err;
}

void print_sheet(Sheet *sheet) {
    if (!sheet || !sheet->output_enabled) return;

    int r_start = sheet->scroll_row;
    int c_start = sheet->scroll_col;

    int r_end = r_start + 10;
    int c_end = c_start + 10;

    if (r_end > sheet->rows) r_end = sheet->rows;
    if (c_end > sheet->cols) c_end = sheet->cols;

    // Print column headers
    printf("   ");
    for (int c = c_start; c < c_end; c++) {
        char col_name[10];
        index_to_cell(0, c, col_name);

        // strip row number part
        int i = 0;
        while (col_name[i] && !isdigit(col_name[i])) {
            putchar(col_name[i]);
            i++;
        }
        printf(" ");
    }
    printf("\n");

    // Print rows
    for (int r = r_start; r < r_end; r++) {
        printf("%d ", r + 1);

        for (int c = c_start; c < c_end; c++) {
            Cell *cell = &sheet->grid[r][c];

            if (cell->is_err) {
                printf("ERR ");
            } else {
                printf("%d ", cell->value);
            }
        }
        printf("\n");
    }
}


