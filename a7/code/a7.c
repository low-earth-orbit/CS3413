#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define OFFSETBITS 12
#define PAGESIZE (1 << OFFSETBITS)
#define PAGETABLEBITS 10
#define ARCH 32
int framesSize;
unsigned int pageFault = 0;
unsigned int pageHits = 0;
unsigned int pagesSwapped = 0;

void printPhysicalAddress(int frameID, unsigned long logicalAddress)
{
    printf("%lu -> %i\n", logicalAddress, frameID * PAGESIZE + getPageOffsetAddress(logicalAddress));
}

double computeFormula()
{
    // TODO work on that
    return 0.0f;
}

unsigned long getPageOffsetAddress(unsigned long logicalAddress)
{
    return logicalAddress % PAGESIZE;
}

void readFromAddress(unsigned long logicalAddress)
{
}

void writeToAddress(unsigned long logicalAddress)
{
}

int main(int argc, char **argv)
{
    framesSize = atoi(argv[1]);
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
    return 0;
}