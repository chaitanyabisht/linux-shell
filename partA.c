#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LEN 512 // Maximum length of command allowed
int RUN_STATUS = 1;  // Flag to determine if the shell should run or not
int WAIT_STATUS = 1;  // Flag to determine if the process should keep running or not


void execute(char *args[]){

    // Defining cases for exit and cd commmands

    if (strcmp(args[0], "exit") == 0){
        RUN_STATUS = 0;
        return;
    }else if (strcmp(args[0], "cd") == 0){
        if (chdir(args[1]) == -1) printf("Directory does not exist\n");
        return;
    }

    // Forking a process if the command is not exit or cd

    pid_t pid;
    int status = 0;
    pid = fork();

    if (pid < 0) { 
        printf("\033[1;31mFailed to fork a child process...\033[0m\n");
        exit(0);

    } else if (pid == 0) {
        // Executing the command and storing its status code in the status variable
        status = execvp(args[0], args);

        // If status code is not zero then the system does not support that command
        if (status < 0) {
            printf("Command not found: %s\n",args[0]);
            exit(0);
        }
    }
    else {
        // Waiting for the child process to finish
        if (WAIT_STATUS) {
            waitpid(pid, NULL, 0);
        } else {
            WAIT_STATUS = 0;
        }
    }

}


int main(void) {
    char *args[MAX_LEN];
    printf("\033[H\033[J"); // Escape sequence to clear the screen after the shell starts

    // Loop to keep the shell running until the user enters exit or quit
    while (RUN_STATUS) {
        // Shell prompt with colors...
        printf("\033[1;34mchaitanya_bisht\033[0m@\033[1;32m12040450\033[0m $ ");
        fflush(stdout); // Flushing the output buffer

        // Reading the input from the user
        char *input = malloc(MAX_LEN * sizeof(char));
        fgets(input, MAX_LEN, stdin);
	if (strcmp(input, "\n") == 0) continue;
        // Adding null character at the end as malloc does not terminate strings
        *(input + strlen(input) - 1) = '\0';

        // Tokenizing the input by spaces
        char *arg = strtok(input, " ");
        int i = 0;

        // Storing the tokens in the args array
        while (arg) {
            args[i++] = arg;
            arg = strtok(NULL, " ");
        }
        args[i] = NULL;

        // Executing the command
        execute(args);
    }
    return 0;
}
