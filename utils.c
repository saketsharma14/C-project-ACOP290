#include "utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> 

bool cell_to_index(const char *cell, int *row, int *col) {
    char col_part[5] = {0};
    char row_part[6] = {0};

    int col_idx = 0, row_idx = 0;

    const char *p = cell;

    while (*p) {
        if (isalpha(*p)) {
            col_part[col_idx++] = toupper(*p);
        } else if (isdigit(*p)) {
            row_part[row_idx++] = *p;
        } else {
            return false;
        }
        p++;
    }

    if (col_idx == 0 || row_idx == 0) return false;

    int col_val = 0;
    for (int i = 0; i < col_idx; i++) {
        col_val = col_val * 26 + (col_part[i] - 'A' + 1);
    }

    int row_val = atoi(row_part);

    *col = col_val - 1;
    *row = row_val - 1;

    return true;
}

void index_to_cell(int row, int col, char *buffer) {
    char col_part[5];
    int i = 0;

    int temp = col;

    do {
        int rem = temp % 26;
        col_part[i++] = 'A' + rem;
        temp = temp / 26 - 1;
    } while (temp >= 0);

    int k = 0;
    for (int j = i - 1; j >= 0; j--) {
        buffer[k++] = col_part[j];
    }

    sprintf(buffer + k, "%d", row + 1);
}

bool parse_range(const char *range,
                 int *r1, int *c1,
                 int *r2, int *c2) 
{
    char buffer[64];
    strncpy(buffer, range, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *sep = strchr(buffer, ':');
    if (!sep) return false;

    *sep = '\0';

    char *start = buffer;
    char *end = sep + 1;

    if (strlen(start) == 0 || strlen(end) == 0) return false;

    if (!cell_to_index(start, r1, c1)) return false;
    if (!cell_to_index(end, r2, c2)) return false;

    if (*r1 > *r2 || *c1 > *c2) return false;

    return true;
}

int main(){

    char range[15]="AC23:BDF424";
    int r1,r2,c1,c2;
    parse_range(range,&r1,&c1,&r2,&c2);

    printf("%d %d %d %d \n",r1,c1,r2,c2);

    return 0;
}