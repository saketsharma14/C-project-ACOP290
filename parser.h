#ifndef PARSER_H
#define PARSER_H

typedef enum {
    INPUT_INVALID,
    INPUT_COMMAND,
    INPUT_FORMULA
} InputType;

typedef enum {
    CMD_NONE,
    CMD_UP,
    CMD_DOWN,
    CMD_LEFT,
    CMD_RIGHT,
    CMD_SCROLL_TO,
    CMD_ENABLE_OUTPUT,
    CMD_DISABLE_OUTPUT,
    CMD_QUIT
} CommandType;

typedef struct {
    InputType type;

    // For formula
    char target[16];      // e.g., "A1"
    char expression[128]; // RHS

    // For command
    CommandType cmd;
    char arg[16]; // e.g., "B2" for scroll_to

} ParsedInput;

ParsedInput parse_input(const char *input);

#endif