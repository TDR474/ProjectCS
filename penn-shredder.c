#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_SIZE 1024

pid_t childPid = 0;

void executeShell(int timeout);

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char *getCommandFromInput();

void registerSignalHandlers();

void killChildProcess();

int main(int argc, char **argv) {
    registerSignalHandlers();

    int timeout = 0;
    if (argc == 2) {
        timeout = atoi(argv[1]);
    }

    if (timeout < 0) {
        writeToStdout("Invalid input detected. Ignoring timeout value.\n");
        timeout = 0;
    }


    while (1) {
        executeShell(timeout);
    }



    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
void killChildProcess() {
    /* terminate the child process correctly so that wait(&status) != -1 */
    if (kill(childPid, SIGKILL) == -1) {
            perror("Error in kill");
            exit(EXIT_FAILURE);
        }
}

/* Signal handler for SIGALRM. Catches SIGALRM signal and
 * kills the child process if it exists and is still executing.
 * It then prints out penn-shredder's catchphrase to standard output */
void alarmHandler(int sig) {
    signal(SIGALRM, sigintHandler);
    /* prints to standard output */
    writeToStdout("Bwahaha ... tonight I dine on turtle soup");
    if (childPid != 0) {
        killChildProcess();
    }
    /* sends sigkill to child process */
}

/* Signal handler for SIGINT. Catches SIGINT signal (e.g. Ctrl + C) and
 * kills the child process if it exists and is executing. Does not
 * do anything to the parent process and its execution */
void sigintHandler(int sig) {
    if (childPid != 0) {
        killChildProcess();
    }
}


/* Registers SIGALRM and SIGINT handlers with corresponding functions.
 * Error checks for signal system call failure and exits program if
 * there is an error */
void registerSignalHandlers() {
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("Error in signal");
        exit(EXIT_FAILURE);
    }
}

/* Prints the shell prompt and waits for input from user.
 * Takes timeout as an argument and starts an alarm of that timeout period
 * if there is a valid command. It then creates a child process which
 * executes the command with its arguments.
 *
 * The parent process waits for the child. On unsuccessful completion,
 * it exits the shell. */
void executeShell(int timeout) {
    char *command;
    int status;
    char minishell[] = "penn-shredder# ";
    writeToStdout(minishell);

    command = getCommandFromInput();

    if (command != NULL) {
        childPid = fork();

        if (childPid < 0) {
            perror("Error in creating child process");
            exit(EXIT_FAILURE);
        }

        if (childPid == 0) {
            char *const envVariables[] = {NULL};
            char *const args[] = {command, NULL};  
            execve(command, args, envVariables);
            if (execve(command, args, envVariables) == -1) {
                perror("Error in execve");
                exit(EXIT_FAILURE);
            }
            
            

        } else {
            /* Sets child pid to 0 */
            /* start a new line before exit */

            do {
                if (wait(&status) == -1) {
                    perror("Error in child process termination");
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            wait(NULL);
            /* handles SIGint */
            signal(SIGINT, sigintHandler);
            childPid = 0;
            
        }
    }
}





/* Writes particular text to standard output */
void writeToStdout(char *text) {
    if (write(STDOUT_FILENO, text, strlen(text)) == -1) {
        perror("Error in write");
        exit(EXIT_FAILURE);
    }
}

/* reads particular text to standard input */
void readFromStdin(char *text) {
    if (read(STDIN_FILENO, text, INPUT_SIZE) == -1) {
        perror("Error in read");
        exit(EXIT_FAILURE);
    }
}

/* Reads input from standard input till it reaches a new line character.
 * Checks if EOF (Ctrl + D) is being read and exits penn-shredder if that is the case
 * Otherwise, it checks for a valid input and adds the characters to an input buffer.
 *
 * From this input buffer, the first 1023 characters (if more than 1023) or the whole
 * buffer are assigned to command and returned. An \0 is appended to the command so
 * that it is null terminated */

void parseCommand(char *input, char *cmd){
	//parse cmd
	//remove spaces and tabs
	while(*input == ' ' || *input == '\t'){ input++; }

	while(*input != ' ' && *input != '\0' && *input != '\n'){
		*cmd++ = *input++;
	}
	//set end of cmd
	*cmd = '\0';
}


char *getCommandFromInput() {


    // allocate space for input buffer
    char* buffer = (char*)calloc(INPUT_SIZE, sizeof(char));
    if (buffer == NULL) {
        perror("Error in calloc");
        free(buffer);
        exit(EXIT_FAILURE);
    }
    int numbytes = read(STDIN_FILENO, buffer, INPUT_SIZE);

    // allocate space for command
    char* command = (char*)calloc(INPUT_SIZE, sizeof(char));


    //handle EOF
    if (numbytes == 0){
        free(buffer);
        free(command);
        exit(0);
    }
    else if (numbytes == -1){
        perror("Unable to read input.");
    }

    // if command is empty, repromt
    if (strcmp(buffer, "\n") == 0) {
        free(command);
        free(buffer);
        return NULL;
    }


    int i = 0;
    while (buffer[i] != '\n' && buffer[i] != '\0') {
        command[i] = buffer[i];
        i++;
    }
    command[i] = '\0';

    // remove leading spaces
    int k = 0;
    while (command[k] == ' ') {
        k++;
    }
    char* command2 = command + k;


    // if command is only a null terminator, reprompt
    if (strcmp(command, "\0") == 0) {
        free(command);
        free(buffer);
        return NULL;
    }
    free(buffer);

    //if command is all spaces, reprompt
    int j = 0;
    if (command[j] == ' '){
        while (command[j] == ' '){
            j++;
        }
        if (command[j] == '\0'){
            free(command);
            return NULL;
        }
    }

    // If there are trailing spaces, remove them and return
    int len = strlen(command2);
    while (command2[len-1] == ' '){
        command2[len-1] = '\0';
        len--;
    }
    return command2;


}



