CC = gcc
CFLAGS = -Wall -Wextra -g

SRCS = main.c utils.c sheet.c parser.c commands.c evaluator.c deps.c
TARGET = sheet

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

test: $(TARGET)
	gcc $(CFLAGS) tests.c -o run_tests && ./run_tests

report:
	pdflatex report.tex
	pdflatex report.tex

clean:
	rm -f $(TARGET) run_tests

.PHONY: all test report clean