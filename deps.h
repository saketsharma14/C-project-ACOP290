#ifndef DEPS_H
#define DEPS_H

void init_deps(int total_cells);
void add_dependency(int from, int to);
void remove_dependencies(int cell);
int detect_cycle(int start);
int get_recalc_order(int start, int *order);

#endif