#include <pthread.h> // for pthread_create(), pthread_join(), etc.
#include <stdio.h>   // for scanf(), printf(), etc.
#include <stdlib.h>  // for malloc()
#include <unistd.h>  // for sleep()
#include <semaphore.h>

#define NUMBER_OF_BOXES_PER_DWELLER 5
#define ROOM_IN_TRUCK 10
#define MIN_BOX_WEIGHT 5
#define MAX_BOX_WEIGHT 50
#define MAX_TIME_FOR_HOUSE_DWELLER 7
#define MIN_TIME_FOR_HOUSE_DWELLER 1
#define MAX_TIME_FOR_MOVER 3
#define MIN_TIME_FOR_MOVER 1
#define MIN_TIME_FOR_TRUCKER 1
#define MAX_TIME_FOR_TRUCKER 3f
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

// mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// semaphore
sem_t sem;

// active thread counter
int active_dwellers;
int active_movers;
int active_drivers;

// some function definitions follow
void *dweller_run(void *param)
{
    RunnerParam *p = (RunnerParam *)param;
    printf("Hello from house dweller %d\n", p->id);

    // Close read end of houseFloor pipe
    close(houseFloor[READ_END]);

    int interval;
    int weight;
    int status;

    for (int i = 0; i < NUMBER_OF_BOXES_PER_DWELLER; i++)
    {

        interval = RANDOM_WITHIN_RANGE(MIN_TIME_FOR_HOUSE_DWELLER, MAX_TIME_FOR_HOUSE_DWELLER, (p->seed));
        weight = RANDOM_WITHIN_RANGE(MIN_BOX_WEIGHT, MAX_BOX_WEIGHT, (p->seed));
        sleep(interval);

        printf("House dweller %d created a box that weights %d in %d units of time\n", p->id, weight, interval);

        // lock mutex before writing
        pthread_mutex_lock(&mutex);

        // write to houseFloor pipe
        status = write(houseFloor[WRITE_END], &weight, sizeof(weight));
        if (status == -1)
        {
            printf("Write to houseFloor pipe failed\n");
        }
        printf("%d data wrote thru pipe: %d\n", status, weight);

        // unlock mutex
        pthread_mutex_unlock(&mutex);

        sem_post(&sem); // signal a box has been placed
    }

    pthread_mutex_lock(&mutex);
    active_dwellers--;
    printf("active_dwellers = %d \n", active_dwellers);
    if (active_dwellers == 0)
    {
        close(houseFloor[WRITE_END]);
    }
    pthread_mutex_unlock(&mutex);

    printf("House dweller %d is done packing\n", p->id);
    pthread_exit(0);
}

void *mover_run(void *param)
{
    // close write end of houseFloor pipe
    close(houseFloor[WRITE_END]);

    RunnerParam *p = (RunnerParam *)param;
    printf("Hello from mover %d\n", p->id);

    int weight;
    int interval;
    int status;

    while (1)
    {
        sem_wait(&sem); // Wait until a box is placed

        // lock mutex before reading from the pipe
        pthread_mutex_lock(&mutex);

        // read data
        status = read(houseFloor[READ_END], &weight, sizeof(weight));
        if (status == -1)
        {
            printf("Read from houseFloor pipe failed\n");
        }

        // unlock mutex
        pthread_mutex_unlock(&mutex);

        printf("data read from pipe: %d\n", weight);

        printf("Mover %d read %d bytes from the pipe: %d\n", p->id, status, weight);

        if (status > 0)
        {
            interval = RANDOM_WITHIN_RANGE(MIN_TIME_FOR_MOVER, MAX_TIME_FOR_MOVER, (p->seed));
            sleep(interval);
            printf("Mover %d brought down a box that weights %d in %d units of time\n", p->id, weight, interval);
        }
        else
        {
            break;
        }
    }
    pthread_mutex_lock(&mutex);
    active_movers--; // decrement active thread counter
    printf("active_movers = %d \n", active_movers);
    if (active_movers == 0)
    {
        close(houseFloor[READ_END]); // close read end of the pipe when the last active thread is about to terminate
    }
    pthread_mutex_unlock(&mutex);
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

    // Initialize semaphore
    sem_init(&sem, 0, 0);

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

    // IDs
    int dweller_ids[num_dwellers];
    int mover_ids[num_movers];
    int driver_ids[num_drivers];

    // thread parameters
    RunnerParam dweller_params[num_dwellers];
    RunnerParam mover_params[num_movers];
    RunnerParam driver_params[num_drivers];

    // active thread counter
    active_dwellers = num_dwellers;
    active_movers = num_movers;
    active_drivers = num_drivers;

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
        pthread_create(&driver_threads[i], NULL, driver_run, &driver_params[i]);
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

    // Cleanup
    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem); // Destroy semaphore
    close(houseFloor[READ_END]);
    close(houseFloor[WRITE_END]);
    close(nextToTrucks[READ_END]);
    close(nextToTrucks[WRITE_END]);

    return 0;
}