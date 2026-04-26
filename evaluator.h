#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "sheet.h"
#include <stdbool.h>

typedef enum {
    EVAL_OK,
    EVAL_ERR_GENERIC,
    EVAL_ERR_INVALID_RANGE,
    EVAL_ERR_DIV_ZERO,
    EVAL_ERR_CIRCULAR   // not used here but useful for completeness
} EvalError;

typedef struct {
    int result;
    bool is_err;
    EvalError err_type;
} EvalResult;

// Evaluate expression string
EvalResult evaluate_expression(Sheet *sheet, const char *expr);

// Extract referenced cells (for deps)
int extract_dependencies(const char *expr, char deps[][16]);

#endif