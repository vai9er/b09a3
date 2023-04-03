#include "commonLibs.h"
#define PROC_STAT "/proc/stat"
#define READ_END 0
#define WRITE_END 1
#define MAX_USERS 100
#include "helpers.c"
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

typedef struct {
    double utilization;
    int num_bars;
} cpu_sample_t;

typedef struct {
    float prev_memory_used;
} mem_info_t;

struct session_info {
    char users[MAX_USERS][100];
    int num_users;
};

struct mem_info {
    unsigned long long memory_usage_kb;
    float *memory_used;
    float memory_total;
    int num_samples;
};

struct cpu_info {
    double utilization;
    int num_bars;
};

struct memory_display {
    char display[100];
};

void clear_screen() {
  printf("\033[2J");  // clear entire screen
  printf("\033[%d;%dH", 0, 0);  // move cursor to the top-left corner
}


//MACHINEINFO
// This function prints the machine information of the system. using the uname() function, we can get the system information and print it out.
void printMachineInfo(){

    printf("### System Information ### \n");
    struct utsname compInfo;
    uname(&compInfo);
    printf("SYSTEM NAME: %s \n", compInfo.sysname);
    printf("NODE NAME: %s \n", compInfo.nodename);
    printf("RELEASE: %s \n", compInfo.release);
    printf("VERSION: %s \n", compInfo.version);
    printf("MACHINE: %s \n", compInfo.machine);
    printf("---------------------------------------\n");

}

//this function prints(writes to pipe) the users and sessions using the utmp file
//the utmp file is a file that stores information about the users and sessions
void logSessional(int machine_pipe[2], int NUM_SAMPLES, int SLEEP_TIME){
    // Machine child process
    close(machine_pipe[READ_END]);

    for (int i = 0; i < NUM_SAMPLES; i++) {
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
        
        // sleep(SLEEP_TIME);
    }

    close(machine_pipe[WRITE_END]);
}

void getMemUtil(int NUM_SAMPLES, int SLEEP_TIME, int pipefd[2]) {
    //step 1: get system information
    struct sysinfo systemInfo;
    sysinfo(&systemInfo);
    
    // Get total and used memory
    float memory_total = systemInfo.totalram;

    struct mem_info info;
    info.memory_total = memory_total;
    info.num_samples = NUM_SAMPLES;
    info.memory_used = malloc(NUM_SAMPLES * sizeof(float));

    for (int i = 0; i < NUM_SAMPLES; i++) {
        sysinfo(&systemInfo);
        info.memory_used[i] = systemInfo.totalram - systemInfo.freeram;

        write(pipefd[1], &info, sizeof(info));

        //sleep(SLEEP_TIME);
    }

    // Deallocate memory
    free(info.memory_used);
}

void getCpuUsage(int samples, int tdelay, int pipe[]) {
    struct cpu_info info[samples];
    for (int i = 0; i < samples; i++) {
        info[i].utilization = 100 * get_cpu_utilization(tdelay);
        info[i].num_bars = (int)(info[i].utilization / 0.15);
        write(pipe[WRITE_END], &info[i], sizeof(info[i]));
        // sleep(tdelay);
    }
}

