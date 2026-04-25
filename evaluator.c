#include "evaluator.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
Grammar (recursive descent):

expr    → term ((+ | -) term)*
term    → factor ((* | /) factor)*
factor  → NUMBER | '(' expr ')'
*/

static const char *p; // global pointer for parsing
static int parse_cell_reference(Sheet *sheet, bool *err);

static void skip_spaces() {
    while (*p && isspace(*p)) p++;
}

static int parse_expr(Sheet *sheet, bool *err);

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

static int parse_factor(Sheet *sheet,bool *err) {
    skip_spaces();

    if (*p == '(') {
        p++; // consume '('
        int val = parse_expr(sheet,err);

        skip_spaces();
        if (*p != ')') {
            *err = true;
            return 0;
        }
        p++; // consume ')'
        return val;
    }

    if (isalpha(*p)) {
        return parse_cell_reference(sheet, err);
    }

    return parse_number(err);
}

static int parse_term(Sheet *sheet,bool *err) {
    int val = parse_factor(sheet,err);

    while (1) {
        skip_spaces();

        if (*p == '*') {
            p++;
            int rhs = parse_factor(sheet,err);
            val *= rhs;
        } else if (*p == '/') {
            p++;
            int rhs = parse_factor(sheet,err);
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

static int parse_expr(Sheet *sheet,bool *err) {
    int val = parse_term(sheet,err);

    while (1) {
        skip_spaces();

        if (*p == '+') {
            p++;
            int rhs = parse_term(sheet,err);
            val += rhs;
        } else if (*p == '-') {
            p++;
            int rhs = parse_term(sheet,err);
            val -= rhs;
        } else {
            break;
        }
    }

    return val;
}

EvalResult evaluate_expression(Sheet *sheet, const char *expr) {

    EvalResult res;
    res.result = 0;
    res.is_err = false;

    if (expr == NULL) {
        res.is_err = true;
        return res;
    }

    p = expr;

    bool err = false;
    int value = parse_expr(sheet,&err);

    skip_spaces();

    // if anything left → invalid
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
        // Look for pattern: LETTERS + DIGITS (e.g., A1, AB12)
        if (isalpha(*p)) {
            char buffer[16];
            int i = 0;

            // letters (column)
            while (isalpha(*p) && i < 15) {
                buffer[i++] = *p++;
            }

            // digits (row)
            if (isdigit(*p)) {
                while (isdigit(*p) && i < 15) {
                    buffer[i++] = *p++;
                }

                buffer[i] = '\0';

                // store dependency
                strcpy(deps[count++], buffer);
            }
        } else {
            p++;
        }
    }

    return count;
}

static int parse_cell_reference(Sheet *sheet, bool *err) {
    skip_spaces();

    char buffer[16];
    int i = 0;

    // read column letters
    while (isalpha(*p) && i < 15) {
        buffer[i++] = *p++;
    }

    // must have digits
    if (!isdigit(*p)) {
        *err = true;
        return 0;
    }

    // read row digits
    while (isdigit(*p) && i < 15) {
        buffer[i++] = *p++;
    }

    buffer[i] = '\0';

    int row, col;
    if (!cell_to_index(buffer, &row, &col)) {
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