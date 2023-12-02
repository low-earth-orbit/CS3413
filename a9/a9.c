#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Check if it is PNG file base on the header
int isPng(FILE *file)
{
    unsigned char pngSignature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    unsigned char buffer[8];

    if (fread(buffer, 1, 8, file) != 8)
    {
        return 0;
    }

    return memcmp(buffer, pngSignature, 8) == 0;
}

void xorChunkData(unsigned char *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        data[i] ^= 42; // XOR each byte with 42
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r+"); // Opens a file to update both reading and writing. The file must exist.
    if (!file)
    {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }

    // First 8 bytes contain the PNG signature
    if (isPng(file))
    {
        printf("It's a PNG file\n");

        bool isTraverseFinished = false;

        while (!isTraverseFinished)
        {
            uint32_t lengthOfChunk;
            char chunkType[5]; // 4 characters + null-terminator
            long dataStartPosition;

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
            if (strcmp(chunkType, "IEND") == 0)
            {
                printf("Found %s\n", chunkType);
                isTraverseFinished = true; // When the final chunk is encountered, the traversal finishes
            }
            else if (strcmp(chunkType, "IDAT") == 0)
            {
                printf("Found %s\n", chunkType);
            }
            else
            {
                printf("Found unknown: %s\n", chunkType);
            }

            printf("Chunk size is:%d\n", lengthOfChunk);

            // If type is IDAT, XOR every byte by 42
            if (strcmp(chunkType, "IDAT") == 0)
            {
                // Save the start position of the chunk data
                dataStartPosition = ftell(file);

                // Allocate memory for a buffer
                unsigned char *buffer = malloc(lengthOfChunk);
                if (!buffer)
                {
                    perror("Failed to allocate memory for buffer\n");
                    return EXIT_FAILURE;
                }

                // Read the chunk data to the buffer
                // This moves file pointer
                if (fread(buffer, 1, lengthOfChunk, file) != lengthOfChunk)
                {
                    free(buffer);
                    perror("Unable to read data buffer\n");
                    return EXIT_FAILURE;
                }

                // XOR the buffer by 42
                xorChunkData(buffer, lengthOfChunk);

                // Move file pointer back to start position of the chunk data
                if (fseek(file, dataStartPosition, SEEK_SET) != 0)
                {
                    free(buffer);
                    perror("fseek failed\n");
                    return EXIT_FAILURE;
                }

                // Write the buffer to the file
                if (fwrite(buffer, 1, lengthOfChunk, file) != lengthOfChunk)
                {
                    free(buffer);
                    perror("fwrite failed\n");
                    return EXIT_FAILURE;
                }

                // Free the data buffer
                free(buffer);
            }
            else
            {
                // Skip data bytes if type is not IDAT
                if (fseek(file, lengthOfChunk, SEEK_CUR) != 0)
                {
                    perror("fseek failed\n");
                    return EXIT_FAILURE;
                }
            }

            // Skip CRC (4 bytes)
            if (fseek(file, 4, SEEK_CUR) != 0)
            {
                perror("fseek failed\n");
                return EXIT_FAILURE;
            }
        }
    }
    else
    {
        printf("It's not a PNG file\n");
    }
    fclose(file);

    return EXIT_SUCCESS;
}