#include "commands.h"
#include "utils.h"
#include "sheet.h"
#include "parser.h"
#include <stdio.h>
#include "sheet.h"
#include "evaluator.h"

void test_expr(Sheet *sheet, const char *expr) {
    EvalResult res = evaluate_expression(sheet, expr);

    if (res.is_err) {
        printf("%s -> ERR\n", expr);
    } else {
        printf("%s -> %d\n", expr, res.result);
    }
}


int main() {
    Sheet *sheet = create_sheet(10, 10);

    printf("Evaluator Tests:\n\n");

    test_expr(sheet, "2+3*4");        // 14
    test_expr(sheet, "(2+3)*4");      // 20
    test_expr(sheet, "10/2");         // 5
    test_expr(sheet, "10/0");         // ERR
    test_expr(sheet, "2+");           // ERR
    test_expr(sheet, "2 + 3 * 4");    // 14

    free_sheet(sheet);
    return 0;

}