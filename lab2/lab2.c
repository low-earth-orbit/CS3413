#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <math.h>
#include <ctype.h>

int main()
{
    printf("(%d)Hello, I am the parent process\n", getpid());

    printf("(%d)Creating the child process\n", getpid());

    // create a child process
    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("Fork failed");
        return 1;
    }

    if (child_pid == 0)
    {
        // This is the child process
        printf("(%d)Hello, I am the child process\n", getpid());

        // create a pipe
        int childToGrandchildPipe[2];

        if (pipe(childToGrandchildPipe) == -1)
        {
            perror("Pipe creation failed");
            return 1;
        }

        // child process creates a grandchild process
        printf("(%d)Creating the grandchild process\n", getpid());
        pid_t child_pid = fork();

        if (child_pid == -1)
        {
            perror("Fork failed");
            return 1;
        }

        if (child_pid == 0)
        {
            // This is the grandchild process
            printf("(%d)Hello, I am the grandchild process\n", getpid());

            // in grandchild process, close the write end of childToGrandchild pipe
            close(childToGrandchildPipe[1]);

            // read word from child process
            char word[100];
            if (read(childToGrandchildPipe[0], word, 100 * sizeof(char)) <= 0)
            {
                perror("Read failed");
                return 1;
            }

            // close the read end of childToGrandchild pipe
            close(childToGrandchildPipe[0]);

            // output word is lowercase
            for (int i = 0; word[i]; i++)
            {
                word[i] = tolower(word[i]);
            }

            // print the word
            printf("Output: %s\n", word);

            // exit the grandchild process with status 0
            printf("(%d)Terminating the grandchild process\n", getpid());
            exit(0);
        }
        else
        {
            // This is the child process
            // in child process, close the read end of childToGrandchild pipe
            close(childToGrandchildPipe[0]);

            // scan one word input from user
            char word[100];
            printf("Enter a word: ");
            scanf("%s", word);

            // Output the number the user typed
            printf("Read: %s\n", word);

            // Send word to the grandchild process
            write(childToGrandchildPipe[1], word, 100 * sizeof(char));
            close(childToGrandchildPipe[1]); // after sending word to the grandchild process, close the write end

            // wait for the child process to finish
            wait(NULL);

            printf("(%d)Hello, I am the child process\n", getpid());
        }

        // exit the child process with status 0
        printf("(%d)Terminating the child process\n", getpid());
        exit(0);
    }
    else
    {
        // wait for the child process to finish
        wait(NULL);

        // This is the parent process
        printf("(%d)Hello, I am the parent process\n", getpid());
    }

    printf("(%d)Ending the program / terminating the parent process\n", getpid());
    return 0;
}