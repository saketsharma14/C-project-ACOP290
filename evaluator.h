#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "sheet.h"
#include <stdbool.h>

typedef struct {
    int result;
    bool is_err;
} EvalResult;

// Evaluate expression string
EvalResult evaluate_expression(Sheet *sheet, const char *expr);

// Extract referenced cells (for deps)
int extract_dependencies(const char *expr, char deps[][16]);

#endif