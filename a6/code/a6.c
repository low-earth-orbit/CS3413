#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// 1 - best fit
// 2 - worst fit
// 3 - first fit
int algorithm;
int memSize;
int totalAllocated = 0;
int totalMemAllocated = 0;
int totalFailed = 0;
int totalTerminated = 0;
int totalFreedMemory = 0;

typedef struct MemoryBlock
{
    int startAddress;
    int endAddress;
    int processId; // -1 if free
    struct MemoryBlock *next;
} MemoryBlock;

MemoryBlock *head = NULL;

void initializeMemory(int size)
{
    head = (MemoryBlock *)malloc(sizeof(MemoryBlock));
    head->startAddress = 0;
    head->endAddress = size - 1;
    head->processId = -1; // Set to -1 if the block is free
    head->next = NULL;
}

int doFree(int processId)
{
    MemoryBlock *current = head;
    MemoryBlock *prev = NULL;
    while (current != NULL)
    {
        if (current->processId == processId)
        {
            current->processId = -1; // Mark block as free
            totalTerminated++;
            totalFreedMemory += current->endAddress - current->startAddress + 1;
            // Merge with next block if it's also free
            if (current->next != NULL && current->next->processId == -1)
            {
                MemoryBlock *temp = current->next;
                current->endAddress = temp->endAddress;
                current->next = temp->next;
                free(temp);
            }
            // Merge with previous block if it's also free
            if (prev != NULL && prev->processId == -1)
            {
                prev->endAddress = current->endAddress;
                prev->next = current->next;
                free(current);
                current = prev;
            }
            return 1; // Success
        }
        prev = current;
        current = current->next;
    }
    totalFailed++; // Increment counter
    printf("Process %d failed to free memory\n", processId);
    return 0; // Failure: processId not found
}

int doAllocate(int howMuchToAllocate, int processId)
{

    switch (algorithm)
    {
    case 1: // 1 - best fit
    {
        break;
    }
    case 2: // 2 - worst fit
    {
        MemoryBlock *current = head;
        int biggest = getBiggest(); // Get biggest available memory size
        // Iterate all memory blocks
        while (current != NULL)
        {
            int blockSize = current->endAddress - current->startAddress + 1;
            if (current->processId == -1 && blockSize >= howMuchToAllocate && blockSize == biggest)
            {
                // Allocate memory
                if (blockSize > howMuchToAllocate)
                {
                    // Split the block
                    MemoryBlock *newBlock = (MemoryBlock *)malloc(sizeof(MemoryBlock));
                    newBlock->startAddress = current->startAddress + howMuchToAllocate;
                    newBlock->endAddress = current->endAddress;
                    newBlock->processId = -1; // The new block is free
                    newBlock->next = current->next;

                    current->endAddress = current->startAddress + howMuchToAllocate - 1;
                    current->next = newBlock;
                }
                current->processId = processId;         // Assign block with process ID
                totalAllocated++;                       // Increment counter
                totalMemAllocated += howMuchToAllocate; // Increment counter
                return 1;                               // Success
            }
            current = current->next; // Go to the next block
        }
        printf("Process %d failed to allocate %d memory\n", processId, howMuchToAllocate);
        totalFailed++; // Increment counter
        return 0;      // Failure
        break;
    }
    case 3: // 3 - first fit
    {
        MemoryBlock *current = head;
        // Iterate all memory blocks
        while (current != NULL)
        {
            int blockSize = current->endAddress - current->startAddress + 1;
            if (current->processId == -1 && blockSize >= howMuchToAllocate)
            {
                // Allocate memory
                if (blockSize > howMuchToAllocate)
                {
                    // Split the block
                    MemoryBlock *newBlock = (MemoryBlock *)malloc(sizeof(MemoryBlock));
                    newBlock->startAddress = current->startAddress + howMuchToAllocate;
                    newBlock->endAddress = current->endAddress;
                    newBlock->processId = -1; // The new block is free
                    newBlock->next = current->next;

                    current->endAddress = current->startAddress + howMuchToAllocate - 1;
                    current->next = newBlock;
                }
                current->processId = processId;         // Assign block with process ID
                totalAllocated++;                       // Increment counter
                totalMemAllocated += howMuchToAllocate; // Increment counter
                return 1;                               // Success
            }
            current = current->next; // Go to the next block
        }
        printf("Process %d failed to allocate %d memory\n", processId, howMuchToAllocate);
        totalFailed++; // Increment counter
        return 0;      // Failure
        break;
    }
    default:
    {
        printf("There was an error, the algorithm is uninitialized");
        exit(0);
    }
    }
}

int calcFinalMemory()
{
    int freeMemory = 0;
    MemoryBlock *current = head;
    while (current != NULL)
    {
        if (current->processId == -1) // Memory block is not used by process
        {
            freeMemory += current->endAddress - current->startAddress + 1;
        }
        current = current->next;
    }
    return freeMemory;
}

// Returns # of free memory chuncks
// Not total memory chuncks in the linked list
int getNumberOfChunks()
{
    int count = 0;
    MemoryBlock *current = head;
    while (current != NULL)
    {
        if (current->processId == -1) // Memory block is not used by process
            count++;
        current = current->next;
    }
    return count;
}

int getSmallest()
{
    int smallest = memSize; // Initialize with maximum possible size
    int found = 0;
    MemoryBlock *current = head;
    while (current != NULL)
    {
        if (current->processId == -1) // Memory block is not used by process
        {
            int blockSize = current->endAddress - current->startAddress + 1;
            if (blockSize <= smallest)
            {
                if (found == 0)
                    found = 1;
                smallest = blockSize;
            }
        }
        current = current->next;
    }

    if (found == 0)
        return 0;

    return smallest;
}

int getBiggest()
{
    int biggest = 0; // Initialize with minimum possible size
    int found = 0;
    MemoryBlock *current = head;
    while (current != NULL)
    {
        if (current->processId == -1) // Memory block is not used by process
        {
            int blockSize = current->endAddress - current->startAddress + 1;
            if (blockSize >= biggest)
            {
                if (found == 0)
                    found = 1;
                biggest = blockSize;
            }
        }
        current = current->next;
    }
    if (found == 0)
        return 0;
    return biggest;
}

int main(int argc, char **argv)
{
    int i = 0;
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-b") == 0)
        {
            algorithm = 1;
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            algorithm = 2;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            memSize = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-f") == 0)
        {
            algorithm = 3;
        }
    }
    if (memSize >= 0)
    {
        // initialize your memory here
        initializeMemory(memSize);
    }
    else
    {
        printf("The program requires size\n");
        exit(0);
    }
    char operation;
    int id = 1337;
    int size;
    while (EOF != scanf("%c", &operation))
    {
        switch (operation)
        {
        case 'N':
            scanf(" %d %d\n", &id, &size);
            doAllocate(size, id);
            break;
        case 'T':
            scanf(" %d\n", &id);
            doFree(id);
            break;
        case 'S':
            printf("Total Processes created %d, Total allocated memory %d, Total Processes\nterminated %d, Total freed memory %d, Final memory available %d, Final\nsmallest and largest fragmented memory sizes %d and %d, total failed requests:%d, number of memory chunks: %d\n", totalAllocated, totalMemAllocated, totalTerminated, totalFreedMemory, calcFinalMemory(), getSmallest(), getBiggest(), totalFailed, getNumberOfChunks());
            break;
        }
    }
    // Free all the allocated memory at the end
    MemoryBlock *current = head;
    while (current != NULL)
    {
        MemoryBlock *temp = current;
        current = current->next;
        free(temp);
    }

    return 0;
}