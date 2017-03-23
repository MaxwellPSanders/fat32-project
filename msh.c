/*
 * Name: David Theodore
 * ID #: 1001122820
 * Programming Assignment 1
 * Description: Mav Shell
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

char user_input[300]; // user input buffer
char *base_command; // base command (separated to help with finding path)
char *args[10]; // array of pointers to strings holding arguments
int counter, counter2; // counters
char *arg; // buffer to hlod current argument being processed while memory allocation in progress
char *paths[4]; // array of pointers to paths 
int pids[10]; //array of ints holding last 10 PIDs (circular FIFO)
int pids_pointer = 0; // PID FIFO location pointer

/* 
 * Function: get_user_input
 * Parameters: None
 * Returns: Nothing
 * Description: Gets input from the user and removes the
 * trailing newline character
 */
void get_user_input() {
    // get user input from standard in
    fgets(user_input, 200, stdin );
    // remove new line char unless the command is empty
    if(strlen(user_input) != 1)
        user_input[strcspn(user_input, "\n")] = 0;
}

/* 
 * Function: cntl_c_handler
 * Parameters: int signal - not completely needed but I would
 * to add onto this later on
 * Returns: Nothing
 * Description: Handles signal event where the user pressed control-c
 * and directs them to use exit or quit to leave the msh
 */
void cntl_c_handler(int signal) {
    // explain proper exit procedure to user
    printf("\nPlease user \"exit\" or \"quit\" to exit the mav shell\nmsh>");
    fflush(stdout);
}

/* 
 * Function: cntl_z_handler
 * Parameters: int signal - not completely needed but I would
 * to add onto this later on
 * Returns: Nothing
 * Description: Handles signal event where the user pressed control-z
 * and directs them to use exit or quit to leave the msh
 */
void cntl_z_handler(int signal) {
    // explain proper exit procedure to user
    printf("\nPlease user \"exit\" or \"quit\" to exit the mav shell\nmsh>");
    fflush(stdout);
}

/* 
 * Function: execute_command
 * Parameters: None
 * Returns: Nothing
 * Description: handles all commands that wil be passed to the system
 * by finding the location fo the command or letting the user know that
 * the command was not found. Also handles the special case where the
 * user command is "cd" and executes a change of directory in chdir()
 */
int execute_command(void) {

    // create a buffer to stor the local current working directory
    char *cwd_buff;
    cwd_buff = malloc(100);

    // remove trailing newline from base_command if it exists   
    base_command[strcspn(base_command, "\n")] = 0;
    
    // allocate memory based on the size of the path that will be created
    // (length of the base command plus pre-path)
    for(counter=0; counter <= 3; counter++)
        paths[counter] = malloc(21 + strlen(base_command));

    // get current working directory
    getcwd(cwd_buff, 100);
    strcat(cwd_buff, "/");

    // populate supported paths to commands
    strcpy(paths[0], cwd_buff);
    strcpy(paths[1], "/usr/local/bin/");
    strcpy(paths[2], "/usr/bin/");
    strcpy(paths[3], "/bin/");
    
    // add base command to path names for use with execl()
    for(counter=0; counter <= 3; counter++) {
        strcat(paths[counter], base_command);
    }

    // create a pid and path number
    pid_t pid;
    int path = -1;

    // find the command in one of the supported paths
    for(counter = 0; counter <= 3 && path == -1; counter++) {
        if(access(paths[counter], F_OK) != -1)
            path = counter; 
    }

    // let the user know if the path was not found
    if(path == -1) {
        printf("%s: Command not found.\n", base_command);
        return 0;
    }
    
    // create a status int for waitpid to watch    
    int command_status;

    // fork a process
    pid = fork();

    // if child then execute command with args
    if(pid == 0) {
        execl(paths[path],
              base_command,
              args[0],
              args[1],
              args[2],
              args[3],
              args[4],
              args[5],
              args[6],
              args[7],
              args[8],
              args[9],
              NULL );
        exit( EXIT_SUCCESS );
    }

    // if parent then record pid in 10-wide pid FIFO
    else {
        pids[pids_pointer%10] = pid;
        pids_pointer++;
    }

    // wait for child to finish running
    waitpid(pid, &command_status, 0);
    return 0;
}

/* 
 * Function: main
 * Parameters: None
 * Returns: int (return code)
 * Description: Initializes signal for control-c and control-z and
 * then begins an infinite while loop to parse user input and decide
 * what to do with the input. Calls execute command when nessesary 
 * or exits.
 */
int main(void) {
    // set up signal catching for control-c and control-z
    signal(SIGINT, cntl_c_handler);
    signal(SIGTSTP, cntl_z_handler);

    // allocate memory for current argument buffer
    arg = malloc(100);
    while (1) {
        
        // print shell input ready
        printf("msh>");

        // get user input 
        get_user_input();

        // load up first token of user input for memory allocation 
        arg = strtok(user_input, " ");
 
        // allocate memory for base command
        base_command = malloc(sizeof(arg)+1);
        base_command = arg;

        // set up counter to count tolkens
        counter = 0;
        do { 
            arg = strtok(NULL, " ");
           
            // allocate memory for arguments
            args[counter] = malloc(sizeof(arg)+1);
            args[counter] = arg;
            counter++;
        // loop until no more arguments
        } while(arg != NULL);

        // handle all exits
        if(!strcmp(base_command, "exit") || !strcmp(base_command, "quit"))
            exit (0);

        // if user asks for PIDs showthe last 10 PIDs in PID FIFO
        else if(!strcmp(base_command, "showpid")) {
            for (counter = 0; counter < 10; counter++) {
                if (pids[(pids_pointer + counter)% 10] != 0)
                    printf("%d\n", pids[(pids_pointer + counter)% 10] );
            }
        }

        // if base command is cd then use chdir()
        else if(!strcmp(base_command, "cd"))
            chdir(args[0]);

        // all other commands pass to execute command
        else 
            execute_command();
    }
    exit(0);
}
