#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    if (isPng(file))
    {
        printf("It's a PNG file\n");
    }
    else
    {
        printf("It's not a PNG file\n");
    }

    // When the final chunk is encountered, the traversal finishes
    // if (!isFinished) {
    // Output every chunk type and size
    // Read the length
    // Skip bytes
    // Skip CRC
    // To next chunk
    //}

    fclose(file);

    return EXIT_SUCCESS;
}