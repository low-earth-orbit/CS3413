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
 * query that returns 0 or 1, if the element specified by the second parameter is in a linked list or not
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
 * prints each element in the linked list to the console
 */
void printList(Node *head)
{
    // if list is empty
    if (head == NULL)
    {
        printf("\n");
        return;
    }

    // traverse list
    while (head != NULL)
    {
        printf("%s\t%c\t%d\t%d\n", head->user, head->process, head->arrival, head->duration);
        head = head->next;
    }
}

/*
 * prints single node to the console
 */
void printSingleNode(Node *node)
{
    // if node is empty
    if (node == NULL)
    {
        printf("NULL\n");
        return;
    }

    printf("%s\t%c\t%d\t%d\n", node->user, node->process, node->arrival, node->duration);
}

/*
 * prints each element and its prev and next elements in the linked list to the console
 */
void printFullList(Node *head)
{
    // if list is empty
    if (head == NULL)
    {
        printf("head is NULL. list may be empty.\n");
        return;
    }

    printf("Value of head node: ");
    printSingleNode(head);

    // traverse list
    while (head != NULL)
    {
        printf("User: %s; Prev: ", head->user);
        if (head->prev != NULL)
        {
            printf("%s", head->prev->user);
        }
        else
        {
            printf("NULL");
        }

        printf("; Next: ");
        if (head->next != NULL)
        {
            printf("%s", head->next->user);
        }
        else
        {
            printf("NULL");
        }
        printf(";\n");
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
    Node *head = NULL;

    int n;          // time quantum
    char user[101]; // assuming a maximum 100 char username
    char line[500]; // line buffer

    // Read time quantum
    if (fgets(line, sizeof(line), stdin) != NULL)
    {
        sscanf(line, "%d", &n);
    }

    // Skip title line
    fgets(line, sizeof(line), stdin);

    // Read jobs and add them to the linked list
    while (fgets(line, sizeof(line), stdin) != NULL)
    {
        char process;
        int arrival, duration;

        sscanf(line, "%100s %c %d %d", user, &process, &arrival, &duration);
        add(&head, user, process, arrival, duration);
    }

    // Print the list
    printList(head);

    // lastly, free memory
    stop(&head);
    return 0;
}