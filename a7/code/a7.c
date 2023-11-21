#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define OFFSETBITS 12
#define PAGESIZE (1 << OFFSETBITS)
#define PAGETABLEBITS 20
#define ARCH 32
int framesSize;
unsigned int pageFault = 0;
unsigned int pageHits = 0;
unsigned int pagesSwapped = 0;
int numOfPages = (1 << PAGETABLEBITS);

typedef struct page
{
    int pageId;
    int dirty;   // page has been written
    int read;    // page has been accessed for reading
    int frameId; // -1 if not in physical memory
    struct page *prev;
    struct page *next;
} Page;

Page *pageTable; // Global variable for the page table
int *frames;     // Global variable for the frames

void initializePageTable(int numOfPages)
{
    pageTable = (Page *)malloc(numOfPages * sizeof(Page));
    if (pageTable == NULL)
    {
        printf("Allocating page table failed\n");
    }
    for (int i = 0; i < numOfPages; i++)
    {
        pageTable[i].pageId = i;
        pageTable[i].dirty = 0;
        pageTable[i].read = 0;
        pageTable[i].frameId = -1; // Initially, no page is in memory
        pageTable[i].prev = NULL;
        pageTable[i].next = NULL;
    }
}

Page *lruFront, *lruRear = NULL; // Global variables for LRU list

void addToFrontOfLRU(Page *page)
{
    page->next = lruFront;
    page->prev = NULL;
    if (lruFront != NULL)
    {
        lruFront->prev = page;
    }
    lruFront = page;
    if (lruRear == NULL)
    {
        lruRear = page;
    }
}

void removeFromLRU(Page *page)
{
    if (page->prev != NULL)
    {
        page->prev->next = page->next;
    }
    else
    {
        lruFront = page->next;
    }
    if (page->next != NULL)
    {
        page->next->prev = page->prev;
    }
    else
    {
        lruRear = page->prev;
    }
}

int getPageNumber(unsigned long logicalAddress)
{
    return logicalAddress / PAGESIZE;
}

int getPageOffsetAddress(unsigned long logicalAddress)
{
    return logicalAddress % PAGESIZE;
}

void printPhysicalAddress(int frameID, unsigned long logicalAddress)
{
    printf("%lu -> %i\n", logicalAddress, frameID * PAGESIZE + getPageOffsetAddress(logicalAddress));
}

double computeFormula()
{
    // TODO work on that
    return 0.0f;
}

int findFreeFrame()
{
    for (int i = 0; i < framesSize; i++)
    {
        if (!frames[i])
        {
            frames[i] = 1; // Mark the frame as occupied
            return i;      // Return the frame ID
        }
    }
    return -1; // No free frame found
}

void handlePageFault(Page *page, unsigned long logicalAddress, bool isWrite)
{
    // Find a free frame
    int freeFrameId = findFreeFrame();
    if (freeFrameId != -1)
    {
        // Free frame found
        // Assign the free frame to the page
        page->frameId = freeFrameId;

        // Print
        printPhysicalAddress(page->frameId, logicalAddress);
    }
    else
    {
        // No free frame
        // Identify the least recently used page
        Page *victimPage = lruRear;

        // The while loop is needed for catching the case that the page has only been accessed for reading
        while (victimPage != NULL)
        {
            // If victimPage is dirty, swap out
            if (victimPage->dirty || !victimPage->read)
            {

                if (victimPage->dirty)
                {
                    pagesSwapped++; // Increment swapped page counter if the page is dirty
                }

                // Remove from LRU
                removeFromLRU(victimPage);

                // Now use victimPage's frame for the new page
                page->frameId = victimPage->frameId;
                page->dirty = isWrite;
                page->read = !isWrite;

                victimPage->frameId = -1;
                victimPage->dirty = 0;
                victimPage->read = 0;

                // Print
                printPhysicalAddress(page->frameId, logicalAddress);

                break;
            }

            // Move to the previous page since this page has only been accessed for reading
            victimPage = victimPage->prev;
        }
    }
    // Add the new page to the LRU list
    addToFrontOfLRU(page);
}

void accessPage(unsigned long logicalAddress, bool isWrite)
{
    int pageNumber = getPageNumber(logicalAddress);
    Page *page = &pageTable[pageNumber];

    if (page->frameId == -1)
    {
        // Page fault encountered
        handlePageFault(page, logicalAddress, isWrite);
        pageFault++;
    }
    else
    {
        // Page found
        printPhysicalAddress(page->frameId, logicalAddress);
        pageHits++;
    }

    if (isWrite)
    {
        page->dirty = 1;
    }
    else
    {
        page->read = 1;
    }

    // Move the page to the front of the LRU list
    removeFromLRU(page);
    addToFrontOfLRU(page);
}

void readFromAddress(unsigned long logicalAddress)
{
    accessPage(logicalAddress, false);
}

void writeToAddress(unsigned long logicalAddress)
{
    accessPage(logicalAddress, true);
}

int main(int argc, char **argv)
{
    framesSize = atoi(argv[1]);

    // Create frames array, which stores the status of frame 1: occupied; 0: free
    frames = (int *)malloc(framesSize * sizeof(int));
    if (frames == NULL)
    {
        printf("Allocating frames failed\n");
    }

    for (int i = 0; i < framesSize; i++)
    {
        frames[i] = 0;
    }

    // Initialize page table
    initializePageTable(numOfPages);

    unsigned long logicalAddress;
    char operation;
    printf("Logical addresses -> Physical addresses:\n");
    while (EOF != scanf("%c %lu\n", &operation, &logicalAddress))
    {
        if (operation == 'r')
        {
            readFromAddress(logicalAddress);
        }
        else
        {
            writeToAddress(logicalAddress);
        }
    }

    printf("\nStats:\nmajor page faults = %u\npage hits = %u\npages swapped out = %u\nEffective Access Time = %.4f\n", pageFault, pageHits, pagesSwapped, computeFormula());

    // Free page table
    free(pageTable);
    // Free frames array
    free(frames);
    return 0;
}