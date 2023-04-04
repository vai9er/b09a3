#include "commonLibs.h"
#include "stats_functions.c"
#include "p.c"

void handle_SIGTSTP(int signum){}

void handle_SIGINT(int signum){
    char response;
    printf("\nDo you want to quit? [y/n] ");
    scanf(" %c", &response);
    if (response == 'y' || response == 'Y'){
        // user wants to quit, terminate the program
        exit(EXIT_SUCCESS);
    }
}

//helper function that checks whether a CHAR* is an int
//if it is, return 1, else return 0
int isInt(char *str){
    int i = 0;
    while (str[i] != '\0'){
        if (isdigit(str[i]) == 0){
            return 0;
        }
        i++;
    }
    return 1;
}

//this function parses the command line arguments. It takes in the argc and argv from main, and the pointers to the variables that will be changed.
//it will change the variables to the values specified by the user, or to the default values if the user did not specify a value
//it will also check for invalid arguments and print an error message if it finds one
void parseArgs(int argc, char** argv, int* systemm, int* user, int* sequential, int* samples, int* tdelay, int* graphics) {
    int positionalArgIndex = 0;
    for (int i = 1; i < argc; i++) {
        //here we check for the system and user flags
        if (strcmp(argv[i], "--system") == 0) {
            *systemm = 1;
        } 
        else if (strcmp(argv[i], "--user") == 0) {
            *user = 1;
        } 
        //check for the sequential flag
        else if (strcmp(argv[i], "--sequential") == 0) {
            *sequential = 1;
        }

        else if (strcmp(argv[i], "--graphics") == 0) {
            *graphics = 1;
        }

        //check for the samples and tdelay flags
        else if (strncmp(argv[i], "--samples=", 10) == 0) {
            *samples = atoi(argv[i] + 10);
            //here we check if the user specified a value for samples that is less than 1
            if (*tdelay <1){
                printf("Invalid arguement: %s\n", argv[i]);
                exit(1);
            }
            //here we check if the user specified a value for samples that is not an int
            if (isInt(argv[i] + 10) == 0){
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }

        }
        //here we check if the user specified a value for tdelay that is less than 1 
         else if (strncmp(argv[i], "--tdelay=", 9) == 0) {
            *tdelay = atoi(argv[i] + 9);
            //here we check if the user specified a value for tdelay that is less than 1
            if (*tdelay <1){
                printf("Invalid arguement: %s\n", argv[i]);
                exit(1);
            }
            //here we check if the user specified a value for tdelay that is not an int
            if (isInt(argv[i] + 9) == 0){
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }
        } 
        //here we check for positional arguments
        else if (positionalArgIndex == 0) {
            int value = atoi(argv[i]);
            if (value <= 0) {
                //here we check if the user specified a value for samples that is less than 1
                printf("Invalid positional argument: %s\n", argv[i]);
                exit(1);
            }
            if (isInt(argv[i]) == 0){
                //here we check if the user specified a value for samples that is not an int
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }
            *samples = value;
            positionalArgIndex++;
        } 
        //here we check for positional arguments
        else if (positionalArgIndex == 1) {
            int value = atoi(argv[i]);
            if (value <= 0) {
                //here we check if the user specified a value for tdelay that is less than 1
                printf("Invalid positional argument: %s\n", argv[i]);
                exit(1);
            }
            //here we check if the user specified a value for tdelay that is not an int
            if (isInt(argv[i]) == 0){
                printf("Invalid argument: %s -- not an int\n", argv[i]);
                exit(1);
            }
            *tdelay = value;
            positionalArgIndex++;
        } else {
            printf("Invalid argument: %s\n", argv[i]);
            exit(1);
        }
    }
    //here we check if the user specified the correct number of positional arguments
    if (positionalArgIndex != 0 && positionalArgIndex != 2) {
        printf("Two positional arguments are required, but %d were provided\n", positionalArgIndex);
        exit(1);
    }
}

