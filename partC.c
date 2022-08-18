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

#define MAX_LENGTH 512 // The maximum length command

int RUN_STATUS = 1;  // flag to determine when to exit program
int WAIT_STATUS = 1; // flag to determine if process should run in the background

// This function handles the outward redirection
void rout(char *file){
    int output = open(file, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    dup2(output, 1);
    close(output);
}

// This function handles the inward redirection
void rin(char *file){
    int input = open(file, O_RDONLY);
    dup2(input, 0);
    close(input);
}


int execute(char *args[]){
    // Defining cases for exit and cd commands
    if (strcmp(args[0], "exit") == 0){
        RUN_STATUS = 0;
        return 0;
    }else if (strcmp(args[0], "cd") == 0){
        if (chdir(args[1]) == -1) printf("Directory does not exist\n");
        return 0;
    }
    // Forking a process if the command is not exit or cd
    pid_t pid = fork();
    int status = 0;
    int* exit_code = malloc(sizeof(int));

    // If fork fails
    if (pid < 0) {
        printf("\033[1;31mFailed to fork a child process...\033[0m\n");
        exit(0);
    }
    else if (pid == 0) {
        // Executing the command and storing its status code in the status variable
        status = execvp(args[0], args);
        // If status code is not zero then the system does not support that command
        if (status < 0) {
            printf("Command not found: %s\n",args[0]);
            exit(0);
        }
    }
    else {
        if (WAIT_STATUS) {
            waitpid(pid, exit_code, 0);
        } else {
            WAIT_STATUS = 0;
        }
        
    }
    // Redireting the output to /dev/tty/
    // /dev/tty is a special place where the output is redirected to the terminal
    // For example, echo "hello" > /dev/tty prints hello to the terminal
    rin("/dev/tty");
    rout("/dev/tty");
    return *exit_code; // Returning the exit code of the process
}

// This function handles the piping
void create_pipe(char *args[]) {
    // Creating a pipe file descriptor
    // fd[0] is the read end of the pipe and fd[1] is the write end
    int fd[2];
    pipe(fd);

    dup2(fd[1], 1);
    close(fd[1]);

    execute(args);

    dup2(fd[0], 0);
    close(fd[0]);
}


char* tokenize(char *input) {
    int j = 0;
    // Multiplied by two as a single token can might take a whitespace along with it as well
    char *tokens = malloc((MAX_LENGTH * 2) * sizeof(char));


    for (int i = 0; i < strlen(input); i++) {
        // If the token is a pipe or a redirection symbol
        if (input[i] == '|' || input[i] == '<' || input[i] == '>') {
            tokens[j++] = ' ';
            tokens[j++] = input[i];
            tokens[j++] = ' ';
        }else{
            tokens[j++] = input[i];
        }
    }
    tokens[j] = '\0';

    // Adding null character at the end as malloc does not terminate strings
    *(tokens + strlen(tokens) - 1) = '\0';

    return tokens;
}


int main(void){
    char *args[MAX_LENGTH];
    printf("\033[H\033[J"); // Clearing the screen
    while (RUN_STATUS) {
        // Shell prompt with colors...
        printf("\033[1;34mchaitanya_bisht\033[0m@\033[1;32m12040450\033[0m $ ");
        fflush(stdout);
        
        // Reading the input from the user
        char input[MAX_LENGTH];
        fgets(input, MAX_LENGTH, stdin);
        if (strcmp(input, "\n") == 0) continue;
        // Tokenizing the input
        char * tokens = tokenize(input);

        char *arg = strtok(tokens, " ");
        int i = 0;

        while (arg) {
            // If the token is a pipe
            if (*arg == '|') {
                args[i] = NULL;
                create_pipe(args);
                i = 0;
            // If the token is a && symbol
            }else if (*arg == '&' && *(arg + 1) == '&') {
                int exit_code = execute(args); // Executing the command and storing its exit code
                // If the exit code is not zero then the command did not execute successfully and the remaining commands do not execute
                if (exit_code != 0){
                    exit(0);
                }
                i = 0;
            // If the token is a redirection < symbol
            }else if (*arg == '<'){
                rin(strtok(NULL, " "));
            // If the token is a redirection > symbol
            }else if (*arg == '>'){
                rout(strtok(NULL, " "));
            // EXTRA: If the token is a ; symbol
            }else if (*arg == ';'){
                execute(args);
                i = 0;
            }
            else{
                args[i] = arg;
                i++;
            }
            arg = strtok(NULL, " ");
        }
        args[i] = NULL;
        execute(args);
    }
    return 0;
}
