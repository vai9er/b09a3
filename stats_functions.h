#define READ_END 0
#define WRITE_END 1
#include "helpers.h"
#define SIGINT 2
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#define SIGTSTP 20
#include <setjmp.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <utmp.h>
#include <unistd.h>
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


void sigint_handler(int sig) {
    char c;
    signal(sig, SIG_IGN);
    printf("\nDo you really want to quit? [y/n] ");
    c = getchar();
    if (c == 'y' || c == 'Y') {
        exit(EXIT_SUCCESS);
    } else {
        signal(SIGINT, sigint_handler);
    }
}

//this function prints(writes to pipe) the users and sessions using the utmp file
//the utmp file is a file that stores information about the users and sessions
void logSessional(int users_pipe[2], int NUM_SAMPLES, int SLEEP_TIME) {
    // Machine child process
    close(users_pipe[READ_END]);

    for (int i = 0; i < NUM_SAMPLES; i++) {
        // Perform machine information task and write results to pipe
        struct session_info info;
        memset(&info, 0, sizeof(info)); // Initialize info structure to zero

        // get sessional information
        struct utmp *userSession;

        //open file
        setutent();

        //print the sessions
        while ((userSession = getutent()) != NULL) {
            if (userSession->ut_type == USER_PROCESS) {
                sprintf(info.users[info.num_users], " %s   %s (%s)\n", userSession->ut_user, userSession->ut_line, userSession->ut_host);
                info.num_users++;
            }
        }

        //close file
        endutent();

        write(users_pipe[WRITE_END], &info, sizeof(info));

        sleep(SLEEP_TIME);
    }

    close(users_pipe[WRITE_END]);
}


void getMachineInfo(int pipefd[2]) {
    MachineInfo info;
    struct utsname compInfo;
    uname(&compInfo);
    strncpy(info.sysname, compInfo.sysname, sizeof(info.sysname));
    strncpy(info.nodename, compInfo.nodename, sizeof(info.nodename));
    strncpy(info.release, compInfo.release, sizeof(info.release));
    strncpy(info.version, compInfo.version, sizeof(info.version));
    strncpy(info.machine, compInfo.machine, sizeof(info.machine));
    write(pipefd[1], &info, sizeof(MachineInfo));
}

void getMemUtil(int NUM_SAMPLES, int SLEEP_TIME, int pipefd[2]) {
    //step 1: get system information
    struct sysinfo systemInfo;
    sysinfo(&systemInfo);
    
    // Get total and used memory
    float memory_total = systemInfo.totalram;
    write(pipefd[1], &memory_total, sizeof(memory_total));

    // struct mem_info info;
    // info.memory_total = memory_total;
    // info.num_samples = NUM_SAMPLES;
    // info.memory_used = malloc(NUM_SAMPLES * sizeof(float));

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sysinfo(&systemInfo);
        float memory_used = systemInfo.totalram - systemInfo.freeram;
        write(pipefd[1], &memory_used, sizeof(memory_used));

        sleep(SLEEP_TIME);
    }

    // // Deallocate memory
    // free(info.memory_used);
}



void getCpuUsage(int samples, int tdelay, int pipe[]) {
    struct cpu_info info[samples];
    for (int i = 0; i < samples; i++) {
        info[i].utilization = 100 * get_cpu_utilization(tdelay);
        info[i].num_bars = (int)(info[i].utilization / 0.75);
        write(pipe[WRITE_END], &info[i], sizeof(info[i]));
        // sleep(tdelay);
    }
}




