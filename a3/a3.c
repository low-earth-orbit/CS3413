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

// thread parameter
typedef struct
{
    int id;
    unsigned int seed;
} RunnerParam;

// some function definitions follow
void *dweller_run(void *param)
{
    RunnerParam *p = (RunnerParam *)param;
    printf("Hello from house dweller %d\n", p->id);

    int interval;
    int weight;
    int status;

    for (int i = 0; i < NUMBER_OF_BOXES_PER_DWELLER; i++)
    {
        interval = RANDOM_WITHIN_RANGE(MIN_TIME_FOR_HOUSE_DWELLER, MAX_TIME_FOR_HOUSE_DWELLER, (p->seed));
        weight = RANDOM_WITHIN_RANGE(MIN_BOX_WEIGHT, MAX_BOX_WEIGHT, (p->seed));

        sleep(interval);

        printf("House dweller %d created a box that weights %d in %d units of time\n", p->id, weight, interval);

        // write to houseFloor pipe
        status = write(houseFloor[WRITE_END], &weight, sizeof(weight));
        if (status == -1)
        {
            printf("Write to houseFloor pipe failed\n");
        }
    }

    printf("House dweller %d is done packing\n", p->id);

    pthread_exit(0);
}

void *mover_run(void *param)
{
    RunnerParam *p = (RunnerParam *)param;
    printf("Hello from mover %d\n", p->id);

    int weight;
    int interval;
    int status;

    status = read(houseFloor[READ_END], &weight, sizeof(weight));

    while (status > 0)
    { // keep reading until EOF

        interval = RANDOM_WITHIN_RANGE(MIN_TIME_FOR_MOVER, MAX_TIME_FOR_MOVER, (p->seed));

        sleep(interval);

        printf("Mover %d brought down a box that weights %d in %d units of time\n", p->id, weight, interval);

        // write to nextToTrucks pipe
        status = write(nextToTrucks[WRITE_END], &weight, sizeof(weight));
        if (status == -1)
        {
            printf("Write to nextToTrucks pipe failed\n");
        }

        status = read(houseFloor[READ_END], &weight, sizeof(weight));
    }
    // Finished moving

    printf("Mover %d is done moving boxes downstairs\n", p->id);

    pthread_exit(0);
}

void *driver_run(void *param)
{
    RunnerParam *p = (RunnerParam *)param;
    printf("Hello from truck driver %d\n", p->id);

    int weight;
    int load_time;
    int trip_time;
    int status;
    int room;
    int sum_weight;

    status = read(nextToTrucks[READ_END], &weight, sizeof(weight));

    while (status > 0) // tripping
    {
        room = ROOM_IN_TRUCK;
        sum_weight = 0;

        while (room > 0 && status > 0) // loading
        {

            load_time = RANDOM_WITHIN_RANGE(MIN_TIME_FOR_TRUCKER, MAX_TIME_FOR_TRUCKER, (p->seed));

            sleep(load_time);
            room--;

            sum_weight += weight;

            printf("Trucker %d  loaded up a box that weights %d to the truck, took %d units of time, room left:%d\n", (p->id), weight, load_time, room);
            status = read(nextToTrucks[READ_END], &weight, sizeof(weight));
        }

        trip_time = RANDOM_WITHIN_RANGE(MIN_TRIP_TIME, MAX_TRIP_TIME, (p->seed));
        // Check if the truck is fully loaded
        if (room == 0)
        {
            printf("Full truck %d with load of %d units of mass departed, round trip will take %d\n", p->id, sum_weight, trip_time);
        }
        else if (status <= 0)
        { // Check if there are no more boxes left to load
            printf("Not full truck %d with load of %d units of mass departed, one way trip will take %d\n", p->id, sum_weight, trip_time);
            break; // Exit the loop as there are no more boxes
        }

        sleep(trip_time);
    }

    printf("Trucker %d is finished\n", (p->id));

    pthread_exit(0);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    // rest of the program follows
    // Create pipes
    if (pipe(houseFloor) == -1 || pipe(nextToTrucks) == -1)
    {
        perror("Pipe failed\n");
        return 1;
    }

    // Prompt user for input
    int num_dwellers, num_movers, num_drivers;
    printf("Please input number of people living in the house, number of movers and number of truck drivers\n");
    scanf("%d %d %d", &num_dwellers, &num_movers, &num_drivers);
    if (num_dwellers < 0 || num_movers < 0 || num_drivers < 0)
    {
        printf("Invalid input\n");
        return 1;
    }

    // 3 types of threads: dweller, mover, driver
    pthread_t dweller_threads[num_dwellers];
    pthread_t mover_threads[num_movers];
    pthread_t driver_threads[num_drivers];

    // thread parameters
    RunnerParam dweller_params[num_dwellers];
    RunnerParam mover_params[num_movers];
    RunnerParam driver_params[num_drivers];

    // create dweller threads
    for (int i = 0; i < num_dwellers; i++)
    {
        dweller_params[i].id = i;
        dweller_params[i].seed = rand();
        if (pthread_create(&dweller_threads[i], NULL, dweller_run, &dweller_params[i]) != 0)
        {
            perror("Failed to create dweller thread\n");
            return 1;
        };
    }

    // do the same for mover
    for (int i = 0; i < num_movers; i++)
    {
        mover_params[i].id = i;
        mover_params[i].seed = rand();
        if (pthread_create(&mover_threads[i], NULL, mover_run, &mover_params[i]) != 0)
        {
            perror("Failed to create mover thread\n");
            return 1;
        }
    }

    // do the same for driver
    for (int i = 0; i < num_drivers; i++)
    {
        driver_params[i].id = i;
        driver_params[i].seed = rand();
        if (pthread_create(&driver_threads[i], NULL, driver_run, &driver_params[i]) != 0)
        {
            perror("Failed to create driver thread\n");
            return 1;
        }
    }

    // join all threads
    for (int i = 0; i < num_dwellers; i++)
    {
        pthread_join(dweller_threads[i], NULL);
    }
    close(houseFloor[WRITE_END]);

    for (int i = 0; i < num_movers; i++)
    {
        pthread_join(mover_threads[i], NULL);
    }
    close(houseFloor[READ_END]);
    close(nextToTrucks[WRITE_END]);

    for (int i = 0; i < num_drivers; i++)
    {
        pthread_join(driver_threads[i], NULL);
    }
    close(nextToTrucks[READ_END]);

    // print final message
    printf("Moving is finished!\n");

    return 0;
}