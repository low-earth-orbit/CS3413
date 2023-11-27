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
    int differentTime = 0;
    while (EOF != (scanf("%i %lf\n", &positionIn, &timeIn)))
    {
        printf("positionIn: %i timeIn:%.1lf\n", positionIn, timeIn);
        printf("Before: Movement: %i Time:%.1lf\n", movementCount, timeCount);

        if (timeCount < timeIn)
        {
            timeCount = timeIn;

            // Check if wrap-around is needed
            if (lastProcessedIndex != -1 && lastProcessedIndex == (requestCount - 1))
            {
                printf("!!!!!!!!!!inside wrap-around block\n");
                currentPosition = start; // Reset to start
                lastProcessedIndex = -1; // Reset last processed index
                // No movement count added for wrap-around, time for redirection only
                timeCount += 15.0;

                // Wrapped around occurred, start from 0
                for (int i = 0; i < requestCount; i++)
                {
                    if (requests[i].sector != -1)
                    {
                        int distance = abs(requests[i].sector - currentPosition);

                        movementCount += distance;
                        printf("Movement count inside: %d\n", movementCount);

                        currentPosition = requests[i].sector;
                        // Update timeCount, assuming continuous processing
                        timeCount += distance / 5.0;
                        requests[i].sector = -1;
                    }
                }
            }
        }

        addRequest(positionIn, timeIn);
        printf("requestCount = %d\n", requestCount);

        // Sort requests by sector
        qsort(requests, requestCount, sizeof(Request), compare);

        // Process the requests
        for (int i = 0; i < requestCount; i++)
        {
            printf("currentPosition = %d requests[i].sector = %d\n", currentPosition, requests[i].sector);
            if (requests[i].sector != -1 && requests[i].sector >= currentPosition)
            {
                int distance = abs(requests[i].sector - currentPosition);
                movementCount += distance;
                currentPosition = requests[i].sector;
                lastProcessedIndex = i;
                timeCount = timeCount + distance / 5.0;
                requests[i].sector = -1;
            }
        }

        printf("requestCount = %i\n", requestCount);
        printf("lastProcessedIndex = %i \n", lastProcessedIndex);
        printf("After: Movement: %i Time:%.1lf\n", movementCount, timeCount);
    }

    int finalWrapAround = 0;
    for (int i = 0; i < requestCount; i++)
    {
        if (requests[i].sector != -1)
        {
            printf("requests[i].sector in wrap around check = %d\n", requests[i].sector);
            finalWrapAround = 1;
            break;
        }
    }

    // Do an extra loop
    if (finalWrapAround == 1)
    {
        timeCount += 15.0;
        currentPosition = start; // Reset to start
        for (int i = 0; i < requestCount; i++)
        {
            if (requests[i].sector != -1)
            {
                printf("requests[i].sector = %d\n", requests[i].sector);

                finalWrapAround = 1;
                int distance = abs(requests[i].sector - currentPosition);

                movementCount += distance;
                printf("Movement count inside: %d\n", movementCount);

                currentPosition = requests[i].sector;
                timeCount += distance / 5.0;
                requests[i].sector = -1;
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