#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define MAXN 5
int **generate_square_matrix(int size)
{
    int **array = malloc(sizeof(int *) * size);
    for (int i = 0; i < size; i++)
    {
        array[i] = malloc(sizeof(int) * size);
        memset(array[i], 0, sizeof(int) * size);
    }
    return array;
}
int **generate_square_matrix_and_fill_it(int size)
{
    int **array = generate_square_matrix(size);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            array[i][j] = rand() % MAXN;
        }
    }
    return array;
}
void print_square_matrix(int **array, int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("%i ", array[i][j]);
        }
        printf("\n");
    }
}
int check_if_matrices_differ(int **array, int **array2, int size)
{
    int result = 0;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            result += array[i][j] - array2[i][j];
            if (result != 0)
            {
                return result;
            }
        }
    }
    return result;
}
typedef struct _params
{
    int **first_array;
    int **second_array;
    int **result;
    int max_threads;
    int row_index;
    int size;
} ThreadParams;

void multiply_matrices(void *threadParams)
{
    ThreadParams *t = (ThreadParams *)threadParams;
    int N = t->size;
    int row = t->row_index;
    int column = 0;
    int temp_result = 0;
    while (row < N)
    {
        column = 0;
        while (column < N)
        {
            temp_result = 0;
            for (int i = 0; i < N; i++)
            {
                temp_result = temp_result + t->first_array[row][i] * t->second_array[i][column];
            }
            t->result[row][column] = temp_result;
            column = column + 1;
        }
        row = row + 1;
    }
}
void *multiply_matrices_threaded(void *threadParams)
{
    /**
     * write a code for matrix multiplication that will utilize the
     * threading capacity and parallelize the computation in such a
     * way that a thread computes result per one or more rows
     */
    ThreadParams *t = (ThreadParams *)threadParams;
    int N = t->size;
    int row = t->row_index;
    int column = 0;
    int temp_result = 0;
    int offset = t->max_threads;
    while (row < N)
    {
        column = 0;
        while (column < N)
        {
            temp_result = 0;
            for (int i = 0; i < N; i++)
            {
                temp_result = temp_result + t->first_array[row][i] * t->second_array[i][column];
            }
            t->result[row][column] = temp_result;
            column = column + 1;
        }
        row = row + offset;
    }

    // terminate the thread
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Please provide size of the matrix and the number of threads to execute\n");
        exit(0);
    }
    int size = atoi(argv[1]);
    int max_threads = atoi(argv[2]);

    // The value you pass to srand determines the random sequence
    srand(time(NULL)); // Line to initialize the random number generator.
    int **array1 = generate_square_matrix_and_fill_it(size);
    int **array2 = generate_square_matrix_and_fill_it(size);
    int **result = generate_square_matrix(size); // generate an empty matrix
    struct timeval begin;
    struct timeval end;
    gettimeofday(&begin, NULL);                                       // fills the contents with time since the beginning of epoch
    ThreadParams *thr = (ThreadParams *)malloc(sizeof(ThreadParams)); // allocate a structure for holding function parameters
    thr->first_array = array1;                                        // first matrix to multiply
    thr->second_array = array2;                                       // the second matrix to multiply
    thr->result = result;                                             // where to store the results - note it needs to be generated
    thr->row_index = 0;                                               // this variable, in combination with max_threads can be used for parallelization
    thr->size = size;
    thr->max_threads = max_threads;

    multiply_matrices((void *)thr);

    gettimeofday(&end, NULL); // fills the contents with time since the beginning of epoch
    // The next line is inspired by https://linuxhint.com/gettimeofday_c_language/
    long long microseconds = (end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec);
    double duration = (1.0 * microseconds) / 1000000;
    printf("Single threaded took %lf seconds to execute \n", duration);
    int **threaded_result = generate_square_matrix(size);
    gettimeofday(&begin, NULL);
    /**
     * Write your code to create and use max_threads here, such that the threaded_result
     * is populated with the result of the computation.
     */
    // create threads
    int num_threads = min(max_threads, size);
    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; i++)
    {
        ThreadParams *new_thr = (ThreadParams *)malloc(sizeof(ThreadParams)); // allocate a structure for holding function parameters
        new_thr->first_array = array1;                                        // first matrix to multiply
        new_thr->second_array = array2;                                       // the second matrix to multiply
        new_thr->result = threaded_result;                                    // where to store the results - note it needs to be generated
        new_thr->row_index = i;                                               // this variable, in combination with max_threads can be used for parallelization
        new_thr->size = size;
        new_thr->max_threads = max_threads;

        pthread_create(&threads[i], NULL, multiply_matrices_threaded, (void *)new_thr);
    }

    gettimeofday(&end, NULL);

    // join all threads
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }
    // The next line is inspired by https://linuxhint.com/gettimeofday_c_language/
    microseconds = (end.tv_sec * 1000000 + end.tv_usec) - (begin.tv_sec * 1000000 + begin.tv_usec);
    duration = (1.0 * microseconds) / 1000000;
    printf("Multi-threaded took %lf seconds to execute \n", duration);

    if (check_if_matrices_differ(result, threaded_result, size) != 0)
    {
        printf("Threaded result differ from single core computation, error\n");
        exit(0);
    }

    return 0;
}

// Run the program with the problem size of 1000 and 10 threads, what is the approximate
// speedup you are achieving?
// -- Single threaded took 3.299390 seconds to execute
// -- Multi-threaded took 0.000202 seconds to execute

// Is there a problem size / number of threads combination that slows down the computation
// process? Why do you think it is happening?
// -- I found for single thread increasing to a few hundreds matrix size, e.g. 600, 800, 1000 obviously
// slows down the computation. For multi-thread, for the numbers I tried (up to 2000), there is no obvious wait time difference,
// as long as the threads is more than a few.
// This is because the number of computation increases per matrix size.

// What is the minimum size of the problem that benefits from creating an extra thread?
// -- Size of 60 and 10 threads is about the break-even point on my machine.

// Does using the threads always improve execution duration?
// -- No. For smaller size of array, multi-threading does not always improve execution duration.

// Guesstimate and comment on the nature of growth of the speedup with the number of threads
// – is it linear, exponential, are there any limits?
// -- I guess it is exponential, then reaches a limit when the multi-threading is maximized.