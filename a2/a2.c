#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main()
{
    char input[100]; // Assuming a maximum input length of 100 characters

    while (1)
    {
        printf("(%d) Please input your shell command:\n", getpid());
        fgets(input, sizeof(input), stdin); // Read user input

        // Remove newline character if present
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n')
        {
            input[len - 1] = '\0';
        }

        if (strcmp(input, "stop") == 0)
        {
            break; // Exit the loop if the input is "stop"
        }
        else if (strcmp(input, "onechild") == 0)
        {
            pid_t child_pid = fork();

            if (child_pid == -1)
            {
                perror("Fork failed");
                return 1;
            }

            if (child_pid == 0)
            {
                // This is the child process
                printf("(%d)Hello, I am a child process\n", getpid());

                // child process logic
                
                exit(0); // kill the child process
            }
            else
            {
                // This is the parent process
                printf("(%d)Hello, I am a parent process\n", getpid());

                // wait for the child process to finish
                waitpid(child_pid, NULL, 0);
            }
        }
        else
        {
            printf("(%d) You entered: %s\n", getpid(), input);
        }
    }

    return 0;
}
