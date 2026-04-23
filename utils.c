#include "utils.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> 

bool cell_to_index(const char *cell, int *row, int *col) {
    char col_part[4]="$$$";
    char row_part[4]="$$$";

    const char *p = cell;

    int col_idx=0;
    int row_idx=0;

    while(*p!='\0'){
        if (isalpha(*p)){
            col_part[col_idx]=*p;
            col_idx++;
        }
        if (isdigit(*p)){
            row_part[row_idx]=*p;
            row_idx++;
        }

        p++;
    }
    col_part[col_idx]='\0';
    row_part[row_idx]='\0';

    int col_val = 0;
    for (int i = 0; col_part[i] != '\0'; i++) {
        col_val = col_val * 26 + (col_part[i] - 'A' + 1);
    }

    int row_val = atoi(row_part);

    *col=col_val;
    *row=row_val;


    return true;
}

void index_to_cell(int row, int col, char *buffer) {
    // TODO
}

bool parse_range(const char *range,
                 int *r1, int *c1,
                 int *r2, int *c2) {
    // TODO
    return false;
}

int main(){

    const char *cell="SG423";
    int row;
    int col;

    cell_to_index(cell,&row,&col);
    printf("%d %d\n",row,col);

    return 0;
}