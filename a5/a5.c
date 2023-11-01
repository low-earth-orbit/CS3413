#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct _node
{
    int arrivalTime;
    int duration;
    int last_job;
    int affinity;
    char processName;
    char *userName;
    struct _node *next;
} Job;

typedef struct cpu
{
    Job *current;
    int time;
} CPU;

// CPU thread parameter
typedef struct
{
    CPU *cpu;
    Job **queue;
    Job *summary;
    int quantum;
    int id;
} CpuThreadParam;

// Mutex for input queue
pthread_mutex_t queue_mutex;

typedef struct _buffer
{
    Job *job;
    int cpu_id;
    int time;
} Buffer;

Buffer printBuffer[1000];
int buffer_index = 0;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
int global_time = 1;
int finished_cpu_count = 0;
pthread_mutex_t jobs_done_mutex;

sem_t print_sem;
sem_t cpu_sem;

sem_t all_cpus_done; // Signal from all CPUs to print
sem_t print_done;    // Signal from print to all CPUs

void enqueue(Job **queue, Job *job)
{
    if (*queue == NULL)
    {
        *queue = (Job *)malloc(sizeof(Job));
        (*queue)->next = NULL;
        (*queue)->arrivalTime = job->arrivalTime;
        (*queue)->duration = job->duration;
        (*queue)->affinity = job->affinity;
        (*queue)->last_job = job->last_job;
        (*queue)->processName = job->processName;
        (*queue)->userName = strdup(job->userName);
        return;
    }
    Job *temp = *queue;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = (Job *)malloc(sizeof(Job));
    temp->next->next = NULL;
    temp->next->arrivalTime = job->arrivalTime;
    temp->next->duration = job->duration;
    temp->next->affinity = job->affinity;
    temp->next->last_job = job->last_job;
    temp->next->processName = job->processName;
    temp->next->userName = strdup(job->userName);
}

void removeJob(Job **queue, Job *job)
{
    if (*queue == NULL)
    {
        return;
    }
    if (*queue == job)
    {
        Job *toFree = *queue;
        *queue = (*queue)->next;
        free(toFree->userName);
        free(toFree);
        return;
    }
    Job *temp = *queue;
    while (temp->next != job)
    {
        temp = temp->next;
    }
    Job *toFree = temp->next;
    temp->next = job->next;
    free(toFree->userName);
    free(toFree);
}

int contains(Job *queue, char *userName)
{
    int result = 0;
    while (queue != NULL)
    {
        if (strcmp(queue->userName, userName) == 0)
        {
            result = 1;
        }
        queue = queue->next;
    }
    return result;
}

void updateSummary(Job *queue, char *userName, int time)
{
    while (queue != NULL)
    {
        if (strcmp(queue->userName, userName) == 0)
        {
            if (queue->last_job < time)
            {
                queue->last_job = time;
            }
            return;
        }
        queue = queue->next;
    }
}

void *print(void *param)
{
    // int total_cpus = *(int *)param; // assuming param is the number of CPUs
    // char jobs[total_cpus];          // to temporarily hold jobs for each CPU

    // for (int i = 0; i < total_cpus; i++)
    // {
    //     sem_wait(&all_cpus_done);
    // }

    // int currentTime = 1;

    // for (int i = 0; i < 20; i++)
    // {
    //     // Wait for all CPU threads to finish a unit of execution

    //     // Get the jobs for the current time unit
    //     for (int i = 0; i < total_cpus; i++)
    //     {
    //         if (i < buffer_index)
    //         {
    //             Buffer entry = printBuffer[i];
    //             jobs[entry.cpu_id] = entry.job->processName;
    //         }
    //         else
    //         {
    //             jobs[i] = '-'; // no job for this CPU
    //         }
    //     }

    //     // Reset buffer index and finished count
    //     buffer_index = 0;
    //     finished_cpu_count = 0;

    //     // Print the jobs for the current time unit
    //     printf("Time: %d\t", currentTime);
    //     for (int i = 0; i < total_cpus; i++)
    //     {
    //         printf("CPU%d: %c\t", i, jobs[i]);
    //     }
    //     printf("\n");

    //     // Signal all CPU threads to continue
    //     for (int i = 0; i < total_cpus; i++)
    //     {
    //         sem_post(&print_done);
    //     }
    //     currentTime++;
    // }

    pthread_exit(0);
}

