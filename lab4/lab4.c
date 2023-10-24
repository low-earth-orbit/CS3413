#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define RANDOM_WITHIN_RANGE(a, b, seed) (rand_r(&seed) % b + a)
#include <semaphore.h>

sem_t sem;
pthread_mutex_t mutex;
int gv = 1;

void *inc()
{
    for (int i = 0; i < 10000; i++)
    {
        pthread_mutex_lock(&mutex);
        gv++; // critial block locked by mutex
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

void *dec()
{
    for (int i = 0; i < 10000; i++)
    {
        pthread_mutex_lock(&mutex);
        gv--; // critial block locked by mutex
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void *minus()
{
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        sem_wait(&sem); // waiting for + to be printed before continuing
        printf("-");    // print -
    }
    return NULL;
}

void *plus(void *argg)
{
    unsigned int seed = *((unsigned int *)argg);
    int interval = RANDOM_WITHIN_RANGE(100000, 500000, seed);
    int i = 0;

    for (i = 0; i < 10; i++)
    {
        printf("+");      // print +
        sem_post(&sem);   // Signal that + has been printed
        usleep(interval); // sleep - so this will leave enough time for minus() to run
        // relying on usleep() for synchronization is incorrect
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_mutex_init(&mutex, NULL);

    setvbuf(stdout, NULL, _IONBF, 0);
    int i = 0;

    pthread_t threads[4];

    // Create two threads
    pthread_create(&(threads[0]), NULL, &inc, NULL);
    pthread_create(&(threads[1]), NULL, &dec, NULL);

    // Join threads
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    printf("gv = %d\n", gv);

    pthread_mutex_destroy(&mutex);

    sem_init(&sem, 0, 0);

    unsigned int *t = (unsigned int *)malloc(sizeof(unsigned int));
    *t = rand();

    pthread_create(&(threads[3]), NULL, &plus, (void *)t);
    pthread_create(&(threads[4]), NULL, &minus, NULL);

    // Join threads
    pthread_join(threads[3], NULL);
    pthread_join(threads[4], NULL);

    // free
    free(t);

    return 0;
}

/* comments

2: Yes. For small number of operations data race has not shown up in the result.

3: This time the value is not always one. Two threads are modifying the value of gv at the same time.

6: This time the value is not always one. Two threads are modifying the value of gv at the same time.

8: Yes. The result is always 1. The use of mutex ensures that gv is written by one thread at a time.

13: After I comment out usleep() statement, the program doesn't work as before anymore. This is because usleep creates a delays that helped synchronization;
however, it is not the correct method. After I change the maximum number of iterations of the plus loop to 3, it prints only +-+-+- because the minus is still
waiting for the semaphore to be posted by the plus function. I think the safe way is to use two semaphores, instead of relying on usleep() in plus().
*/