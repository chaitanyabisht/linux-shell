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

#define MAX_LENGTH 512 // Maximum length of command allowed

int RUN_STATUS = 1; // Flag to determine if the shell should run or not
int WAIT_STATUS = 1; // Flag to determine if the process should keep running or not

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
    // Defining cases for exit and cd commmands
    if (strcmp(args[0], "exit") == 0){
        RUN_STATUS = 0;
        return 0;
    }else if (strcmp(args[0], "cd") == 0){
        if (chdir(args[1]) == -1) {
            printf("Directory does not exist\n");
            return 1;
        }
        return 0;
    }

    // Forking a process if the command is not exit or cd
    pid_t pid = fork();
    int status = 0;
    int* exit_code = malloc(sizeof(int));

    if (pid < 0) { 
        printf("\033[1;31mFailed to fork a child process...\033[0m\n");
        exit(0);
    }
    else if (pid == 0) {
        // Executing the command and storing its status code in the status variable
        status = execvp(args[0], args);
        if (status < 0) {
            // If status code is not zero then the system does not support that command
            printf("Command not found: %s\n",args[0]);
            exit(0);
        }
    }
    else {
        // Waiting for the child process to finish
        if (WAIT_STATUS) {
            waitpid(pid, exit_code, 0);
        } else {
            WAIT_STATUS = 0;
        }
        
    }
    
    rin("/dev/tty");
    rout("/dev/tty");

    // Returning the exit code of the process
    return *exit_code;
}

// This function handles the piping 
void create_pipe(char *args[]) {
    // Creating a pipe file descriptor
    // fd[0] is for reading and fd[1] is for writing
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
    // Creating a buffer to store the tokenized input, MAX_LENGTH*2 is due to the fact that we store extra whitespaces in the buffer
    char *tokens = malloc((MAX_LENGTH * 2) * sizeof(char));


    for (int i = 0; i < strlen(input); i++) {
        // Adding extra whitespaces to the buffer if there is a pipe character in the input
        if (input[i] == '|') {
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
    
    printf("\033[H\033[J"); /// Clear the screen

    // Loop to keep the shell running
    while (RUN_STATUS) {
        // Shell prompt with colors...
        printf("\033[1;34mchaitanya_bisht\033[0m@\033[1;32m12040450\033[0m $ ");
        fflush(stdout); // Flushing the buffer
        
        // Reading the input from the user
        char input[MAX_LENGTH];
        fgets(input, MAX_LENGTH, stdin);
	if (strcmp(input, "\n") == 0) continue;
        // Tokenizing the input
        char *tokens = tokenize(input);
        char *arg = strtok(tokens, " ");

        int i = 0;
        // Looping through the tokens and storing them in the args array
        while (arg) {
            // Checking if the token is a pipe
            if (*arg == '|') {
                args[i] = NULL;
                create_pipe(args);
                i = 0;
            
            // Checking if the token is &&
            }else if (*arg == '&' && *(arg + 1) == '&') {

                int exit_code = execute(args); // Executing the command and storing the exit code of the process in variable "exit_code"
                // Terminate the processes if the exit code of previous process is not zero
                if (exit_code != 0){
                    break;
                }
                i = 0;
            // Checking if the token is ;
            } else if (*arg == ';'){
                execute(args);
                i = 0;
            }
            else {
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
