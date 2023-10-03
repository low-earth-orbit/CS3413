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
} DwellerParam;

// some function definitions follow
void *dweller_run(void *param)
{
    DwellerParam *p = (DwellerParam *)param;
    printf("Hello from house dweller %d\n", p->id);

    for (int i = 0; i < NUMBER_OF_BOXES_PER_DWELLER; i++)
    {
        int interval = RANDOM_WITHIN_RANGE(MIN_TIME_FOR_HOUSE_DWELLER, MAX_TIME_FOR_HOUSE_DWELLER, (p->seed));
        int weight = RANDOM_WITHIN_RANGE(MIN_BOX_WEIGHT, MAX_BOX_WEIGHT, (p->seed));
        sleep(interval);

        printf("House dweller %d created a box that weights %d in %d units of time\n", p->id, weight, interval);
    }
    printf("House dweller %d is done packing\n", p->id);
    pthread_exit(0);
}

void *mover_run(void *param)
{
    int i = *((int *)param);
    printf("Hello from mover %d\n", i);
    pthread_exit(0);
}

void *driver_run(void *param)
{
    int i = *((int *)param);
    printf("Hello from truck driver %d\n", i);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    // rest of the program follows
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

    // IDs
    int dweller_ids[num_dwellers];
    int mover_ids[num_movers];
    int driver_ids[num_drivers];

    // thread parameters
    DwellerParam dweller_params[num_dwellers];

    // create dweller threads
    for (int i = 0; i < num_dwellers; i++)
    {
        dweller_params[i].id = i;
        dweller_params[i].seed = rand();
        pthread_create(&dweller_threads[i], NULL, dweller_run, &dweller_params[i]);
    }

    // do the same for mover
    for (int i = 0; i < num_movers; i++)
    {
        mover_ids[i] = i;
        pthread_create(&mover_threads[i], NULL, mover_run, &mover_ids[i]);
    }

    // do the same for driver
    for (int i = 0; i < num_drivers; i++)
    {
        driver_ids[i] = i;
        pthread_create(&driver_threads[i], NULL, driver_run, &driver_ids[i]);
    }

    // join all threads
    for (int i = 0; i < num_dwellers; i++)
    {
        pthread_join(dweller_threads[i], NULL);
    }

    for (int i = 0; i < num_movers; i++)
    {
        pthread_join(mover_threads[i], NULL);
    }

    for (int i = 0; i < num_drivers; i++)
    {
        pthread_join(driver_threads[i], NULL);
    }

    printf("In main (2)\n");
    return 0;
}