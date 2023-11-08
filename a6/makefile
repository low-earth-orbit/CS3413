.PHONY: all clean 1 2 3 4 5 6 7 8 9 10 11 12 13 14 

all: clean program1 1 2 3 4 5 6 7 8 9 10 11 12 13 14

program1:
	gcc -g -O0 ./code/*.c -o program1

1: 
	./program1 -s 10 -f < ./in/test1.in > ./student_out/test1-a.out && diff ./student_out/test1-a.out ./out/test1.out
2:
	./program1 -s 10 -f < ./in/test2.in > ./student_out/test2-a.out && diff ./student_out/test2-a.out ./out/test2.out
3:
	./program1 -s 10 -f < ./in/test3.in > ./student_out/test3-a.out && diff ./student_out/test3-a.out ./out/test3.out
4:
	./program1 -s 10 -f < ./in/test4.in > ./student_out/test4-a.out && diff ./student_out/test4-a.out ./out/test4.out
5:
	./program1 -s 10 -f < ./in/test5.in > ./student_out/test5-a.out && diff ./student_out/test5-a.out ./out/test5.out
6:
	./program1 -s 10 -f < ./in/test6.in > ./student_out/test6-a.out && diff ./student_out/test6-a.out ./out/test6.out
7:
	./program1 -s 5 -f < ./in/test7.in > ./student_out/test7-a.out && diff ./student_out/test7-a.out ./out/test7-a.out
8:
	./program1 -s 5 -b < ./in/test7.in > ./student_out/test7-b.out && diff ./student_out/test7-b.out ./out/test7-b.out
9:
	./program1 -s 5 -w < ./in/test7.in > ./student_out/test7-c.out && diff ./student_out/test7-c.out ./out/test7-c.out
10:
	./program1 -s 5 -f < ./in/test8.in > ./student_out/test8-a.out && diff ./student_out/test8-a.out ./out/test8-a.out
11:
	./program1 -s 5 -b < ./in/test8.in > ./student_out/test8-b.out && diff ./student_out/test8-b.out ./out/test8-b.out
12:
	./program1 -s 5 -w < ./in/test8.in > ./student_out/test8-c.out && diff ./student_out/test8-c.out ./out/test8-c.out
13:
	./program1 -s 5 -w < ./in/test9.in > ./student_out/test13.out && diff ./student_out/test13.out ./out/test13.out
14:
	./program1 -s 103 -f	 < ./in/test10.in > ./student_out/test14.out && diff ./student_out/test14.out ./out/test14.out



clean:
	rm -rf ./program1
	rm -rf student_out/*


