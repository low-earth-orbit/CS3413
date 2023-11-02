#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

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

char *printBuffer;

sem_t all_cpus_done; // Signal from all CPUs to print
sem_t print_done;    // Signal from print to all CPUs
pthread_mutex_t cpus_done_mutex = PTHREAD_MUTEX_INITIALIZER;
int completed_cpus = 0;
int numCpu = 0;

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
    int print_time = 1;

    // int numCpu = *(int *)param;
    // printf("Total cpus in print thread: %d\n", numCpu);
    char jobs[numCpu];
    printf("Time\t");
    for (int i = 0; i < numCpu; i++)
    {
        printf("CPU%d\t", i);
    }
    printf("\n");

    while (1)
    {
        for (int i = 0; i < numCpu; i++)
        {
            sem_wait(&all_cpus_done);
        }

        printf("%d\t", print_time);
        for (int i = 0; i < numCpu; i++)
        {
            pthread_mutex_lock(&queue_mutex);
            printf("%c\t", printBuffer[i]);
            pthread_mutex_unlock(&queue_mutex);
        }
        printf("\n");

        print_time++; // Increment the time

        // Signal all CPU threads to continue
        for (int i = 0; i < numCpu; i++)
        {
            // printf("[PRINT] Sending signal to CPU%d to proceed.\n", i);
            sem_post(&print_done);
        }

        pthread_mutex_lock(&cpus_done_mutex);
        if (completed_cpus >= numCpu)
        {
            pthread_mutex_unlock(&cpus_done_mutex);
            break;
        }
        pthread_mutex_unlock(&cpus_done_mutex);
    }
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
    int simulationStarted = 1;
    Job **runningQueue = (Job **)malloc(sizeof(Job *));
    *runningQueue = NULL;

    int firstJob = 0;

    do
    {
        sem_wait(&print_done);

        // printf("In CPU%d \n", id);
        // Lock the read process from input queue to CPU thread
        // Load input queue to local running queue
        pthread_mutex_lock(&queue_mutex);

        // Use a temp pointer for traversal without modifying the queue.
        Job *tempPtr = *queue;

        // Loads matching jobs to running queue
        while (tempPtr != NULL)
        {
            // Add the job to local running queue if affinity matches CPU id
            if (tempPtr->affinity == id && cpu->time >= tempPtr->arrivalTime)
            {
                if (firstJob == 0)
                {
                    simulationStarted = tempPtr->arrivalTime;
                    firstJob = 1;
                }
                // Add job to local running queue without removing it from the main queue
                enqueue(runningQueue, tempPtr);
                removeJob(queue, tempPtr);
            }

            // Move to the next item in the queue
            // Until every item for the unit of time has been loaded
            tempPtr = tempPtr->next;
        }

        pthread_mutex_unlock(&queue_mutex);

        // set buffer to '-' whatever
        pthread_mutex_lock(&queue_mutex);
        printBuffer[id] = '-';
        // printf("CPU%d, time%d, printBuffer: %s\n", id, cpu->time, printBuffer);
        pthread_mutex_unlock(&queue_mutex);

        if (*runningQueue != NULL)
        {
            // printf("CPU%d time = %d\n", id, cpu->time);
            if (cpu->current == NULL)
            {
                cpu->current = *runningQueue;
                simulationStarted = cpu->time;
            }

            if (cpu->current != NULL)
            {
                // printf("CPU%d\t%d\t%c\n", id, cpu->time, cpu->current->processName);

                // Place job to print buffer
                pthread_mutex_lock(&queue_mutex);
                printBuffer[id] = cpu->current->processName;
                // printf("CPU%d\t%d\t%c\n", id, cpu->time, cpu->current->processName);
                // printf("CPU%d, time%d, printBuffer: %s\n", id, cpu->time, printBuffer);
                pthread_mutex_unlock(&queue_mutex);

                cpu->current->duration--;
                if (cpu->current->duration == 0)
                {
                    updateSummary(summary, cpu->current->userName, cpu->time);
                    removeJob(runningQueue, cpu->current);
                    cpu->current = NULL;
                    simulationStarted = cpu->time + 1;
                }
                else if ((cpu->time - simulationStarted + 1) % quantum == 0)
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
        // printf("[SIMULATE CPU%d] Finished time unit %d and signaled PRINT.\n", id, cpu->time);
        sem_post(&all_cpus_done);

        // printf("[SIMULATE CPU%d] Received signal from PRINT to proceed with time unit %d.\n", id, cpu->time + 1);

        if (*runningQueue == NULL && *queue == NULL)
        {
            // set buffer to '-' whatever
            pthread_mutex_lock(&queue_mutex);
            printBuffer[id] = '-';
            // printf("CPU%d, time%d, printBuffer: %s\n", id, cpu->time, printBuffer);
            pthread_mutex_unlock(&queue_mutex);

            pthread_mutex_lock(&cpus_done_mutex);
            completed_cpus++;
            pthread_mutex_unlock(&cpus_done_mutex);

            sem_post(&all_cpus_done);
        }
    } while (*runningQueue != NULL || *queue != NULL);

    sem_wait(&print_done);

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
    scanf("%i", &numCpu); // Scan to get the number of CPU
    int quantums[numCpu]; // Create an array to store quantum for CPU

    Job *summary = NULL;
    Job *queue = NULL;
    char buf[100];
    char buf2[100];
    char buf3[100];
    char buf4[100];
    char buf5[100];

    // Create CPUs
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
        }
        newJob = (Job *)malloc(sizeof(Job));
        newJob->userName = strdup(userName);
        newJob->processName = processName;
        newJob->arrivalTime = arrival;
        newJob->duration = duration;
        newJob->affinity = affinity;
        newJob->next = NULL;
        enqueue(&queue, newJob);
    }
    sem_init(&all_cpus_done, 0, 1);
    sem_init(&print_done, 0, 2);

    // init mutext for input queue
    if (pthread_mutex_init(&queue_mutex, NULL) != 0)
    {
        perror("Failed to initialize queue_mutex");
        return 1; // or handle the error as appropriate
    }

    // Initialize print buffer
    printBuffer = malloc((numCpu + 1) * sizeof(char));
    for (int i = 0; i < numCpu; i++)
    {
        printBuffer[i] = '-';
    }
    printBuffer[numCpu] = '\0';

    // Create printing thread before creating CPU threads
    pthread_t printing_thread;
    if (pthread_create(&printing_thread, NULL, print, &numCpu) != 0)
    {
        perror("Failed to create printing thread");
        return 1;
    }

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

    // Join printing thread
    pthread_join(printing_thread, NULL);

    // Join all CPU threads
    for (int i = 0; i < numCpu; i++)
    {
        pthread_join(cpu_threads[i], NULL);
    }

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
    sem_destroy(&all_cpus_done);
    sem_destroy(&print_done);

    return 0;
}