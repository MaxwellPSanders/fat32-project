/*
 * Name: David Theodore and Maxwell Sanders
 * ID #: 1001122820 and 1001069652
 * Programming assignment 3
 * Description: Fat32 file system reader
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <ctype.h>

char user_input[300]; // user input buffer
char *base_command; // base command (separated to help with finding path)
char *args[10]; // array of pointers to strings holding arguments
int counter, counter2; // counters
char *arg; // buffer to hold current argument being processed while memory allocation in progress
char *paths[4]; // array of pointers to paths 
int opened = 0; // boolean to see if a file is already open
FILE *fp; // file pointer to the fat32
FILE *temp_fp; //file pointer for calls to read
int didsomething = 0; // boolean to see if the command was used

//The actual fat32 variables
char BS_OEMName[8]; // the name of the OEM
int16_t BPB_BytesPerSec; // bytes per sector
int8_t BPB_SecPerClus; // sectors per cluster
int16_t BPB_RsvdSecCnt; // number of reserved sectors in reserved region
int8_t BPB_NumFATs; // number of FAT data structures (should be 2)
int16_t BPB_RootEntCnt; // number of 32 byte directories in the root (should be 0)
char BS_VolLab[11]; // label of the volume
int32_t BPB_FATSz32; // number of sectors contained in one FAT
int32_t BPB_RootClus; // the number of the first cluster of the root directory

int32_t RootDirClusterAddr = 0; // offset location of the root directory
int32_t CurrentDirClusterAddr = 0; // offset location of the directory you are currently in

struct DirectoryEntry {
    char DIR_Name[12];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

struct DirectoryEntry dir[16]; // holds the directory entries for the current directory

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
 * Function: open_file
 * Parameters: the filename of the file
 * Returns: Nothing
 * Description: Tries to open the file
 * If it doesn't open then returns an error statement
 * If it does open then will change opened to 1
 * This will also populate the BPB variables as well as the root directory entry
 */
void open_file(char* filename) {
    //tries to open the file
    fp = fopen(filename, "r");

    //either indicate the file is opened or closed
    if(fp != NULL){
        printf("Opened %s\n", filename);
        opened = 1;

        //grab all of BPB variables
        fseek(fp, 3, SEEK_SET);
        fread(BS_OEMName, 1, 8, fp);
        fread(&BPB_BytesPerSec, 1, 2, fp);
        fread(&BPB_SecPerClus, 1, 1, fp);
        fread(&BPB_RsvdSecCnt, 1, 2, fp);
        fread(&BPB_NumFATs, 1, 1, fp);
        fread(&BPB_RootEntCnt, 1, 2, fp);
        fseek(fp, 36, SEEK_SET);
        fread(&BPB_FATSz32, 1, 4, fp);
        fseek(fp, 44, SEEK_SET);
        fread(&BPB_RootClus, 1, 4, fp);
        fseek(fp, 71, SEEK_SET);
        fread(BS_VolLab, 1, 11, fp); 

        //calculate the root directory location
        RootDirClusterAddr = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) +
                             (BPB_RsvdSecCnt * BPB_BytesPerSec);

        //start off in the root directory
        CurrentDirClusterAddr = RootDirClusterAddr;

        //fill dir by going to the address and reading in the data to structs
        fseek(fp, RootDirClusterAddr, SEEK_SET);
        for(counter = 0; counter < 16; counter ++){
             fread(dir[counter].DIR_Name, 1, 11, fp);
             fread(&dir[counter].DIR_Attr, 1, 1, fp);
             fread(&dir[counter].Unused1, 1, 8, fp);
             fread(&dir[counter].DIR_FirstClusterHigh, 2, 1, fp);
             fread(&dir[counter].Unused2, 1, 4, fp);
             fread(&dir[counter].DIR_FirstClusterLow, 2, 1, fp);
             fread(&dir[counter].DIR_FileSize, 4, 1, fp);
        }
    }
    else
        printf("Could not locate file\n");

}

/* 
 * Function: display_info
 * Parameters: None
 * Returns: Nothing
 * Description: Will display the filesystem info
 * If a file is not opened then it will display 
 * an error message instead
 */
void display_info() {
    if(!opened)
        printf("There is no file open!\n");
    else{
        //lines commented out are for debug only
        //printf("OEMName : %s\n", BS_OEMName);
        printf("Bytes Per Sector:\n    Hex - %x\n    Decimal - %d\n", 
                BPB_BytesPerSec, BPB_BytesPerSec);
        printf("Sectors Per Clusters:\n    Hex - %x\n    Decimal - %d\n", 
                BPB_SecPerClus, BPB_SecPerClus);
        printf("Reserved Sector Count:\n    Hex - %x\n    Decimal - %d\n", 
                BPB_RsvdSecCnt, BPB_RsvdSecCnt);
        printf("Number of FAT regions:\n    Hex - %x\n    Decimal - %d\n", 
                BPB_NumFATs, BPB_NumFATs);
        //printf("Root Entry Count : %d\n", BPB_RootEntCnt);
        printf("Sectors per FAT:\n    Hex - %x\n    Decimal - %d\n", 
                BPB_FATSz32, BPB_FATSz32);
        //next line won't print since the value of VolLab is 02, 11 times 
        //printf("Volume Label : %s\n", BS_VolLab);
        //printf("First Cluster of the Root Directory : %d\n", BPB_RootClus);

    } 
}