void refresh23(int samples, int tdelay){
    int machine_pipe[2];
    pid_t machine_pid;
    int memory_pipe[2];
    pid_t memory_pid;

    if (pipe(machine_pipe) == -1 || pipe(memory_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    machine_pid = fork();
    if (machine_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (machine_pid == 0) {
        logSessional(machine_pipe, samples, tdelay);
        exit(EXIT_SUCCESS);
    }

    memory_pid = fork();
    if (memory_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (memory_pid == 0) {
        // Memory child process
        close(memory_pipe[READ_END]);
        // Perform memory utilization task and write results to pipe
        getMemUtil(samples, tdelay, memory_pipe);
        close(memory_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        struct session_info info;
        close(machine_pipe[WRITE_END]);
        
        for (int i = 0; i < samples; i++) {
            clear_screen();
            struct mem_info mem_info;
            read(memory_pipe[READ_END], &mem_info, sizeof(mem_info));
            printf("Memory usage: %ld kilobytes\n", mem_info.memory_usage_kb);
            // printf("%.2f GB / %.2f GB\n", mem_info.memory_used / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024));
            read(machine_pipe[READ_END], &info, sizeof(info));

            printf("### Sessions/users ### \n");
            for (int j = 0; j < info.num_users; j++) {
                printf("%s", info.users[j]);
            }
            
            
            sleep(tdelay);
        }
        
        close(machine_pipe[READ_END]);
        close(memory_pipe[READ_END]);
    }
}



void refresh234(int samples, int tdelay){
    int machine_pipe[2];
    pid_t machine_pid;
    int memory_pipe[2];
    pid_t memory_pid;
    int cpu_pipe[2];
    pid_t cpu_pid;

    if (pipe(machine_pipe) == -1 || pipe(memory_pipe) == -1 || pipe(cpu_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    machine_pid = fork();
    if (machine_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (machine_pid == 0) {
        logSessional(machine_pipe, samples, tdelay);
        exit(EXIT_SUCCESS);
    }

    memory_pid = fork();
    if (memory_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (memory_pid == 0) {
        // Memory child process
        close(memory_pipe[READ_END]);
        // Perform memory utilization task and write results to pipe
        getMemUtil(samples, tdelay, memory_pipe);
        close(memory_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    cpu_pid = fork();
    if (cpu_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (cpu_pid == 0) {
        // CPU child process
        close(cpu_pipe[READ_END]);
        // Perform CPU utilization task and write results to pipe
        getCpuUsage(samples, tdelay, cpu_pipe);
        close(cpu_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        struct session_info info;
        close(machine_pipe[WRITE_END]);
        
        struct cpu_info cpu_info[samples];
        
        for (int i = 0; i < samples; i++) {
            read(cpu_pipe[READ_END], &cpu_info[i], sizeof(cpu_info[i]));
            clear_screen();
            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);

            struct sysinfo systemInfo;
            sysinfo(&systemInfo);

            float memory_total = systemInfo.totalram;
            float memory_used = systemInfo.totalram - systemInfo.freeram;

            printf("Memory usage: %.0f kilobytes\n", (memory_used / 1024)/1024);

            struct mem_info mem_info;
            read(memory_pipe[READ_END], &mem_info, sizeof(mem_info));
            // printf("%.2f GB / %.2f GB\n", mem_info.memory_used / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024));

            read(machine_pipe[READ_END], &info, sizeof(info));

            for(int z = 0; z < samples - i; z++){
                printf("\n");
            }
            printf("### Sessions/users ### \n");
            for (int j = 0; j < info.num_users; j++) {
                printf("%s", info.users[j]);
            }

            printf("---------------------------------------\n");
            printf("Number of cores: %d \n", get_nprocs());
            printf("total cpu use = %.2f%%\n", cpu_info[i].utilization);

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

            printMachineInfo();

        }
        
        close(machine_pipe[READ_END]);
        close(memory_pipe[READ_END]);
        close(cpu_pipe[READ_END]);
    }
}

void graphicalRefresh(int samples, int tdelay){
    int machine_pipe[2];
    pid_t machine_pid;
    int memory_pipe[2];
    pid_t memory_pid;
    int cpu_pipe[2];
    pid_t cpu_pid;

    if (pipe(machine_pipe) == -1 || pipe(memory_pipe) == -1 || pipe(cpu_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    machine_pid = fork();
    if (machine_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (machine_pid == 0) {
        logSessional(machine_pipe, samples, tdelay);
        exit(EXIT_SUCCESS);
    }

    memory_pid = fork();
    if (memory_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (memory_pid == 0) {
        // Memory child process
        close(memory_pipe[READ_END]);
        // Perform memory utilization task and write results to pipe
        getMemUtil(samples, tdelay, memory_pipe);
        close(memory_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    cpu_pid = fork();
    if (cpu_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (cpu_pid == 0) {
        // CPU child process
        close(cpu_pipe[READ_END]);
        // Perform CPU utilization task and write results to pipe
        getCpuUsage(samples, tdelay, cpu_pipe);
        close(cpu_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(memory_pipe[WRITE_END]);

        struct mem_info mem_info;
        read(memory_pipe[READ_END], &mem_info, sizeof(mem_info));
        mem_info.memory_used = malloc(mem_info.num_samples * sizeof(float));
        read(memory_pipe[READ_END], mem_info.memory_used, mem_info.num_samples * sizeof(float));

        struct session_info info;
        close(machine_pipe[WRITE_END]);
        
        struct cpu_info cpu_info[samples];

        
        float memory_usage[samples];
        for (int i = 0; i < mem_info.num_samples; i++) {
            memory_usage[i] = mem_info.memory_used[i];
            read(cpu_pipe[READ_END], &cpu_info[i], sizeof(cpu_info[i]));
            clear_screen();
            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
            struct sysinfo systemInfo;
            sysinfo(&systemInfo);

            float memory_total = systemInfo.totalram;
            float memory_used = systemInfo.totalram - systemInfo.freeram;

              
            printf("Memory usage: %.0f kilobytes\n", (memory_used / 1024)/1024);
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            printf("---------------------------------------\n");

            struct memory_display memory_display[samples];
            for (int j = 0; j < i; j++) {
                float change = memory_usage[j+1] - memory_usage[j];
                printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", mem_info.memory_used[j] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[j] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
                if (j == i - 1) {
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
                printf("%s", memory_display[j].display);
            }
            printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", mem_info.memory_used[i] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));


            
            read(machine_pipe[READ_END], &info, sizeof(info));

            for(int z = 0; z < samples - i; z++){
                printf("\n");
            }
            printf("### Sessions/users ### \n");
            for (int j = 0; j < info.num_users; j++) {
                printf("%s", info.users[j]);
            }

            printf("---------------------------------------\n");
            printf("Number of cores: %d \n", get_nprocs());
            printf("total cpu use = %.2f%%\n", cpu_info[i].utilization);

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

            printMachineInfo();
        }
        free(mem_info.memory_used);
        close(machine_pipe[READ_END]);
        close(memory_pipe[READ_END]);
        close(cpu_pipe[READ_END]);
    }
}

void defaultOutput(int samples, int tdelay){
    int machine_pipe[2];
    pid_t machine_pid;
    int memory_pipe[2];
    pid_t memory_pid;
    int cpu_pipe[2];
    pid_t cpu_pid;

    if (pipe(machine_pipe) == -1 || pipe(memory_pipe) == -1 || pipe(cpu_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    machine_pid = fork();
    if (machine_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (machine_pid == 0) {
        logSessional(machine_pipe, samples, tdelay);
        exit(EXIT_SUCCESS);
    }

    memory_pid = fork();
    if (memory_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (memory_pid == 0) {
        // Memory child process
        close(memory_pipe[READ_END]);
        // Perform memory utilization task and write results to pipe
        getMemUtil(samples, tdelay, memory_pipe);
        close(memory_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    cpu_pid = fork();
    if (cpu_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (cpu_pid == 0) {
        // CPU child process
        close(cpu_pipe[READ_END]);
        // Perform CPU utilization task and write results to pipe
        getCpuUsage(samples, tdelay, cpu_pipe);
        close(cpu_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        close(memory_pipe[WRITE_END]);

        struct mem_info mem_info;
        read(memory_pipe[READ_END], &mem_info, sizeof(mem_info));
        mem_info.memory_used = malloc(mem_info.num_samples * sizeof(float));
        read(memory_pipe[READ_END], mem_info.memory_used, mem_info.num_samples * sizeof(float));

        struct session_info info;
        close(machine_pipe[WRITE_END]);
        
        struct cpu_info cpu_info[samples];

        
        float memory_usage[samples];
        for (int i = 0; i < mem_info.num_samples; i++) {
            read(cpu_pipe[READ_END], &cpu_info[i], sizeof(cpu_info[i]));
            clear_screen();
            memory_usage[i] = mem_info.memory_used[i];

            printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
            struct sysinfo systemInfo;
            sysinfo(&systemInfo);

            float memory_total = systemInfo.totalram;
            float memory_used = systemInfo.totalram - systemInfo.freeram;

              
            printf("Memory usage: %.0f kilobytes\n", (memory_used / 1024)/1024);
            printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
            printf("---------------------------------------\n");

            struct memory_display memory_display[samples];
            for (int j = 0; j < i; j++) {
                float change = memory_usage[j+1] - memory_usage[j];
                printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", mem_info.memory_used[j] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[j] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
            }
            printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", mem_info.memory_used[i] / (1024 * 1024 * 1024), mem_info.memory_total / (1024 * 1024 * 1024), mem_info.memory_used[i] / (1024 * 1024 * 1024), (mem_info.memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));

            
            read(machine_pipe[READ_END], &info, sizeof(info));

            for(int z = 0; z < samples - i; z++){
                printf("\n");
            }
            printf("### Sessions/users ### \n");
            for (int j = 0; j < info.num_users; j++) {
                printf("%s", info.users[j]);
            }

            printf("---------------------------------------\n");
            printf("Number of cores: %d \n", get_nprocs());
            printf("total cpu use = %.2f%%\n", cpu_info[i].utilization);

            // for (int k = 0; k <= i; k++) {
            //     if (cpu_info[k].num_bars > 0) {
            //         printf("\t");
            //         for (int j = 0; j < cpu_info[k].num_bars; j++) {
            //             printf("|");
            //         }
            //         printf(" %.2f\n", cpu_info[k].utilization);
            //     } else {
            //         printf("\t %.2f\n", cpu_info[k].utilization);
            //     }
            // }

            printMachineInfo();

        }
        
        close(machine_pipe[READ_END]);
        close(memory_pipe[READ_END]);
        close(cpu_pipe[READ_END]);
    }
}