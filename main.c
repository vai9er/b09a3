#include "commonLibs.h"
#include "stats_functions.c"
#include "parseArgs.c"

void handle_SIGTSTP(int signum){}

int handle_SIGINT(int signum){
    char response;
    printf("\nDo you want to quit? [y/n] ");
    scanf(" %c", &response);
    if (response == 'y' || response == 'Y'){
        // user wants to quit, terminate the program
        exit(0);
    }
    if (response == 'n' || response == 'N'){}
    
}


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
        systemmm(samples, tdelay, graphics);
    } 

    //--user
    else if (user == 1) {
        if(graphics == 1){
            ///
        }
    } 

    //--sequential
    else if (sequential == 1) {
        if(graphics == 1){
            ///
        }

        //warnng: give it some time if you are trying to output to a file
        // seqFlag(samples,tdelay);

    } 
    else {

        //signal(SIGINT, handle_SIGINT);
        graphicalRefresh(samples, tdelay, graphics);
        //systemmm(samples, tdelay, graphics);
    }
}

//print hello World