/*
 * Function: show_stat
 * Parameters: None
 * Returns: nothing
 * Description: Will display attributes, starting cluster number, and size of
 * a specified filename or directory.
 * If the file/directory can't be found then it will pop an error
 */
void show_stat(){
    char namebuffer[20]; // string for the name
    char extbuffer[4]; // string for the extension
    char statbuffer[20]; // string that will become the concatenation of the previous two
    char *token; // token for the string to strtok into
    int whitespace; // the offset white space
    int found = 0; // boolean to see if an entry was found
    int i = 0;

    //if the filesytem is open then do the process
    //otherwise notify the user
    if(opened){
        //the max name length could only be 8 + 1 + 3
        if(strlen(args[0]) < 13){
            //copy the first part of the string, delimited by .
            strcpy(namebuffer, strtok(args[0], ".\n"));
            //grab the second part of the string after the .
            token = strtok(NULL, "\n");
            //if the token didn't receive anything then check if the file is a directory
            //otherwise check if it a valid file
            if(token == NULL && strlen(namebuffer) < 12){
                //uppercase everything
                for(counter = 0; counter < 11; counter ++){
                    namebuffer[counter] = toupper(namebuffer[counter]);
                }
                //add spaces
                whitespace = 11 - strlen(namebuffer);
                //concat whitespace
                for(counter = 0; counter < whitespace; counter ++){
                    strcat(namebuffer, " ");
                }
                //check through the directory
                found = 0;
                for(counter = 0; counter < 16; counter ++){
                    //if you find the entry then break out
                    if(!strcmp(dir[counter].DIR_Name, namebuffer)){
                        //print attribute, starting cluster number, and size
                        printf("Attribute: %x\n", dir[counter].DIR_Attr);
                        printf("Starting cluster number:\n    Hex - %x\n    Decimal - %d\n", 
                                   dir[counter].DIR_FirstClusterLow, dir[counter].DIR_FirstClusterLow);
                        //becuase it is a directory
                        printf("Size: 0 Bytes\n"); 
                        found = 1;
                        break;
                    }
                }    
                if(!found)
                    printf("Couldn't find the directory\n");
            }
            else if(strlen(token) > 3){
                printf("File extension too long (3 characters)\n");
            }
            else if(strlen(namebuffer) > 8){
                printf("File name too long (8 characters).\n");
            }
            else{
                //add spaces
                whitespace = 8 - strlen(namebuffer);
                //concat
                //uppercase everything
                //check through directory   
            }
        }
        else
            printf("That is an invalid file or directory name\n");
    }
    else
        printf("There is no filesystem open.\n");
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
    arg = malloc(200);
    while (1) {

        //nothing is done from the start
        didsomething = 0;
        
        // print shell input ready
        printf("mfs>");

        // get user input 
        get_user_input();

        // load up first token of user input for memory allocation 
        arg = strtok(user_input, " ");
 
        // allocate memory for base command
        base_command = malloc(sizeof(arg)+1);
        base_command = arg;

        // set up counter to count tokens
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
       	if(!strcmp(base_command, "exit") || !strcmp(base_command, "quit")){
            if(opened)
                fclose(fp);
            exit (0);
        }
	
        // open the file or tell them a file is already open
        if(!strcmp(base_command, "open")){
            //if a file isn't opened try to open a file
            if(!opened)
                open_file(args[0]);
            else
                printf("There is already a file open\n");
            didsomething ++;
        }
 
        // close the file or tell them no files are open
        if(!strcmp(base_command, "close")){
            //if a file is opened close it
            if(opened){
                fclose(fp);
                printf("Closed file\n");	
                opened = 0;
            }
            else
                printf("There is no files open\n");
            didsomething ++;
        }

        // print out some of the stats inside the file
        if(!strcmp(base_command, "info")){
            display_info();
            didsomething ++;
        }

        // print out some of the stats inside the file
        if(!strcmp(base_command, "stat") && args[1] == NULL){
            show_stat();
            didsomething ++;
        }

        //if the base command is something and not used 
        if(strcmp(base_command, "\n") && !didsomething)
            printf("That isn't a valid command\n");

    }
    exit(0);
}
