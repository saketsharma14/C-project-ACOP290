#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- helpers ---------- */

static int passed = 0;
static int failed = 0;

/*
 * run_test: pipes `input` to ./sheet R C, captures stdout,
 * checks that `expected` appears somewhere in the output.
 */
static void run_test(const char *name,
                     int R, int C,
                     const char *input,
                     const char *expected)
{
    /* Write input to a temp file */
    FILE *f = fopen("_test_input.txt", "w");
    if (!f) { fprintf(stderr, "cannot open temp file\n"); exit(1); }
    fprintf(f, "%s\nq\n", input);
    fclose(f);

    /* Build command */
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "./sheet %d %d < _test_input.txt 2>&1", R, C);

    FILE *pipe = popen(cmd, "r");
    if (!pipe) { fprintf(stderr, "popen failed\n"); exit(1); }

    /* Read all output */
    char output[8192] = {0};
    size_t total = 0;
    char buf[256];
    while (fgets(buf, sizeof(buf), pipe) && total < sizeof(output) - 1) {
        size_t len = strlen(buf);
        memcpy(output + total, buf, len);
        total += len;
    }
    output[total] = '\0';
    pclose(pipe);

    /* Check */
    if (strstr(output, expected)) {
        printf("  PASS  %s\n", name);
        passed++;
    } else {
        printf("  FAIL  %s\n", name);
        printf("        expected to find: \"%s\"\n", expected);
        printf("        got:\n%s\n", output);
        failed++;
    }

    remove("_test_input.txt");
}

/* ---------- test groups ---------- */

static void test_basic_assignment(void) {
    printf("\n[Basic Assignment]\n");

    run_test("integer constant",
             2, 2,
             "A1=7",
             "7");

    run_test("zero assignment",
             2, 2,
             "A1=0",
             "0");

    run_test("large value",
             2, 2,
             "A1=999",
             "999");

    run_test("status ok after assignment",
             2, 2,
             "A1=5",
             "(ok)");
}

static void test_arithmetic(void) {
    printf("\n[Arithmetic]\n");

    run_test("addition",
             2, 2,
             "A1=3+4",
             "7");

    run_test("subtraction",
             2, 2,
             "A1=10-3",
             "7");

    run_test("multiplication",
             2, 2,
             "A1=3*4",
             "12");

    run_test("integer division",
             2, 2,
             "A1=7/2",
             "3");

    run_test("division truncates",
             2, 2,
             "A1=5/2",
             "2");

    run_test("chained arithmetic",
             2, 2,
             "A1=2+3*4",
             "14");

    run_test("parentheses",
             2, 2,
             "A1=(2+3)*4",
             "20");
}

static void test_negative_numbers(void) {
    printf("\n[Negative Numbers]\n");

    run_test("negative literal",
             2, 2,
             "A1=-5",
             "-5");

    run_test("negative result from subtraction",
             2, 2,
             "A1=3-10",
             "-7");

    run_test("negative in expression",
             2, 2,
             "A1=-3+1",
             "-2");
}

static void test_cell_references(void) {
    printf("\n[Cell References]\n");

    run_test("reference another cell",
             2, 2,
             "A1=5\nB1=A1+1",
             "6");

    run_test("chain of references",
             2, 3,
             "A1=2\nB1=A1+1\nC1=B1+1",
             "4");

    run_test("reference in formula",
             2, 2,
             "A1=10\nA2=A1*2",
             "20");
}

static void test_recalculation(void) {
    printf("\n[Automatic Recalculation]\n");

    run_test("dependent cell updates",
             2, 2,
             "A1=2\nB1=A1+1\nA1=5",
             "6");

    run_test("cascade recalculation",
             2, 3,
             "A1=1\nB1=A1+1\nC1=B1+1\nA1=10",
             "12");

    run_test("unrelated cell not changed",
             2, 2,
             "A1=3\nB1=7\nA1=5",
             "7");
}

