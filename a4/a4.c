#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    char *user;
    char process;
    int arrival;
    int duration;
    struct node *prev;
    struct node *next;
} Node;

/*
 * adds an element to the end of the list
 */
void add(Node **head, char *user, char process, int arrival, int duration)
{
    // allocate memory for new node
    Node *newNode = (Node *)malloc(sizeof(Node));

    // error handling
    if (newNode == NULL)
    {
        return;
    }

    // assign user to new node
    newNode->user = strdup(user); // allocates memory upon copy string
    if (newNode->user == NULL)
    {
        perror("strdup failed\n");
        free(newNode);
        return;
    }
    newNode->process = process;
    newNode->arrival = arrival;
    newNode->duration = duration;

    // next element of new node is empty
    newNode->next = NULL;

    // if the list is empty
    if (*head == NULL)
    {
        newNode->prev = NULL;
        *head = newNode; // head now points to the first element in list
        return;
    }

    // store the head node for traversing
    Node *current = *head;

    // find the last element in the linked list
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = newNode;
    newNode->prev = current;
}

/*
 * removes the first occurrance of the element specified by the second paramter from the linked list
 */
void delete(Node **head, char process)
{
    // if list is empty
    if (*head == NULL)
    {
        return;
    }

    // store the head pointer for traversing
    Node *current = *head;

    // find the last element in the linked list
    while (current != NULL)
    {
        // if a match is found
        if (current->process == process)
        {
            if (current->prev != NULL)
            {
                current->prev->next = current->next;
            }
            else
            {
                *head = current->next;
            }

            if (current->next != NULL)
            {
                current->next->prev = current->prev;
            }

            // free up memory
            free(current->user);
            free(current);

            return;
        }
        current = current->next;
    }
}

/*
 * query that returns 0 or 1, if the user is included in the list or not
 */
int contains(Node *head, char *user)
{
    // if list is empty
    if (head == NULL)
    {
        return 0;
    }

    while (head != NULL)
    {
        // if a match is found
        if (strcmp(head->user, user) == 0)
        {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

/*
 * find the user in the list, then update duration (used as the time of the last job is finished for the user) for summary queue
 */
void findAndUpdate(Node *head, char *user, int duration)
{
    // if list is empty
    if (head == NULL)
    {
        return;
    }

    while (head != NULL)
    {
        // if a match is found
        if (strcmp(head->user, user) == 0)
        {
            head->duration = duration; // copy the new data over
            return;
        }
        head = head->next;
    }
}

/*
 * prints summary queue
 */
void printSummary(Node *head)
{
    printf("Summary\n");
    // if list is empty
    if (head == NULL)
    {
        printf("\n");
        return;
    }

    // traverse list
    while (head != NULL)
    {
        printf("%s\t%d\n", head->user, head->duration);
        head = head->next;
    }
}

/*
 * frees all resources
 */
int stop(Node **head)
{
    // if list is empty
    if (*head == NULL)
    {
        return 0;
    }

    while (*head != NULL)
    {
        Node *temp = *head;
        *head = (*head)->next;
        free(temp->user);
        free(temp);
    }
    *head = NULL; // reset head after free memory

    return 0;
}

int main()
{
    // input task queue
    Node *inputQueueHead = NULL;

    int q;          // time quantum
    char user[101]; // assuming a maximum 100 char username
    char line[500]; // line buffer

    // Read time quantum
    if (fgets(line, sizeof(line), stdin) != NULL)
    {
        sscanf(line, "%d", &q);
    }

    // counter for quantum
    int qCount = q;

    // Skip title line
    fgets(line, sizeof(line), stdin);

    // Read jobs and add them to the linked list
    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        char process;
        int arrival, duration;

        sscanf(line, "%s %c %d %d", user, &process, &arrival, &duration);
        add(&inputQueueHead, user, process, arrival, duration);
    }

    int t = 0; // CPU time counter

    Node *schedulingQueueHead = NULL;
    Node *summaryQueueHead = NULL;

    Node *schedulingQueueCurrent = NULL;

    printf("Time Job\n");

    while (inputQueueHead != NULL || schedulingQueueHead != NULL)
    {
        t++; // increment CPU time

        while (inputQueueHead != NULL)
        {
            // If the current job's arrival time matches the CPU time
            if (inputQueueHead->arrival == t)
            {
                // Add to scheduling queue
                add(&schedulingQueueHead, inputQueueHead->user, inputQueueHead->process, inputQueueHead->arrival, inputQueueHead->duration);

                // Add or update to summary queue
                if (contains(summaryQueueHead, inputQueueHead->user))
                {
                    findAndUpdate(summaryQueueHead, inputQueueHead->user, 0);
                }
                else
                {
                    add(&summaryQueueHead, inputQueueHead->user, ' ', 0, 0);
                }

                // If schedulingQueueCurrent is NULL, set it to point to schedulingQueueHead
                // Only do this when the first job arrives
                if (!schedulingQueueCurrent)
                {
                    schedulingQueueCurrent = schedulingQueueHead;
                }

                // Remove from input queue. The head is updated using the function.
                delete (&inputQueueHead, inputQueueHead->process);
            }
            else
            {
                break;
            }
        }

        if (schedulingQueueCurrent != NULL)
        {
            // check if the current job is finished
            if (schedulingQueueCurrent->duration == 0)
            {
                // Update summary
                findAndUpdate(summaryQueueHead, schedulingQueueCurrent->user, t - 1);

                // Remove finished job from the scheduling queue
                delete (&schedulingQueueHead, schedulingQueueCurrent->process);

                // The job from the queue head should be scheduled
                schedulingQueueCurrent = schedulingQueueHead;

                // reset quantum
                qCount = q;
            }
            else if (qCount == 0)
            {
                // The next job should be scheduled
                if (schedulingQueueCurrent->next)
                    schedulingQueueCurrent = schedulingQueueCurrent->next;

                // reset quantum
                qCount = q;
            }

            if (schedulingQueueCurrent && schedulingQueueCurrent->duration > 0)
            {
                printf("%d\t%c\n", t, schedulingQueueCurrent->process);
                schedulingQueueCurrent->duration--;
            }
            else
            {
                // catch the case schedulingQueueCurrent just becomes null
                // or duration is 0
                printf("%d\t-\n", t);
            }

            qCount--;
        }
        else
        {
            // No jobs in scheduling queue
            // Print current time and -
            printf("%d\t-\n", t);
        }
    }
    printf("\n");

    printSummary(summaryQueueHead);

    // lastly, free memory
    stop(&inputQueueHead);
    stop(&schedulingQueueHead);
    stop(&summaryQueueHead);
    return 0;
}