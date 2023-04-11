#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <utmp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <signal.h>
#include <limits.h>

#include "stats_functions.h"
#include "parseArgs.h"


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

    if(systemm == 1 && user == 1){
        printf("Error: Cannot have both --systemm and --user\n");
        exit(1);
    }
    //--system
    if (systemm == 1) {
        systemRefresh(samples,tdelay,sequential);
    } 

    //--user
    else if (user == 1) {
        usersRefresh(samples, tdelay, graphics, sequential);
    } 
    else {
        signal(SIGINT, sigint_handler);
        graphicalRefresh(samples, tdelay, graphics, sequential);
    }
}

//print hello World