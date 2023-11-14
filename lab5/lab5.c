#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define OFFSETBITS 12
// fancy way of computing 2^OFFSETBITS
#define PAGESIZE (1 << OFFSETBITS)
#define WORDSIZE 32
int getPageNumber(int logicalAddress)
{
    return logicalAddress / PAGESIZE;
}

int getPageOffset(int logicalAddress)
{
    return logicalAddress % PAGESIZE;
}
int getStartingAddressOfTheFrameByIndex(int indexInFramesArray)
{
    return indexInFramesArray * PAGESIZE;
}

int main(int argc, char **argv)
{
    int MAX_NUM_FRAMES = pow(2, WORDSIZE - OFFSETBITS);
    int *frames = (int *)malloc(MAX_NUM_FRAMES * sizeof(int));
    for (int i = 0; i < MAX_NUM_FRAMES; i++)
    {
        frames[i] = -1; // Initialize frames to -1 indicating they're empty
    }

    int maxPreviouslyUsedFrame = 0;
    int logicalAddress;
    int i;
    int hasAFrameAssignedBefore = 0;
    int tempFrameIndex = 0;
    while (EOF != scanf("%d\n", &logicalAddress))
    {
        /** reset the value of hasAFrameAssignedBefore to 0 */
        hasAFrameAssignedBefore = 0;
        /* for all the values from 0 to maxPreviouslyUsedFrame, check if there's a page number of the current logical address in the frame array. if it is, mark hasAFrameAssignedBefore as 1 and store the index of the frame you just found into tempFrameIndex */

        int pageNumber = getPageNumber(logicalAddress);
        int pageOffset = getPageOffset(logicalAddress);
        int tempFrameIndex = 0;

        for (int i = 0; i <= maxPreviouslyUsedFrame; i++)
        {
            if (frames[i] == pageNumber)
            {
                hasAFrameAssignedBefore = 1;
                tempFrameIndex = i;
                break;
            }
        }
        /* After looking up through all the values from 0 to maxPreviouslyUsed, if the page did not have a frame assigned before, assign the page number to the frame array in the maxPreviously used frame position, store the max previously used frame into tempFrameIndex,  increment the value of the maxPreviously used frame */

        // If a frame is not assigned before for the logical address
        // Assign a new fram for the address
        if (hasAFrameAssignedBefore == 0)
        {
            frames[maxPreviouslyUsedFrame] = pageNumber;
            tempFrameIndex = maxPreviouslyUsedFrame;
            maxPreviouslyUsedFrame++;
        }

        /* now that you have the physical frame index corresponding to your page stored in tempFrameIndex, get the starting address of the frame by index and add the page offset to it and voila, you have your physical address. Print it out in the format
        printf("Physical address is %i logical address is %i\n" ..);
        */
        printf("Read %i\n", logicalAddress);

        int physicalAddress = getStartingAddressOfTheFrameByIndex(tempFrameIndex) + pageOffset;
        printf("Physical address is %d; Logical address is %d\n", physicalAddress, logicalAddress);
    }

    free(frames);
    return 0;
}

/*

Part 1
======
1. 2 values: 0,1
2. 10 values: 0,1,2,3,4,5,6,7,8,9
3. 2^7 values
4. m^n values
5. 1024*1024 addresses
6. 1024*1024*8 addresses
7. 1024*1024*8 bytes
8. 1024*1024*8*8 bytes

Part 2
======
2. 4 megabytes
*/