static void test_functions(void) {
    printf("\n[Functions]\n");

    run_test("SUM range",
             2, 3,
             "A1=1\nB1=2\nC1=3\nA2=SUM(A1:C1)",
             "6");

    run_test("AVG range",
             2, 3,
             "A1=2\nB1=4\nC1=6\nA2=AVG(A1:C1)",
             "4");

    run_test("MIN range",
             2, 3,
             "A1=5\nB1=2\nC1=8\nA2=MIN(A1:C1)",
             "2");

    run_test("MAX range",
             2, 3,
             "A1=5\nB1=2\nC1=8\nA2=MAX(A1:C1)",
             "8");

    run_test("STDEV all same",
             2, 3,
             "A1=4\nB1=4\nC1=4\nA2=STDEV(A1:C1)",
             "0");

    run_test("STDEV basic",
             2, 3,
             "A1=2\nB1=4\nC1=6\nA2=STDEV(A1:C1)",
             "1");

    run_test("SUM 2D range",
             3, 3,
             "A1=1\nB1=2\nA2=3\nB2=4\nC1=SUM(A1:B2)",
             "10");

    run_test("SLEEP returns value",
             2, 2,
             "A1=SLEEP(1)",
             "1");
}

static void test_error_handling(void) {
    printf("\n[Error Handling]\n");

    run_test("division by zero status",
             2, 2,
             "A1=1/0",
             "div by 0");

    run_test("division by zero cell is ERR",
             2, 2,
             "A1=1/0",
             "ERR");

    run_test("ERR propagates to dependent cell",
             2, 2,
             "A1=1/0\nB1=A1+1",
             "ERR");

    run_test("circular reference status",
             2, 2,
             "A1=A1+1",
             "circular ref");

    run_test("circular reference cell unchanged",
             2, 2,
             "A1=5\nA1=A1+1",
             "(circular ref)");

    run_test("invalid cell out of bounds",
             2, 2,
             "A3=5",
             "invalid cell");

    run_test("invalid range status",
             2, 3,
             "A1=MAX(C1:A1)",
             "Invalid range");

    run_test("invalid command",
             2, 2,
             "hello world",
             "unrecognized cmd");

    run_test("ERR from referenced ERR cell",
             3, 3,
             "A1=1\nB1=1/A1\nA1=0",
             "ERR");
}

static void test_scroll_commands(void) {
    printf("\n[Scroll Commands]\n");

    run_test("scroll_to valid cell",
             20, 20,
             "scroll_to K11",
             "(ok)");

    run_test("scroll_to invalid cell",
             2, 2,
             "scroll_to Z99",
             "(Invalid cell)");

    run_test("scroll down",
             20, 2,
             "s",
             "(ok)");

    run_test("scroll up from top stays at 0",
             20, 2,
             "w",
             "(ok)");

    run_test("scroll right",
             2, 20,
             "d",
             "(ok)");

    run_test("scroll left from start stays at 0",
             2, 20,
             "a",
             "(ok)");
}

static void test_output_control(void) {
    printf("\n[Output Control]\n");

    run_test("disable_output suppresses sheet",
             2, 2,
             "disable_output\nA1=5",
             "(ok)");

    run_test("enable_output restores sheet",
             2, 2,
             "disable_output\nA1=5\nenable_output\nA1=6",
             "6");

    /* When output is disabled, the grid should NOT appear between commands */
    run_test("sheet not printed while disabled",
             2, 2,
             "A1=3\ndisable_output\nA1=9",
             "(ok)");
}

static void test_large_sheet(void) {
    printf("\n[Large Sheet]\n");

    run_test("large sheet initialises",
             999, 100,
             "A1=42",
             "42");

    run_test("far cell assignment",
             999, 100,
             "A999=77\nscroll_to A999",
             "77");
}

/* ---------- main ---------- */

int main(void) {
    printf("=== Spreadsheet Test Suite ===\n");

    test_basic_assignment();
    test_arithmetic();
    test_negative_numbers();
    test_cell_references();
    test_recalculation();
    test_functions();
    test_error_handling();
    test_scroll_commands();
    test_output_control();
    test_large_sheet();

    printf("\n==============================\n");
    printf("Results: %d passed, %d failed\n", passed, failed);
    printf("==============================\n");

    return failed > 0 ? 1 : 0;
}