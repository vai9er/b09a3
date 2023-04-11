#include "commonLibs.h"
#include "stats_functions.c"
#include "parseArgs.c"


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
        systemRefresh(samples,tdelay,graphics);
    } 

    //--user
    else if (user == 1) {
        if(graphics == 1){
            ///
        }
    } 

    //--sequential
    else if (sequential == 1) {
        sequentialRefresh(samples,tdelay,graphics);

        //warnng: give it some time if you are trying to output to a file

    } 
    else {
        signal(SIGINT, sigint_handler);
        graphicalRefresh(samples, tdelay, graphics);
    }
}

//print hello World