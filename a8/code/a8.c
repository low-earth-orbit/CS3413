#include <stdio.h>
#include <stdlib.h>

char currentDirection;
int nextPosition;
int start = 0;

void fcfs()
{
    int movement = 0;
    double timeIn = 0; // This is job's arrival time
    double time = 0;   // This is total time
    int currentPosition = start;

    while (EOF != (scanf("%i %lf\n", &nextPosition, &timeIn)))
    {
        int distance = abs(nextPosition - currentPosition);
        movement += distance;

        // Direction reversed
        if ((nextPosition - currentPosition) < 0 && currentDirection == 'a')
        {
            if (timeIn > time)
            {
                time = timeIn + distance / 5.0 + 15;
            }
            else
            {
                time = time + distance / 5.0 + 15;
            }
            currentDirection = 'd';
        }
        else if ((nextPosition - currentPosition) > 0 && currentDirection == 'd')
        {
            if (timeIn > time)
            {
                time = timeIn + distance / 5.0 + 15;
            }
            else
            {
                time = time + distance / 5.0 + 15;
            }
            currentDirection = 'a';
        }
        else
        {
            if (timeIn > time)
            {
                time = timeIn + distance / 5.0;
            }
            else
            {
                time = time + distance / 5.0;
            }
        }

        // Update currentPosition
        currentPosition = nextPosition;
    }

    printf("Movement:%i Time:%.1lf\n", movement, time);
}

void cscan(int start)
{
    int movement = 0;
    double time = 0;
    printf("Movement: %i Time:%.1lf\n", movement, time);
}

int main(int argc, char **argv)
{

    char algorithm = argv[1][0];
    int start = 0;
    currentDirection = 'a';

    if (algorithm == 'F')
    {
        fcfs(start);
    }
    else if (algorithm == 'C')
    {
        cscan(start);
    }

    return 0;
}