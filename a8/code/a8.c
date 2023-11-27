#include <stdio.h>
#include <stdlib.h>
#define MAX_REQUESTS 1000
char currentDirection;
int positionIn;
int start = 0;

typedef struct
{
    int sector;
    double arrivalTime;
} Request;

Request requests[MAX_REQUESTS];
int requestCount = 0;

void addRequest(int sector, double arrivalTime)
{
    if (requestCount < MAX_REQUESTS)
    {
        requests[requestCount].sector = sector;
        requests[requestCount].arrivalTime = arrivalTime;
        requestCount++;
    }
}

int compare(const void *a, const void *b)
{
    Request *requestA = (Request *)a;
    Request *requestB = (Request *)b;
    return requestA->sector - requestB->sector;
}

void fcfs()
{
    int movement = 0;
    double timeIn = 0; // This is job's arrival time
    double time = 0;   // This is total time
    int currentPosition = start;

    while (EOF != (scanf("%i %lf\n", &positionIn, &timeIn)))
    {
        int distance = abs(positionIn - currentPosition);
        movement += distance;

        // Direction reversed
        if ((positionIn - currentPosition) < 0 && currentDirection == 'a')
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
        else if ((positionIn - currentPosition) > 0 && currentDirection == 'd')
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
        currentPosition = positionIn;
    }

    printf("Movement:%i Time:%.1lf\n", movement, time);
}

void cscan()
{
    int movementCount = 0;
    double timeIn = 0;    // This is job's arrival time
    double timeCount = 0; // This is total time
    int currentPosition = start;
    int lastProcessedIndex = -1;

    while (EOF != (scanf("%i %lf\n", &positionIn, &timeIn)))
    {
        printf("positionIn: %i timeIn:%.1lf\n", positionIn, timeIn);
        printf("Movement: %i Time:%.1lf\n", movementCount, timeCount);

        if (timeCount < timeIn && requestCount > 0)
        {
            // Go back if the last request in queue
            if (lastProcessedIndex == requestCount - 1)
            {
                currentPosition = start; // To start
                int distance = abs(requests[lastProcessedIndex].sector - currentPosition);
                movementCount += distance;
                // Update timeCount, additional time to redirection

                timeCount = timeCount + distance / 5.0 + 15.0;

                lastProcessedIndex = -1;
            }
        }

        addRequest(positionIn, timeIn);
        // Sort requests by sector
        qsort(requests, requestCount, sizeof(Request), compare);

        // Process the requests
        for (int i = 0; i < requestCount; i++)
        {
            if (requests[i].sector != -1 && requests[i].sector >= currentPosition && i > lastProcessedIndex)
            {
                int distance = abs(requests[i].sector - currentPosition);
                movementCount += distance;
                currentPosition = requests[i].sector;
                lastProcessedIndex = i;
                // Update timeCount
                if (timeIn > timeCount)
                {
                    timeCount = timeIn + distance / 5.0;
                }
                else
                {
                    timeCount = timeCount + distance / 5.0;
                }
            }
        }
    }

    printf("Movement: %i Time:%.1lf\n", movementCount, timeCount);
}

int main(int argc, char **argv)
{
    char algorithm = argv[1][0];
    currentDirection = 'a';

    if (algorithm == 'F')
    {
        fcfs();
    }
    else if (algorithm == 'C')
    {
        cscan();
    }

    return 0;
}