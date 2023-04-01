#include <stdio.h>
#include <utmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define READ_END 0
#define WRITE_END 1
#define MAX_USERS 100

struct session_info {
    char users[MAX_USERS][100];
    int num_users;
};

void logSessional(int machine_pipe[2]){
    // Machine child process
    close(machine_pipe[READ_END]);

    // Perform machine information task and write results to pipe
    struct session_info info;
    info.num_users = 0;

    // get sessional information
    struct utmp *userSession;

    //open file
    setutent();

    //print the sessions
    while ( (userSession = getutent()) != NULL) {
        if (userSession->ut_type == USER_PROCESS) {
            sprintf(info.users[info.num_users], " %s   %s (%s)\n", userSession->ut_user, userSession->ut_line, userSession->ut_host);
            info.num_users++;
        }
    }

    //close file
    endutent();

    write(machine_pipe[WRITE_END], &info, sizeof(info));

    close(machine_pipe[WRITE_END]);
}


int main() {
    int machine_pipe[2];
    pid_t machine_pid;

    if (pipe(machine_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    machine_pid = fork();
    if (machine_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (machine_pid == 0) {
        logSessional(machine_pipe);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        struct session_info info;
        close(machine_pipe[WRITE_END]);
        read(machine_pipe[READ_END], &info, sizeof(info));
        printf("### Sessions/users ### \n");
        for (int i = 0; i < info.num_users; i++) {
            printf("%s", info.users[i]);
        }
        close(machine_pipe[READ_END]);
    }

    return 0;
}