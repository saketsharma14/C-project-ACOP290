#include "evaluator.h"
#include "utils.h"
#include "sheet.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> // sleep

/*
Grammar:

expr    → term ((+ | -) term)*
term    → factor ((* | /) factor)*
factor  → NUMBER | CELL | FUNCTION | '(' expr ')'
*/

static const char *p;

static void skip_spaces() {
    while (*p && isspace(*p)) p++;
}

static int parse_expr(Sheet *sheet, bool *err);
static int parse_function(Sheet *sheet, const char *name, bool *err);

/* ---------- NUMBER ---------- */

static int parse_number(bool *err) {
    skip_spaces();

    if (!isdigit(*p)) {
        *err = true;
        return 0;
    }

    int val = 0;
    while (isdigit(*p)) {
        val = val * 10 + (*p - '0');
        p++;
    }

    return val;
}

/* ---------- CELL ---------- */

static int parse_cell_reference(Sheet *sheet, bool *err) {
    skip_spaces();

    char buffer[16];
    int i = 0;

    while (isalpha(*p) && i < 15) {
        buffer[i++] = *p++;
    }

    if (!isdigit(*p)) {
        *err = true;
        return 0;
    }

    while (isdigit(*p) && i < 15) {
        buffer[i++] = *p++;
    }

    buffer[i] = '\0';

    int row, col;
    if (!cell_to_index(buffer, &row, &col)) {
        *err = true;
        return 0;
    }

    if (row < 0 || row >= sheet->rows ||
        col < 0 || col >= sheet->cols) {
        *err = true;
        return 0;
    }

    Cell *cell = get_cell(sheet, row, col);
    if (!cell || cell->is_err) {
        *err = true;
        return 0;
    }

    return cell->value;
}

/* ---------- FACTOR ---------- */

static int parse_factor(Sheet *sheet, bool *err) {
    skip_spaces();

    // (expr)
    if (*p == '(') {
        p++;
        int val = parse_expr(sheet, err);

        skip_spaces();
        if (*p != ')') {
            *err = true;
            return 0;
        }
        p++;
        return val;
    }

    // FUNCTION or CELL
    if (isalpha(*p)) {
        char name[16];
        int i = 0;

        while (isalpha(*p) && i < 15) {
            name[i++] = *p++;
        }
        name[i] = '\0';

        skip_spaces();

        if (*p == '(') {
            return parse_function(sheet, name, err);
        }

        // fallback → treat as cell
        p -= i; // rewind
        return parse_cell_reference(sheet, err);
    }

    return parse_number(err);
}

/* ---------- TERM ---------- */

static int parse_term(Sheet *sheet, bool *err) {
    int val = parse_factor(sheet, err);

    while (1) {
        skip_spaces();

        if (*p == '*') {
            p++;
            int rhs = parse_factor(sheet, err);
            val *= rhs;
        } else if (*p == '/') {
            p++;
            int rhs = parse_factor(sheet, err);
            if (rhs == 0) {
                *err = true;
                return 0;
            }
            val /= rhs;
        } else {
            break;
        }
    }

    return val;
}

/* ---------- EXPR ---------- */

static int parse_expr(Sheet *sheet, bool *err) {
    int val = parse_term(sheet, err);

    while (1) {
        skip_spaces();

        if (*p == '+') {
            p++;
            int rhs = parse_term(sheet, err);
            val += rhs;
        } else if (*p == '-') {
            p++;
            int rhs = parse_term(sheet, err);
            val -= rhs;
        } else {
            break;
        }
    }

    return val;
}

/* ---------- FUNCTION ---------- */

static int parse_function(Sheet *sheet, const char *name, bool *err) {
    p++; // skip '('

    skip_spaces();

    // -------- SLEEP --------
    if (strcmp(name, "SLEEP") == 0) {
        int val = parse_expr(sheet, err);

        skip_spaces();
        if (*p != ')') {
            *err = true;
            return 0;
        }
        p++;

        if (!(*err)) {
            sleep(val);
        }

        return val;
    }

    // -------- RANGE FUNCTIONS --------

    char start[16], end[16];
    int i = 0;

    while (*p && *p != ':' && *p != ')' && i < 15) {
        start[i++] = *p++;
    }
    start[i] = '\0';

    if (*p != ':') {
        *err = true;
        return 0;
    }
    p++;

    i = 0;
    while (*p && *p != ')' && i < 15) {
        end[i++] = *p++;
    }
    end[i] = '\0';

    if (*p != ')') {
        *err = true;
        return 0;
    }
    p++;

    int r1, c1, r2, c2;
    char range[40];
    snprintf(range, sizeof(range), "%s:%s", start, end);

    if (!parse_range(range, &r1, &c1, &r2, &c2)) {
        *err = true;
        return 0;
    }

    if (r1 > r2 || c1 > c2) {
        *err = true;
        return 0;
    }

    int count = 0;
    int sum = 0;
    int min = 0, max = 0;

    for (int r = r1; r <= r2; r++) {
        for (int c = c1; c <= c2; c++) {
            if (r < 0 || r >= sheet->rows ||
                c < 0 || c >= sheet->cols) {
                *err = true;
                return 0;
            }

            Cell *cell = get_cell(sheet, r, c);

            if (cell->is_err) {
                *err = true;
                return 0;
            }

            int v = cell->value;

            if (count == 0) {
                min = max = v;
            } else {
                if (v < min) min = v;
                if (v > max) max = v;
            }

            sum += v;
            count++;
        }
    }

    if (count == 0) {
        *err = true;
        return 0;
    }

    if (strcmp(name, "SUM") == 0) return sum;
    if (strcmp(name, "AVG") == 0) return sum / count;
    if (strcmp(name, "MIN") == 0) return min;
    if (strcmp(name, "MAX") == 0) return max;

    *err = true;
    return 0;
}

/* ---------- ENTRY ---------- */

EvalResult evaluate_expression(Sheet *sheet, const char *expr) {
    EvalResult res;
    res.result = 0;
    res.is_err = false;

    if (!expr) {
        res.is_err = true;
        return res;
    }

    p = expr;

    bool err = false;
    int value = parse_expr(sheet, &err);

    skip_spaces();

    if (*p != '\0') {
        err = true;
    }

    if (err) {
        res.is_err = true;
        return res;
    }

    res.result = value;
    return res;
}

int extract_dependencies(const char *expr, char deps[][16]) {
    int count = 0;

    const char *p = expr;

    while (*p) {
        if (isalpha(*p)) {
            char buffer[16];
            int i = 0;

            // letters
            while (isalpha(*p) && i < 15) {
                buffer[i++] = *p++;
            }

            // digits
            if (isdigit(*p)) {
                while (isdigit(*p) && i < 15) {
                    buffer[i++] = *p++;
                }

                buffer[i] = '\0';

                strcpy(deps[count++], buffer);
            }
        } else {
            p++;
        }
    }

    return count;
}