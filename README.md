# CSpreadsheet

A terminal-based spreadsheet program written in C, built for ACOP290 at IITD-AD.

**Team**
- Saket Sharma — `24A1CSEB0019`
- Aditya Mohan — `24A1CSEB0003`

---

## Getting Started

```bash
make          # compile → produces ./sheet
./sheet R C   # run with R rows and C columns
make test     # run the test suite
make report   # build report.pdf from report.tex (requires pdflatex)
make clean    # remove build artifacts
```

Grid size: 1 ≤ R ≤ 999, 1 ≤ C ≤ 18278 (columns A through ZZZ).  
All cells hold integers and start at 0. At most a 10×10 window is shown at any time.

---

## Usage

### Formula assignment

```
A1=5
B1=A1+1
C1=SUM(A1:B1)
```

### Supported expressions

| Type | Examples |
|---|---|
| Constants | `A1=42` |
| Arithmetic | `A1=B1+C1`, `A1=10/3`, `A1=(B1+2)*C1` |
| Negative numbers | `A1=-5`, `A1=-B1` |
| Cell references | `B1=A1+1` |
| SUM | `A1=SUM(A1:D4)` |
| AVG | `A1=AVG(A1:A10)` |
| MIN | `A1=MIN(B1:B5)` |
| MAX | `A1=MAX(A1:Z1)` |
| STDEV | `A1=STDEV(A1:C1)` |
| SLEEP(n) | `A1=SLEEP(3)` — sleeps 3 seconds, returns 3 |

Ranges can be 1D (`A1:A10`, `A1:F1`) or 2D (`A1:D4`). Both ends are inclusive.

### Control commands

| Input | Action |
|---|---|
| `w` | scroll up 10 rows |
| `s` | scroll down 10 rows |
| `a` | scroll left 10 columns |
| `d` | scroll right 10 columns |
| `scroll_to A5` | jump viewport to cell A5 |
| `disable_output` | suppress sheet printing |
| `enable_output` | restore sheet printing |
| `q` | quit |

---

## Error Handling

| Situation | Status shown |
|---|---|
| Valid command | `(ok)` |
| Division by zero | `(div by 0)` — affected cells show `ERR` |
| Circular reference | `(circular ref)` — assignment rejected |
| Invalid cell | `(invalid cell)` |
| Invalid range | `(Invalid range)` |
| Unknown command | `(unrecognized cmd)` |

ERR propagates: if a cell is ERR, every cell that depends on it also becomes ERR.

---

## Automatic Recalculation

When a cell's value changes, only the cells that depend on it (directly or transitively) are recalculated, in topological order. Unrelated cells — including those with `SLEEP` formulas — are not touched.

---

## Project Structure

```
main.c        — REPL loop and orchestration
parser.c/h    — input classification (formula / command / invalid)
sheet.c/h     — 2D cell grid, get/set/print
evaluator.c/h — expression evaluation, dependency extraction
deps.c/h      — dependency graph, cycle detection, recalc order
commands.c/h  — scroll and output control commands
utils.c/h     — cell name ↔ index conversion, range parsing
tests.c       — automated test suite (run with make test)
Makefile      — build, test, and report targets
report.tex    — LaTeX design document
```