void *simulate(void *param)
{
    CpuThreadParam *p = (CpuThreadParam *)param;

    CPU *cpu = p->cpu;
    Job **queue = p->queue;
    Job *summary = p->summary;
    int quantum = p->quantum;
    int id = p->id;

    printf("CPU thread: time = %d\t quantum = %d\t id = %d\n", cpu->time, quantum, id);

    int simulationStarted = 0;
    Job **runningQueue = (Job **)malloc(sizeof(Job *));
    *runningQueue = NULL;
    if (*queue != NULL)
    {
        simulationStarted = (*queue)->arrivalTime;
        enqueue(runningQueue, *queue);
        removeJob(queue, *queue);
    }
    while (*runningQueue != NULL || *queue != NULL)
    {
        pthread_mutex_lock(&queue_mutex);

        while (*queue != NULL && cpu->time >= (*queue)->arrivalTime)
        {
            // Add the job to local running queue if affinity matches CPU id
            if ((*queue)->affinity == id)
            {
                // Add job to local running queue
                enqueue(runningQueue, *queue);
                // Remove job from input queue
                removeJob(queue, *queue);
            }
            else // otherwise, move to the next one
            {
                queue = &((*queue)->next);
            }
        }

        pthread_mutex_unlock(&queue_mutex);

        if (cpu->current == NULL)
        {
            if (*runningQueue != NULL && cpu->time >= (*runningQueue)->arrivalTime)
            {
                cpu->current = *runningQueue;
            }
            else
            {
                // printf("%d\t-\n", cpu->time);
            }
        }
        if (cpu->current != NULL)
        {
            printf("%d\t%c\n", cpu->time, cpu->current->processName);
            cpu->current->duration--;

            // Place job to print buffer
            pthread_mutex_lock(&print_mutex);
            printBuffer[buffer_index].job = cpu->current;
            printBuffer[buffer_index].cpu_id = id;
            printBuffer[buffer_index].time = cpu->time;
            buffer_index++;
            pthread_mutex_unlock(&print_mutex);

            // // Signal print function
            // sem_post(&print_sem);

            // // Wait for print function
            // sem_wait(&cpu_sem);

            // sem_post(&all_cpus_done);
            // sem_wait(&print_done);

            if (cpu->current->duration == 0)
            {
                updateSummary(summary, cpu->current->userName, cpu->time);
                removeJob(runningQueue, cpu->current);
                cpu->current = NULL;
                simulationStarted = cpu->time + 1;
            }
            else
            {
                if ((cpu->time - simulationStarted + 1) % quantum == 0)
                {
                    cpu->current = NULL;
                    Job *temp = *runningQueue;
                    if (temp != NULL)
                    {
                        enqueue(runningQueue, temp);
                    }
                    removeJob(runningQueue, temp);
                }
            }
        }

        cpu->time++;
    }
    pthread_exit(0);
}

void printSummary(Job *queue)
{
    printf("\nSummary\n");
    while (queue != NULL)
    {
        printf("%s\t%d\n", queue->userName, queue->last_job);
        queue = queue->next;
    }
}

