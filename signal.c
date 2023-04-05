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