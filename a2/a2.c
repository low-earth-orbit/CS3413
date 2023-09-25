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

                // exit the child process with status 0
                exit(0);
            }
            else
            {
                // This is the parent process
                printf("(%d)Hello, I am a parent process\n", getpid());

                // wait for the child process to finish
                // to be more specific, I can use: waitpid(child_pid, NULL, 0);
                wait(NULL);
            }
        }
        else if (strcmp(input, "addnumbers") == 0)
        {
            // create two pipes
            int parentToChildPipe[2];
            int childToParentPipe[2];

            if (pipe(parentToChildPipe) == -1 || pipe(childToParentPipe) == -1)
            {
                perror("Pipe creation failed");
                return 1;
            }

            // fork a child process
            pid_t child_pid = fork();

            if (child_pid == -1)
            {
                perror("Fork failed");
                return 1;
            }

            if (child_pid == 0)
            {
                // This is the child process
                close(parentToChildPipe[1]); // Close the write end of the parentToChildPipe
                close(childToParentPipe[0]); // Close the read end of the childToParentPipe

                int sum = 0;
                int num;

                while (1)
                {
                    if (read(parentToChildPipe[0], &num, sizeof(int)) <= 0)
                    {
                        perror("Read failed");
                        close(parentToChildPipe[0]);
                        close(childToParentPipe[1]);
                        return 1;
                    }

                    sum += num;
                    printf("(%d)The subtotal is: %d\n", getpid(), sum);

                    // if user entered 0, end the read process
                    if (num == 0)
                    {
                        // Close the read end of the parent to child pipe
                        close(parentToChildPipe[0]);

                        // Send the sum to the parent process
                        write(childToParentPipe[1], &sum, sizeof(int));

                        // Close the write end of the child to parent pipe
                        close(childToParentPipe[1]);

                        // Terminate the child process
                        exit(0);
                    }
                }
            }
            else
            {
                // This is the parent process
                close(parentToChildPipe[0]); // Close the read end of the parentToChildPipe
                close(childToParentPipe[1]); // Close the write end of the childToParentPipe

                int num;

                while (1)
                {
                    printf("(%d)Please input number, (0) to terminate\n", getpid());
                    fgets(input, sizeof(input), stdin);
                    sscanf(input, "%d", &num);

                    // print number read
                    printf("(%d)Read  %d\n", getpid(), num);

                    // Send numbers to the child process
                    write(parentToChildPipe[1], &num, sizeof(int));

                    if (num == 0)
                    {
                        // Close the write end of the pipe
                        close(parentToChildPipe[1]);

                        // Wait for the child process to finish
                        wait(NULL);

                        // Receive the sum from the child process
                        int sum;
                        read(childToParentPipe[0], &sum, sizeof(int));
                        printf("(%d)The sum is %d\n", getpid(), sum);

                        // Close the read end of the childToParentPipe
                        close(childToParentPipe[0]);

                        break; // Exit the loop if zero is encountered
                    }
                }
            }
        }
        else
        {
            printf("(%d) You entered: %s\n", getpid(), input);
        }
    }

    return 0;
}
