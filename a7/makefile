SDIR=./code
INDIR=./in
OUTDIR=./out
STDOUTDIR=./student_out

_OBJ=./a.out

CC=gcc
CFLAGS= -O0 -g -lm -std=c99
DIFF=diff

all:
	$(CC) $(CFLAGS) $(SDIR)/*.c -o $(_OBJ)
1:
	./a.out 3 < ./in/algo-check.in > ./student_out/sample_output.out
	diff ./student_out/sample_output.out ./out/sample_output.out
2:
	./a.out 3 < ./in/algo-check-2.in > ./student_out/sample_output_2.out 
	diff ./student_out/sample_output_2.out   ./out/sample_output_2.out 
3:
	./a.out 1 < ./in/discard-test.in > ./student_out/discard-test-1.out
	diff ./student_out/discard-test-1.out ./out/discard-test-1.out
4:
	./a.out 2 < ./in/discard-test.in > ./student_out/discard-test-2.out
	diff ./student_out/discard-test-2.out ./out/discard-test-2.out
5:
	./a.out 1 < ./in/major-test.in > ./student_out/major-test-1.out
	diff ./student_out/major-test-1.out ./out/major-test-1.out
6:
	./a.out 2 < ./in/major-test.in > ./student_out/major-test-2.out
	diff ./student_out/major-test-2.out ./out/major-test-2.out
7:
	./a.out 5 < ./in/page-hit-test.in > ./student_out/page-hit-test.out
	diff ./student_out/page-hit-test.out ./out/page-hit-test.out
8:
	./a.out 3 < ./in/algo-check-o.in > ./student_out/algo-check-o-l.out
	diff ./student_out/algo-check-o-l.out ./out/algo-check-o-l.out
9:
	./a.out 3 < ./in/fromText.in > ./student_out/fromText.out
	diff ./student_out/fromText.out ./out/fromText.out
