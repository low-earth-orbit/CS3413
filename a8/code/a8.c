#include <stdio.h>
char currentDirection;

void fcfs(int start){
    int movement = 0;
    double time = 0;
    printf("Movement: %i Time:%.1lf\n", movement, time);
}

void cscan(int start){
    int movement = 0;
    double time = 0;
    printf("Movement: %i Time:%.1lf\n", movement, time);
}

int main (int argc, char** argv){
    int position, time;
    char algorithm = argv[1][0];
    int start = 0;
    currentDirection  = 'a';


    while ( EOF!=(scanf("%i %i\n",&position,&time)))
    {
        printf("Delete me: position %i, Delete me: time %i\n",position,time);
    }
    if (algorithm == 'F'){
        fcfs(start);
    }else if ( algorithm == 'C'){
        cscan(start);
    }

    return 0;
}