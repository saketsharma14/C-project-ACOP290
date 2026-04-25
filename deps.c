#include "deps.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static int **graph = NULL;      // graph[from][to] = 1 means from -> to
static int total = 0;

/* ---------- internal helpers ---------- */

static void free_graph(void) {
    if (graph == NULL) return;

    for (int i = 0; i < total; i++) {
        free(graph[i]);
    }
    free(graph);
    graph = NULL;
    total = 0;
}

static bool in_bounds(int cell) {
    return cell >= 0 && cell < total;
}

static bool dfs_cycle(int node, bool *visited, bool *rec_stack) {
    if (!visited[node]) {
        visited[node] = true;
        rec_stack[node] = true;

        for (int next = 0; next < total; next++) {
            if (graph[node][next]) {
                if (!visited[next] && dfs_cycle(next, visited, rec_stack)) {
                    return true;
                }
                if (rec_stack[next]) {
                    return true;
                }
            }
        }
    }

    rec_stack[node] = false;
    return false;
}

static void dfs_order(int node, bool *visited, int *order, int *idx) {
    visited[node] = true;

    for (int next = 0; next < total; next++) {
        if (graph[node][next] && !visited[next]) {
            dfs_order(next, visited, order, idx);
        }
    }

    order[(*idx)++] = node;
}

/* ---------- public API ---------- */

void init_deps(int total_cells) {
    free_graph();

    if (total_cells <= 0) {
        return;
    }

    total = total_cells;
    graph = (int **)malloc(total * sizeof(int *));
    if (!graph) {
        total = 0;
        return;
    }

    for (int i = 0; i < total; i++) {
        graph[i] = (int *)calloc(total, sizeof(int));
        if (!graph[i]) {
            free_graph();
            return;
        }
    }
}

void add_dependency(int from, int to) {
    if (!graph) return;
    if (!in_bounds(from) || !in_bounds(to)) return;

    graph[from][to] = 1;
}

void remove_dependencies(int cell) {
    if (!graph) return;
    if (!in_bounds(cell)) return;

    // Remove all incoming edges to `cell`
    for (int from = 0; from < total; from++) {
        graph[from][cell] = 0;
    }
}

int detect_cycle(int start) {
    if (!graph) return 0;
    if (!in_bounds(start)) return 0;

    bool *visited = (bool *)calloc(total, sizeof(bool));
    bool *rec_stack = (bool *)calloc(total, sizeof(bool));

    if (!visited || !rec_stack) {
        free(visited);
        free(rec_stack);
        return 0;
    }

    int has_cycle = dfs_cycle(start, visited, rec_stack) ? 1 : 0;

    free(visited);
    free(rec_stack);
    return has_cycle;
}

int get_recalc_order(int start, int *order) {
    if (!graph || !order) return 0;
    if (!in_bounds(start)) return 0;

    bool *visited = (bool *)calloc(total, sizeof(bool));
    int *post = (int *)malloc(total * sizeof(int));

    if (!visited || !post) {
        free(visited);
        free(post);
        return 0;
    }

    int idx = 0;
    dfs_order(start, visited, post, &idx);

    // Reverse postorder gives dependency-safe recalculation order
    for (int i = 0; i < idx; i++) {
        order[i] = post[idx - 1 - i];
    }

    free(visited);
    free(post);
    return idx;
}