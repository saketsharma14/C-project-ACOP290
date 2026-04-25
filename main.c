#include "commands.h"
#include "utils.h"
#include "sheet.h"
#include "parser.h"
#include <stdio.h>

int main() {
    // create sheet
    Sheet *sheet = create_sheet(5, 5);

    if (!sheet) {
        printf("Failed to create sheet\n");
        return 1;
    }

   ParsedInput p;

    p = parse_input("d");
    execute_command(sheet, &p);

    p = parse_input("s");
    execute_command(sheet, &p);

    p = parse_input("scroll_to B2");
    execute_command(sheet, &p);

    print_sheet(sheet);
}