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

char *printBuffer;

sem_t cpu_sem[100];                                       // CPU semaphore array
sem_t print_done[100];                                    // Signal from print to CPU
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER; // mutex to protect r/w of print buffer
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;  // mutex to protect r/w of input queue among CPU threads
int completed_cpus = 0;
int numCpu = 0;
int isDone = 0;

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
            sem_wait(&cpu_sem[i]);
        }

        printf("%d\t", print_time);
        for (int i = 0; i < numCpu; i++)
        {
            pthread_mutex_lock(&buffer_mutex);
            printf("%c\t", printBuffer[i]);
            pthread_mutex_unlock(&buffer_mutex);
        }
        printf("\n");

        print_time++; // Increment the time

        if (completed_cpus >= numCpu)
        {
            break;
        }

        for (int i = 0; i < numCpu; i++)
        {
            sem_post(&print_done[i]);
        }
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
        pthread_mutex_lock(&buffer_mutex);
        printBuffer[id] = '-';
        pthread_mutex_unlock(&buffer_mutex);

        if (*runningQueue != NULL)
        {
            if (cpu->current == NULL)
            {
                cpu->current = *runningQueue;
                simulationStarted = cpu->time;
            }

            if (cpu->current != NULL)
            {
                // Place job to print buffer
                pthread_mutex_lock(&buffer_mutex);
                printBuffer[id] = cpu->current->processName;
                pthread_mutex_unlock(&buffer_mutex);

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
        sem_post(&cpu_sem[id]);
        printf("In CPU%d (6)\n", id);
        sem_wait(&print_done[id]);
        printf("In CPU%d (7)\n", id);
    } while (*runningQueue != NULL || *queue != NULL);

    printf("In CPU%d (8)\n", id);
    // if (*runningQueue == NULL && *queue == NULL)
    // {
    //     sem_wait(&print_done);
    //     // set buffer to '-' whatever
    //     pthread_mutex_lock(&buffer_mutex);
    //     printBuffer[id] = '-';
    //     // printf("CPU%d, time%d, printBuffer: %s\n", id, cpu->time, printBuffer);
    //     pthread_mutex_unlock(&buffer_mutex);

    //     pthread_mutex_lock(&queue_mutex);
    //     completed_cpus++;
    //     pthread_mutex_unlock(&queue_mutex);

    // }

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

    // Quantums array
    int *quantums = (int *)malloc(numCpu * sizeof(int));
    if (quantums == NULL)
    {
        perror("Failed to allocate memory for quantums\n");
        return 1;
    }

    Job *summary = NULL;
    Job *queue = NULL;
    char buf[100];
    char buf2[100];
    char buf3[100];
    char buf4[100];
    char buf5[100];

    // Create CPUs
    CPU *cpus = (CPU *)malloc(numCpu * sizeof(CPU));
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
    // Initialize CPU semaphore
    for (int i = 0; i < numCpu; i++)
    {
        sem_init(&cpu_sem[i], 0, 0);
    }

    for (int i = 0; i < numCpu; i++)
    {
        sem_init(&print_done[i], 0, 0);
    }

    // init mutext for input queue
    if (pthread_mutex_init(&buffer_mutex, NULL) != 0)
    {
        perror("Failed to initialize buffer_mutex");
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
    // Dynamically allocating an array for thread parameters
    CpuThreadParam *cpu_thread_param = malloc(numCpu * sizeof(CpuThreadParam));
    if (cpu_thread_param == NULL)
    {
        perror("Failed to allocate memory for thread parameters\n");
        return 1;
    }

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
    pthread_mutex_destroy(&buffer_mutex);

    for (int i = 0; i < numCpu; i++)
    {
        sem_destroy(&cpu_sem[i]);
        sem_destroy(&print_done[i]);
    }

    free(cpus);
    free(cpu_thread_param);
    free(quantums);

    return 0;
}