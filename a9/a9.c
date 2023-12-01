#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Check if it is PNG file base on the header
int isPng(FILE *file)
{
    unsigned char pngSignature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char buffer[8];

    if (fread(buffer, 1, 8, file) != 8)
    {
        return 0;
    }

    return memcmp(buffer, pngSignature, 8) == 0;
}

bool isTraverseFinished = false;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file)
    {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    // First 8 bytes contain the PNG signature
    if (isPng(file))
    {
        printf("It's a PNG file\n");
    }
    else
    {
        printf("It's not a PNG file\n");
    }

    if (!isTraverseFinished)
    {

        uint32_t lengthOfChunk;
        char chunkType[5]; // 4 characters + null terminator

        // Read the length of the chunk. 4 bytes. big-endian
        if (fread(&lengthOfChunk, 1, 4, file) != 4)
        {
            perror("Unable to read file");
            return EXIT_FAILURE;
        }

        // Convert to little-endian using endianness converter
        lengthOfChunk = ntohl(lengthOfChunk);

        // Read the chunk type (4 bytes)
        if (fread(chunkType, 1, 4, file) != 4)
        {
            perror("Unable to read file");
            return EXIT_FAILURE;
        }

        // Add null-terminator
        chunkType[4] = '\0';

        // Output chunk type and size
        printf("Found unknown: %s\n", chunkType);
        printf("Chunk size is:%d\n", lengthOfChunk);

        // Skip bytes
        fseek(file, lengthOfChunk, SEEK_CUR);

        // Skip CRC (4 bytes)
        fseek(file, 4, SEEK_CUR);

        // To next chunk
        // When the final chunk is encountered, the traversal finishes
    }

    fclose(file);

    return EXIT_SUCCESS;
}