void graphicalRefresh(int samples, int tdelay, int graphics, int sequential){
    int users_pipe[2];
    pid_t users_pid;
    int memory_pipe[2];
    pid_t memory_pid;
    int cpu_pipe[2];
    pid_t cpu_pid;

    int machine_pipe[2];
    pid_t machine_pid;

    pid_t watchdog_pid;
    if (pipe(users_pipe) == -1 || pipe(memory_pipe) == -1 || pipe(cpu_pipe) == -1 || pipe(machine_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTSTP);
    sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
    watchdog_pid = fork();
    if (watchdog_pid < 0) {
        fprintf(stderr,"Watchdog fork failed");
        exit(EXIT_FAILURE);
    } else if (watchdog_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        pid_t parent = getppid();
        while (1) {
            // Check if parent process is still alive
            if (getppid() != parent) {
                // Parent process has died, terminate all child processes and exit
                kill(users_pid, SIGTERM);
                kill(memory_pid, SIGTERM);
                kill(cpu_pid, SIGTERM);
                kill(machine_pid, SIGTERM);
                exit(EXIT_SUCCESS);
            }

            sleep(tdelay);
        }
    }

    users_pid = fork();
    if (users_pid < 0) {
        fprintf(stderr,"Users/Sessions fork failed");
        exit(EXIT_FAILURE);
    } else if (users_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        logSessional(users_pipe, samples, tdelay);
        exit(EXIT_SUCCESS);
    }

    memory_pid = fork();
    if (memory_pid < 0) {
        fprintf(stderr,"Memory util fork failed");
        exit(EXIT_FAILURE);
    } else if (memory_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        // Memory child process
        close(memory_pipe[READ_END]);
        // Perform memory utilization task and write results to pipe

        getMemUtil(samples, tdelay, memory_pipe);
        
        close(memory_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    machine_pid = fork();
    if(machine_pid < 0){
        fprintf(stderr,"Machine info fork failed");
        exit(EXIT_FAILURE);
    }
    else if (machine_pid == 0){
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        close(machine_pipe[0]); // close unused read end
        getMachineInfo(machine_pipe);
        close(machine_pipe[1]); // close write end after writing
        exit(EXIT_SUCCESS);
    }

    cpu_pid = fork();
    if (cpu_pid < 0) {
        fprintf(stderr,"Cpu util fork failed");
        exit(EXIT_FAILURE);
    } else if (cpu_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        // CPU child process
        close(cpu_pipe[READ_END]);
        // Perform CPU utilization task and write results to pipe
        getCpuUsage(samples, tdelay, cpu_pipe);
        close(cpu_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        signal(SIGINT, sigint_handler);
        close(memory_pipe[WRITE_END]);
        close(machine_pipe[1]); 
        struct mem_info mem_info;
        mem_info.num_samples = samples;
        mem_info.memory_used = malloc(samples * sizeof(float));
        read(memory_pipe[READ_END], &mem_info.memory_total, sizeof(mem_info.memory_total));
        float memory_usage[samples];


        struct session_info info;
        close(users_pipe[WRITE_END]);
        

        struct cpu_info cpu_info[samples];


        MachineInfo Minfo;
        read(machine_pipe[0], &Minfo, sizeof(MachineInfo));

        struct sysinfo systemInfo;
        sysinfo(&systemInfo);

        //float memory_total = systemInfo.totalram;
        float memory_used = systemInfo.totalram - systemInfo.freeram;

        for (int i = 0; i < mem_info.num_samples; i++) {
            if(sequential == 1){
                printf("\n>>>ITERATION %d\n", i+1);
            }
            else{ clear_screen();}

            read(memory_pipe[READ_END], &mem_info.memory_used[i], sizeof(mem_info.memory_used[i]));
            memory_usage[i] = mem_info.memory_used[i];


            read(cpu_pipe[READ_END], &cpu_info[i], sizeof(cpu_info[i]));


            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);

              
            printf("Memory usage: %.0f kilobytes\n", (memory_used / 1024)/1024);
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            printf("---------------------------------------\n");


            struct memory_display memory_display[samples];
            for (int j = 0; j < i; j++) {
                float change = memory_usage[j+1] - memory_usage[j];
                if(sequential == 1){printf("\n");}

                else{printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", mem_info.memory_used[j] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[j] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));}
                
                if (j == i - 1 && graphics == 1) {
                    if (change == 0) {
                        sprintf(memory_display[j].display, " |o %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[j+1] / (1024 * 1024 * 1024));
                    } else if (change > 0) {
                        int num_hashes = change / (mem_info.memory_total / (samples*8));
                        sprintf(memory_display[j].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            sprintf(memory_display[j].display + strlen(memory_display[j].display), "#");
                        }
                        sprintf(memory_display[j].display + strlen(memory_display[j].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[j+1] / (1024 * 1024 * 1024));
                    } else {
                        int num_hashes = -change / (mem_info.memory_total / (samples*10));
                        sprintf(memory_display[j].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            sprintf(memory_display[j].display + strlen(memory_display[j].display), "@");
                        }
                        sprintf(memory_display[j].display + strlen(memory_display[j].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[j+1] / (1024 * 1024 * 1024));
                    }
                }
                if(sequential == 1){printf("\n");}
                else{printf("%s\n", memory_display[j].display);}
            }
            if(graphics == 1){
                if(i == 0){printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB |o 0.00 (%.2f) ", mem_info.memory_used[i] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));}
                else{    
                    float change = memory_usage[i] - memory_usage[i-1];
                    printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\t", mem_info.memory_used[i-1] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i-1] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
                    if (change == 0) {
                        printf(memory_display[i-1].display, " |o %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        sprintf(memory_display[i-1].display, " |o %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                    } else if (change > 0) {
                        int num_hashes = change / (mem_info.memory_total / (samples*8));
                        printf(memory_display[i-1].display, " |");
                        sprintf(memory_display[i-1].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "#");
                            sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "#");
                        }
                        printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                    } else {
                        int num_hashes = -change / (mem_info.memory_total / (samples*10));
                        printf(memory_display[i-1].display, " |");
                        sprintf(memory_display[i-1].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "@");
                            sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "@");
                        }
                        printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        
                    }
                    printf("%s\n", memory_display[i].display);
                }
            }

            else{printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", mem_info.memory_used[i] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));}
            
            read(users_pipe[READ_END], &info, sizeof(info));

             for(int z = 0; z < samples - i; z++){
                 printf("\n");
             }
            printf("---------------------------------------\n");
            printf("\n### Sessions/users ### \n");
            for (int j = 0; j < info.num_users; j++) {
                printf("%s", info.users[j]);
            }

            printf("---------------------------------------\n");
            printf("Number of cores: %d \n", get_nprocs());
            printf("total cpu use = %.2f%%\n", cpu_info[i].utilization);

            if(graphics == 1){
                for (int k = 0; k <= i; k++) {
                    if (cpu_info[k].num_bars > 0) {
                        printf("\t");
                        for (int j = 0; j < cpu_info[k].num_bars; j++) {
                            printf("|");
                        }
                        printf(" %.2f\n", cpu_info[k].utilization);
                    } else {
                        printf("\t %.2f\n", cpu_info[k].utilization);
                    }
                }
            }
            printf("---------------------------------------\n");
            printf("### System Information ### \n");
            printf("SYSTEM NAME: %s \n", Minfo.sysname);
            printf("NODE NAME: %s \n", Minfo.nodename);
            printf("RELEASE: %s \n", Minfo.release);
            printf("VERSION: %s \n", Minfo.version);
            printf("MACHINE: %s \n", Minfo.machine);
            printf("---------------------------------------\n");
            

            if (i == samples){
                exit(EXIT_SUCCESS);
            }
        }
        free(mem_info.memory_used);
        close(users_pipe[READ_END]);
        close(memory_pipe[READ_END]);
        close(cpu_pipe[READ_END]);
        //WRITE IMPLEMENTATION HERE
        close(machine_pipe[0]);
    }
}



void systemRefresh(int samples, int tdelay, int graphics, int sequential){
    int memory_pipe[2];
    pid_t memory_pid;
    int cpu_pipe[2];
    pid_t cpu_pid;


    pid_t watchdog_pid;
    if (pipe(memory_pipe) == -1 || pipe(cpu_pipe) == -1 ) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTSTP);
    sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
    watchdog_pid = fork();
    if (watchdog_pid < 0) {
        fprintf(stderr,"Watchdog fork failed");
        exit(EXIT_FAILURE);
    } else if (watchdog_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        pid_t parent = getppid();
        while (1) {
            // Check if parent process is still alive
            if (getppid() != parent) {
                // Parent process has died, terminate all child processes and exit
                kill(memory_pid, SIGTERM);
                kill(cpu_pid, SIGTERM);
                exit(EXIT_SUCCESS);
            }

            sleep(tdelay);
        }
    }


    memory_pid = fork();
    if (memory_pid < 0) {
        fprintf(stderr,"Memory util fork failed");
        exit(EXIT_FAILURE);
    } else if (memory_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        //pid_t parent = getppid();
        // Memory child process
        close(memory_pipe[READ_END]);
        // Perform memory utilization task and write results to pipe

        getMemUtil(samples, tdelay, memory_pipe);
        
        close(memory_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    cpu_pid = fork();
    if (cpu_pid < 0) {
        fprintf(stderr,"Cpu util fork failed");
        exit(EXIT_FAILURE);
    } else if (cpu_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        //pid_t parent = getppid();
        // CPU child process
        close(cpu_pipe[READ_END]);
        // Perform CPU utilization task and write results to pipe
        getCpuUsage(samples, tdelay, cpu_pipe);
        close(cpu_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        signal(SIGINT, sigint_handler);
        close(memory_pipe[WRITE_END]);
        struct mem_info mem_info;
        mem_info.num_samples = samples;
        mem_info.memory_used = malloc(samples * sizeof(float));
        read(memory_pipe[READ_END], &mem_info.memory_total, sizeof(mem_info.memory_total));
        float memory_usage[samples];


        //struct session_info info;
        

        struct cpu_info cpu_info[samples];

        struct sysinfo systemInfo;
        sysinfo(&systemInfo);

        //float memory_total = systemInfo.totalram;
        float memory_used = systemInfo.totalram - systemInfo.freeram;

        for (int i = 0; i < mem_info.num_samples; i++) {
            if(sequential == 1){
                printf("\n>>>ITERATION %d\n", i+1);
            }
            else{ clear_screen();}

            read(memory_pipe[READ_END], &mem_info.memory_used[i], sizeof(mem_info.memory_used[i]));
            memory_usage[i] = mem_info.memory_used[i];


            read(cpu_pipe[READ_END], &cpu_info[i], sizeof(cpu_info[i]));


            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);

              
            printf("Memory usage: %.0f kilobytes\n", (memory_used / 1024)/1024);
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            printf("---------------------------------------\n");


            struct memory_display memory_display[samples];
            for (int j = 0; j < i; j++) {
                float change = memory_usage[j+1] - memory_usage[j];
                if(sequential == 1){printf("\n");}

                else{printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\t", mem_info.memory_used[j] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[j] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));}
                
                if (j == i - 1 && graphics == 1) {
                    if (change == 0) {
                        sprintf(memory_display[j].display, " |o %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[j+1] / (1024 * 1024 * 1024));
                    } else if (change > 0) {
                        int num_hashes = change / (mem_info.memory_total / (samples*8));
                        sprintf(memory_display[j].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            sprintf(memory_display[j].display + strlen(memory_display[j].display), "#");
                        }
                        sprintf(memory_display[j].display + strlen(memory_display[j].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[j+1] / (1024 * 1024 * 1024));
                    } else {
                        int num_hashes = -change / (mem_info.memory_total / (samples*10));
                        sprintf(memory_display[j].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            sprintf(memory_display[j].display + strlen(memory_display[j].display), "@");
                        }
                        sprintf(memory_display[j].display + strlen(memory_display[j].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[j+1] / (1024 * 1024 * 1024));
                    }
                }
                if(sequential == 1){printf("\n");}
                else{printf("%s\n", memory_display[j].display);}
            }
            if(graphics == 1){
                if(i == 0){printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB |o 0.00 (%.2f) ", mem_info.memory_used[i] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));}
                else{    
                    float change = memory_usage[i] - memory_usage[i-1];
                    printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\t", mem_info.memory_used[i-1] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i-1] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
                    if (change == 0) {
                        printf(memory_display[i-1].display, " |o %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        sprintf(memory_display[i-1].display, " |o %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                    } else if (change > 0) {
                        int num_hashes = change / (mem_info.memory_total / (samples*8));
                        printf(memory_display[i-1].display, " |");
                        sprintf(memory_display[i-1].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "#");
                            sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "#");
                        }
                        printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                    } else {
                        int num_hashes = -change / (mem_info.memory_total / (samples*10));
                        printf(memory_display[i-1].display, " |");
                        sprintf(memory_display[i-1].display, " |");
                        for (int k = 0; k < num_hashes; k++) {
                            printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "@");
                            sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "@");
                        }
                        printf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        sprintf(memory_display[i-1].display + strlen(memory_display[i-1].display), "* %.2f (%.2f)\n", change / (1024 * 1024 * 1024), memory_usage[i] / (1024 * 1024 * 1024));
                        
                    }
                    printf("%s\n", memory_display[i].display);
                }
            }

            else{printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", mem_info.memory_used[i] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));}
            

             for(int z = 0; z < samples - i; z++){
                 printf("\n");
             }

            printf("---------------------------------------\n");
            printf("Number of cores: %d \n", get_nprocs());
            printf("total cpu use = %.2f%%\n", cpu_info[i].utilization);

            if(graphics == 1){
                for (int k = 0; k <= i; k++) {
                    if (cpu_info[k].num_bars > 0) {
                        printf("\t");
                        for (int j = 0; j < cpu_info[k].num_bars; j++) {
                            printf("|");
                        }
                        printf(" %.2f\n", cpu_info[k].utilization);
                    } else {
                        printf("\t %.2f\n", cpu_info[k].utilization);
                    }
                }
            }
            
        }
        free(mem_info.memory_used);
        close(memory_pipe[READ_END]);
        close(cpu_pipe[READ_END]);
        //WRITE IMPLEMENTATION HERE
    }
}



void usersRefresh(int samples, int tdelay, int sequential){
    int users_pipe[2];
    pid_t users_pid;

    pid_t watchdog_pid;
    if (pipe(users_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGTSTP);
    sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
    watchdog_pid = fork();
    if (watchdog_pid < 0) {
        fprintf(stderr,"Watchdog fork failed");
        exit(EXIT_FAILURE);
    } else if (watchdog_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        pid_t parent = getppid();
        while (1) {
            // Check if parent process is still alive
            if (getppid() != parent) {
                // Parent process has died, terminate all child processes and exit
                kill(users_pid, SIGTERM);
                exit(EXIT_SUCCESS);
            }

            sleep(tdelay);
        }
    }

    users_pid = fork();
    if (users_pid < 0) {
        fprintf(stderr,"Users/Sessions fork failed");
        exit(EXIT_FAILURE);
    } else if (users_pid == 0) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGTSTP);
        sigaddset(&sigset, SIGINT);
        sigprocmask(SIG_BLOCK, &sigset, NULL);// Add the signals in the set to the process's blocked signals
        logSessional(users_pipe, samples, tdelay);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        signal(SIGINT, sigint_handler);

        close(users_pipe[WRITE_END]);

        for (int i = 0; i < samples; i++) {
            struct session_info info;
            read(users_pipe[READ_END], &info, sizeof(info));

            printf("\n### Sessions/users ### \n");
            for (int j = 0; j < info.num_users; j++) {
                printf("%s", info.users[j]);
            }
            printf("---------------------------------------\n");

            if (i == samples){
                exit(EXIT_SUCCESS);
            }
        }
        
        close(users_pipe[READ_END]);
    }
}