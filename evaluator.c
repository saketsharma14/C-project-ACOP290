#include "evaluator.h"
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

static void skip_spaces() {
    while (*p && isspace(*p)) p++;
}

static int parse_expr(bool *err);

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

static int parse_factor(bool *err) {
    skip_spaces();

    if (*p == '(') {
        p++; // consume '('
        int val = parse_expr(err);

        skip_spaces();
        if (*p != ')') {
            *err = true;
            return 0;
        }
        p++; // consume ')'
        return val;
    }

    return parse_number(err);
}

static int parse_term(bool *err) {
    int val = parse_factor(err);

    while (1) {
        skip_spaces();

        if (*p == '*') {
            p++;
            int rhs = parse_factor(err);
            val *= rhs;
        } else if (*p == '/') {
            p++;
            int rhs = parse_factor(err);
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

static int parse_expr(bool *err) {
    int val = parse_term(err);

    while (1) {
        skip_spaces();

        if (*p == '+') {
            p++;
            int rhs = parse_term(err);
            val += rhs;
        } else if (*p == '-') {
            p++;
            int rhs = parse_term(err);
            val -= rhs;
        } else {
            break;
        }
    }

    return val;
}

EvalResult evaluate_expression(Sheet *sheet, const char *expr) {
    (void)sheet; // unused for now (Phase 2)

    EvalResult res;
    res.result = 0;
    res.is_err = false;

    if (expr == NULL) {
        res.is_err = true;
        return res;
    }

    p = expr;

    bool err = false;
    int value = parse_expr(&err);

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