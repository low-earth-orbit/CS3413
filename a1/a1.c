#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    char *data;
    struct node *prev;
    struct node *next;
} Node;

/*
 * adds an element to the end of the list
 */
void add(Node **head, char *data)
{
    // allocate memory for new node
    Node *newNode = (Node *)malloc(sizeof(Node));

    // error handling
    if (newNode == NULL)
    {
        return;
    }

    // assign data to new node
    newNode->data = strdup(data); // allocates memory upon copy string

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
void delete(Node **head, char *data)
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
        if (strcmp(current->data, data) == 0)
        {
            // if deleted element is the first element
            if (current->prev == NULL)
            {
                // set head
                if (current->next == NULL) // if deleted element is the last element
                {
                    *head = NULL;
                }
                else
                {
                    current->next->prev = NULL;
                    *head = current->next;
                }
            }
            else
            {
                if (current->next == NULL) // if deleted element is the last element
                {
                    current->prev->next = NULL;
                }
                else
                {
                    current->next->prev = current->prev;
                    current->prev->next = current->next;
                }
            }

            // free up memory
            free(current->data);
            free(current);

            return;
        }
        current = current->next;
    }
}

/*
 * query that returns 0 or 1, if the element specified by the second parameter is in a linked list or not
 */
int contains(Node *head, char *data)
{
    // if list is empty
    if (head == NULL)
    {
        return 0;
    }

    while (head != NULL)
    {
        // if a match is found
        if (strcmp(head->data, data) == 0)
        {
            return 1;
        }
        head = head->next;
    }
    return 0;
}

/*
 * looks up for the first occurance of the element specified by the second element and replaces it with the value specified by the third parameter
 */
void findAndReplace(Node *head, char *data, char *newData)
{
    // if list is empty
    if (head == NULL)
    {
        return;
    }

    while (head != NULL)
    {
        // if a match is found
        if (strcmp(head->data, data) == 0)
        {
            free(head->data);             // free up memeory for old data
            head->data = strdup(newData); // copy the new data over
            return;
        }
        head = head->next;
    }
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
        printf("%s\n", head->data);
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

    printf("%s\n", node->data);
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
        printf("Data: %s; Prev: ", head->data);
        if (head->prev != NULL)
        {
            printf("%s", head->prev->data);
        }
        else
        {
            printf("NULL");
        }

        printf("; Next: ");
        if (head->next != NULL)
        {
            printf("%s", head->next->data);
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
        free(temp->data);
        free(temp);
    }
    *head = NULL; // reset head after free memory

    return 0;
}

int main()
{
    Node *head = NULL;

    char line[206];
    char operation;
    char data[101];    // assuming a maximum length of 100
    char newData[101]; // assuming a maximum length of 100
    while ((fgets(line, sizeof(line), stdin) != NULL))
    {
        sscanf(line, " %c %s %s", &operation, data, newData); // sscanf to parse the line
        switch (operation)
        {
        case 'a':
            add(&head, data);
            break;
        case 'd':
            delete (&head, data);
            break;
        case 'c':
            contains(head, data);
            break;
        case 'f':
            findAndReplace(head, data, newData);
            break;
        case 'p':
            printList(head);
            break;
        case 's':
            stop(&head);
            break;
        default:
            break;
        }
        // debug
        // printf("Operation: %c\n", operation);
        // printf("List is now:\n");
        // printFullList(head);
    }

    // lastly, free memory
    stop(&head);
    return 0;
}