//clears the screen with ANSI escape codes
// void clear_screen() {
//   printf("\033[2J");  // clear entire screen
//   printf("\033[%d;%dH", 0, 0);  // move cursor to the top-left corner
// }

// //function to print the menu
// void seqFlag(int samples, int tdelay){
//     int j = 0;
//     for (int i = 0; i < samples; i++) {
//         printf(">>> ITERATION %d\n", i+1);
//         printf("Number of samples: %d -- every %d secs\n", samples, tdelay);

//         //step 1: get system information
//         struct sysinfo systemInfo;
//         sysinfo(&systemInfo);
    
//         // Get total and used memory
//         float memory_total = systemInfo.totalram;
//         float memory_used = systemInfo.totalram - systemInfo.freeram;

//         // Print memory usage in kilobytes
//         printf("Memory usage: %f kilobytes\n", memory_used / 1024);
//         printf("---------------------------------------\n");
//         printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");


//         for (j = 0; j < i; j++){
//             printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", memory_used / (1024 * 1024 * 1024), memory_total / (1024 * 1024 * 1024), memory_used / (1024 * 1024 * 1024), (memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
//         }
//         printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", memory_used / (1024 * 1024 * 1024), memory_total / (1024 * 1024 * 1024), memory_used / (1024 * 1024 * 1024), (memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
//         for (j = i; j < 9; j++){
//             printf("\n");
//         }
//         printUsers();
//         logCores();
//         printMachineInfo();
//         logCpuUsage();
//         sleep(tdelay);
//         printf("\n");
//     }
// }

// void refresh(int samples, int tdelay){
//     clear_screen();
//     int j = 0;
//     for (int i = 0; i < samples; i++) {
//         printf("Number of samples: %d -- every %d secs\n", samples, tdelay);
        
//         //step 1: get system information
//         struct sysinfo systemInfo;
//         sysinfo(&systemInfo);
    
//         // Get total and used memory
//         float memory_total = systemInfo.totalram;
//         float memory_used = systemInfo.totalram - systemInfo.freeram;

//         // Print memory usage in kilobytes
//         printf("Memory usage: %.0f kilobytes\n", (memory_used / 1024)/1024);
//         printf("---------------------------------------\n");
//         printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

//         // Print memory usage in kilobytes before
//         for (j = 0; j < i; j++){
//             printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", memory_used / (1024 * 1024 * 1024), memory_total / (1024 * 1024 * 1024), memory_used / (1024 * 1024 * 1024), (memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
//         }
//         // Print current memory usage in kilobytes
//         printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", memory_used / (1024 * 1024 * 1024), memory_total / (1024 * 1024 * 1024), memory_used / (1024 * 1024 * 1024), (memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));

//         // Print memory usage in kilobytes after
//         for (j = i; j < 9; j++){
//             printf("\n");
//         }
//         //print the rest of the information
//         printUsers();
//         logCores();
//         printMachineInfo();
//         logCpuUsage();
//         clear_screen();
        
//         //gn
//         sleep(tdelay);
//     }
// }

int main(int argc, char** argv){
    //default values
    int systemm = 0;
    int user = 0;
    int samples = 10;
    int tdelay = 1;
    int sequential = 0;
    int graphics = 0;
    
    //parse the arguments
    parseArgs(argc, argv, &systemm, &user, &sequential, &samples, &tdelay, &graphics);

    //--system
    if (systemm == 1) {
        // printMemUsage(samples, tdelay);
        // logCores();
        // logCpuUsage();
        // printMachineInfo();
    } 

    //--user
    else if (user == 1) {
        // printUsers();
    } 

    //--sequential
    else if (sequential == 1) {

        //warnng: give it some time if you are trying to output to a file
        // seqFlag(samples,tdelay);

    } 
    else {
        if(graphics == 1){
            graphicalRefresh(samples, tdelay);
            return 0;
        }
        //refresh-like output
        //graphicalRefresh(samples,tdelay);
        //defaultOutput(samples, tdelay);
        bruh(samples, tdelay);

    }
}

//print hello World