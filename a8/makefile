all:
	gcc -g -O0 ./code/*.c -lm 
1:
	./a.out F < in/all_known_in_advance_spaces.in > student_out/f-in-advance.out && diff  student_out/f-in-advance.out  out/f-in-advance.out 
2:
	./a.out C < in/all_known_in_advance_spaces.in > student_out/c-in-advance.out && diff student_out/c-in-advance.out out/c-in-advance.out 
3:
	./a.out F < in/time_dependent_spaces.in > student_out/f-time_dependent.out  && diff student_out/f-time_dependent.out  out/f-time_dependent.out  
4:
	./a.out C < in/time_dependent_spaces.in > student_out/c-time_dependent.out  && diff student_out/c-time_dependent.out  out/c-time_dependent.out  
5:
	./a.out F < in/time_dependent_second_spaces.in > student_out/f-time_dependent_second.out && diff student_out/f-time_dependent_second.out  out/f-time_dependent_second.out
6:
	./a.out C < in/time_dependent_second_spaces.in > student_out/c-time_dependent_second.out && diff student_out/c-time_dependent_second.out out/c-time_dependent_second.out  

