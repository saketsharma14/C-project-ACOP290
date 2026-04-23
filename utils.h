#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

// Convert "A1" → row, col (0-based)
bool cell_to_index(const char *cell, int *row, int *col);

// Convert row, col → "A1"
void index_to_cell(int row, int col, char *buffer);

// Parse range "A1:B2"
bool parse_range(const char *range,
                 int *r1, int *c1,
                 int *r2, int *c2);

#endif