int main(int argc, char **argv)
{
    int numCpu;           // number of CPUs
    scanf("%i", &numCpu); // Scan to get the number of CPU
    int quantums[numCpu]; // Create an array to store quantum for CPU

    /* For debug - Start */
    // for (int i = 0; i < numCpu; i++)
    // {
    //     scanf("%d", &quantums[i]);
    // }

    // printf("numCpu = %d\n", numCpu);
    // for (int i = 0; i < numCpu; i++)
    // {
    //     printf("quantums[%d] = %d\n", i, quantums[i]);
    // }
    /* For debug - End */

    Job *summary = NULL;
    Job *queue = NULL;
    char buf[100];
    char buf2[100];
    char buf3[100];
    char buf4[100];
    char buf5[100];

    // Initialize CPUs
    CPU cpus[numCpu];
    for (int i = 0; i < numCpu; i++)
    {
        cpus[i].current = NULL;
        cpus[i].time = 1;
        scanf("%d", &quantums[i]);
    }

    char userName[100];
    char processName;
    int arrival;
    int duration;
    int affinity;
    Job *newJob;
    scanf("%s %s %s %s %s\n", buf, buf2, buf3, buf4, buf5); // Consume the title line
    while (EOF != scanf("%s %c %d %d %d", userName, &processName, &arrival, &duration, &affinity))
    {
        // printf("%s\t%c\t%d\t%d\t%d\n", userName, processName, arrival, duration, affinity);
        if (contains(summary, userName) == 0)
        {
            newJob = (Job *)malloc(sizeof(Job));
            newJob->userName = strdup(userName);
            newJob->processName = processName;
            newJob->arrivalTime = arrival;
            newJob->duration = duration;
            newJob->next = NULL;
            newJob->affinity = affinity;
            enqueue(&summary, newJob);
            free(newJob->userName);
            free(newJob);
        }
        newJob = (Job *)malloc(sizeof(Job));
        newJob->userName = strdup(userName);
        newJob->processName = processName;
        newJob->arrivalTime = arrival;
        newJob->duration = duration;
        newJob->affinity = affinity;
        newJob->next = NULL;
        enqueue(&queue, newJob);
        free(newJob->userName);
        free(newJob);
    }

    /* For debug - Start */
    // while (queue != NULL)
    // {
    //     printf("%s\t%c\t%d\t%d\t%d\n", queue->userName, queue->processName, queue->arrivalTime, queue->duration, queue->affinity);
    //     queue = queue->next;
    // }
    /* For debug - End */

    if (pthread_mutex_init(&print_mutex, NULL) != 0)
    {
        perror("Failed to initialize print_mutex");
        return 1;
    }

    // Create prining thread before creating CPU threads
    pthread_t printing_thread;
    if (pthread_create(&printing_thread, NULL, print, &numCpu) != 0)
    {
        perror("Failed to create printing thread");
        return 1;
    }

    // init mutext for input queue
    if (pthread_mutex_init(&queue_mutex, NULL) != 0)
    {
        perror("Failed to initialize queue_mutex");
        return 1; // or handle the error as appropriate
    }
    pthread_mutex_init(&jobs_done_mutex, NULL);

    sem_init(&print_sem, 0, 0);
    sem_init(&cpu_sem, 0, numCpu);

    sem_init(&all_cpus_done, 0, 0); // Initialized to 0
    sem_init(&print_done, 0, 0);    // Initialized to 0

    // Create CPU threads
    pthread_t cpu_threads[numCpu];
    CpuThreadParam cpu_thread_param[numCpu];
    for (int i = 0; i < numCpu; i++)
    {
        // Define cpu_thread_param[i]
        cpu_thread_param[i].cpu = &cpus[i];
        cpu_thread_param[i].quantum = quantums[i];
        cpu_thread_param[i].queue = &queue;
        cpu_thread_param[i].summary = summary;
        cpu_thread_param[i].id = i;

        if (pthread_create(&cpu_threads[i], NULL, simulate, &cpu_thread_param[i]) != 0)
        {
            perror("Failed to create CPU thread\n");
            return 1;
        }
    }

    // Join all CPU threads
    for (int i = 0; i < numCpu; i++)
    {
        pthread_join(cpu_threads[i], NULL);
    }

    // Join printing thread
    pthread_join(printing_thread, NULL);

    // Print the summary in the main thread
    // This is different than what printing thread prints
    printSummary(summary);

    // Free memory
    Job *temp;
    while (queue)
    {
        temp = queue;
        queue = queue->next;
        free(temp->userName);
        free(temp);
    }
    while (summary)
    {
        temp = summary;
        summary = summary->next;
        free(temp->userName);
        free(temp);
    }
    // Destory mutex and semaphore
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&print_mutex);
    sem_destroy(&print_sem);
    sem_destroy(&cpu_sem);

    return 0;
}