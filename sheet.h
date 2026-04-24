#ifndef SHEET_H
#define SHEET_H

#include <stdbool.h>

typedef struct {
    int value;
    bool is_err;
    char formula[128];
} Cell;

typedef struct {
    int rows;
    int cols;
    Cell **grid;

    int scroll_row;
    int scroll_col;

    bool output_enabled;
} Sheet;

Sheet* create_sheet(int rows, int cols);
void free_sheet(Sheet *sheet);

Cell* get_cell(Sheet *sheet, int row, int col);
void set_cell_value(Sheet *sheet, int row, int col, int value, bool is_err);

void print_sheet(Sheet *sheet);

#endif