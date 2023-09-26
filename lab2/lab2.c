#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <math.h>

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

        printf("(%d)Creating the grandchild process\n", getpid());

        // child process creates a grandchild process
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

            // exit the grandchild process with status 0
            printf("(%d)Terminating the grandchild process\n", getpid());
            exit(0);
        }
        else
        {
            // This is the child process

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