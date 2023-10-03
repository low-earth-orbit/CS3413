#include <pthread.h> // for pthread_create(), pthread_join(), etc.
#include <stdio.h>   // for scanf(), printf(), etc.
#include <stdlib.h>  // for malloc()
#include <unistd.h>  // for sleep()

#define NUMBER_OF_BOXES_PER_DWELLER 5
#define ROOM_IN_TRUCK 10
#define MIN_BOX_WEIGHT 5
#define MAX_BOX_WEIGHT 50
#define MAX_TIME_FOR_HOUSE_DWELLER 7
#define MIN_TIME_FOR_HOUSE_DWELLER 1
#define MAX_TIME_FOR_MOVER 3
#define MIN_TIME_FOR_MOVER 1
#define MIN_TIME_FOR_TRUCKER 1
#define MAX_TIME_FOR_TRUCKER 3
#define MIN_TRIP_TIME 5
#define MAX_TRIP_TIME 10
#define RANDOM_WITHIN_RANGE(a, b, seed) (a + rand_r(&seed) % (b - a))
// For pipes
#define READ_END 0
#define WRITE_END 1

// Pipe between house dwellers and movers
int houseFloor[2];
// Pipe between movers and truckers
int nextToTrucks[2];

// some function definitions follow
void *run(void *param)
{
    printf("Hello from run\n");
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    // rest of the program follows
    printf("In main (1)\n");

    // Prompt user for input
    int num_people, num_movers, num_drivers;
    printf("Please input number of people living in the house, number of movers and number of truck drivers\n");
    scanf("%d %d %d", &num_people, &num_movers, &num_drivers);
    if (num_people < 0 || num_movers < 0 || num_drivers < 0)
    {
        printf("Invalid input! All numbers must be non-negative.\n");
        return 1;
    }

    printf("Number of people living in the house: %d\n", num_people);
    printf("Number of movers: %d\n", num_movers);
    printf("Number of truck drivers: %d\n", num_drivers);

    // create 3 threads
    pthread_t threads[3];
    for (int i = 0; i < 3; i++)
    {
        pthread_create(&threads[i], NULL, &run, NULL);
    }

    // join all threads
    for (int i = 0; i < 3; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("In main (2)\n");
    return 0;
}