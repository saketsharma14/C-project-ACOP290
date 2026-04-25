#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

ParsedInput parse_input(const char *input) {
    ParsedInput result;

    // Initialize safely
    result.type = INPUT_INVALID;
    result.cmd = CMD_NONE;
    result.target[0] = '\0';
    result.expression[0] = '\0';
    result.arg[0] = '\0';

    if (!input || strlen(input) == 0) return result;

    // ---------- 1. Single character commands ----------
    if (strlen(input) == 1) {
        switch (input[0]) {
            case 'w': result.type = INPUT_COMMAND; result.cmd = CMD_UP; return result;
            case 'a': result.type = INPUT_COMMAND; result.cmd = CMD_LEFT; return result;
            case 's': result.type = INPUT_COMMAND; result.cmd = CMD_DOWN; return result;
            case 'd': result.type = INPUT_COMMAND; result.cmd = CMD_RIGHT; return result;
            case 'q': result.type = INPUT_COMMAND; result.cmd = CMD_QUIT; return result;
        }
    }

    // ---------- 2. Word commands ----------
    if (strcmp(input, "enable_output") == 0) {
        result.type = INPUT_COMMAND;
        result.cmd = CMD_ENABLE_OUTPUT;
        return result;
    }

    if (strcmp(input, "disable_output") == 0) {
        result.type = INPUT_COMMAND;
        result.cmd = CMD_DISABLE_OUTPUT;
        return result;
    }

    // ---------- 3. scroll_to command ----------
    if (strncmp(input, "scroll_to ", 10) == 0) {
        result.type = INPUT_COMMAND;
        result.cmd = CMD_SCROLL_TO;

        const char *cell = input + 10;

        // copy argument
        strncpy(result.arg, cell, sizeof(result.arg) - 1);
        result.arg[sizeof(result.arg) - 1] = '\0';

        return result;
    }

    // ---------- 4. Formula ----------
    const char *eq = strchr(input, '=');
    if (eq) {
        // split LHS and RHS
        int lhs_len = eq - input;
        int rhs_len = strlen(eq + 1);

        if (lhs_len <= 0 || rhs_len <= 0) {
            return result; // invalid
        }

        // copy target
        strncpy(result.target, input, lhs_len);
        result.target[lhs_len] = '\0';

        // copy expression
        strncpy(result.expression, eq + 1, sizeof(result.expression) - 1);
        result.expression[sizeof(result.expression) - 1] = '\0';

        result.type = INPUT_FORMULA;
        return result;
    }

    return result;
}

