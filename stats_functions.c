#include "commonLibs.h"
#define PROC_STAT "/proc/stat"
#define READ_END 0
#define WRITE_END 1
#define MAX_USERS 100

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

void printMemUsage( int NUM_SAMPLES, int SLEEP_TIME) {
    printf("Number of samples: %d -- every %d secs\n", NUM_SAMPLES, SLEEP_TIME);

    //step 1: get system information
    struct sysinfo systemInfo;
    sysinfo(&systemInfo);
    
    // Get total and used memory
    float memory_total = systemInfo.totalram;
    float memory_used = systemInfo.totalram - systemInfo.freeram;

    // Print memory usage in kilobytes
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    // Print memory usage in kilobytes
    long memory_usage_kb = usage.ru_maxrss;
    printf("Memory usage: %ld kilobytes\n", memory_usage_kb);
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

    for (int i = 0; i < NUM_SAMPLES; i++) {

        //use .2f as specifier- no more than 2 decimal places to mimic the handout
        printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB\n", memory_used / (1024 * 1024 * 1024), memory_total / (1024 * 1024 * 1024), memory_used / (1024 * 1024 * 1024), (memory_total + systemInfo.totalswap) / (1024 * 1024 * 1024));
        
        sleep(SLEEP_TIME);
    }


    printf("---------------------------------------\n");

}

//this function prints the number of cores on the system
void logCores(){

    printf("-------------------------------------\n");
    printf("Number of cores: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));
    
}

unsigned sleep(unsigned sec);

//struct to hold the cpu stats
struct cpustat {
    unsigned long t_user;
    unsigned long t_nice;
    unsigned long t_system;
    unsigned long t_idle;
    unsigned long t_iowait;
    unsigned long t_irq;
    unsigned long t_softirq;
};

//function to skip lines in a file
void skip_lines(FILE *fp, int numlines)
{
    int cnt = 0;
    char ch;
    //while the counter is less than the number of lines to skip and the character is not the end of the file
    while((cnt < numlines) && ((ch = getc(fp)) != EOF))
    {
        //if the character is a new line, increment the counter
        if (ch == '\n')
            cnt++;
    }
    return;
}

//function to get the cpu stats
void get_stats(struct cpustat *st, int cpunum)
{
    //open the file
    FILE *fp = fopen("/proc/stat", "r");
    //lskip is the number of lines to skip
    int lskip = cpunum+1;
    //skip the lines
    skip_lines(fp, lskip);
    //read the file
    char cpun[255];
    //read the file and store the values in the struct
    fscanf(fp, "%s %ld %ld %ld %ld %ld %ld %ld", cpun, &(st->t_user), &(st->t_nice), 
        &(st->t_system), &(st->t_idle), &(st->t_iowait), &(st->t_irq),
        &(st->t_softirq));
    //close the file
    fclose(fp);
	return;
}

//function to calculate the cpu load
double calculate_load(struct cpustat *prev, struct cpustat *cur)
{
    //calculate the idle and non idle times
    int idle_prev = (prev->t_idle) + (prev->t_iowait);
    int idle_cur = (cur->t_idle) + (cur->t_iowait);

    //calculate the non idle times
    int nidle_prev = (prev->t_user) + (prev->t_nice) + (prev->t_system) + (prev->t_irq) + (prev->t_softirq);
    int nidle_cur = (cur->t_user) + (cur->t_nice) + (cur->t_system) + (cur->t_irq) + (cur->t_softirq);

    //calculate the total times
    int total_prev = idle_prev + nidle_prev;
    int total_cur = idle_cur + nidle_cur;

    //calculate the cpu load
    double totald = (double) total_cur - (double) total_prev;
    double idled = (double) idle_cur - (double) idle_prev;

    //calculate the cpu percentage
    double cpu_perc = (1000 * (totald - idled) / totald + 1) / 10;

    //return the cpu percentage
    return cpu_perc;
}

void logCpuUsage(){
    //print the cpu usage
    struct cpustat st0_0, st0_1;
    //get the stats
    get_stats(&st0_0, -1);
    //sleep for 1 second
    sleep(1);
    //get the stats again
    get_stats(&st0_1, -1);
    //print the cpu usage
    printf("CPU: %.2lf%%\n\n", calculate_load(&st0_0, &st0_1));
    
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

void get_cpu_times(unsigned long long *idle, unsigned long long *total) {
    FILE *fp;
    unsigned long long user, nice, sys, irq, softirq, steal;
    unsigned long long ioWait;

    fp = fopen(PROC_STAT, "r");
    if (fp == NULL) {
        perror("Failed to open " PROC_STAT);
        exit(EXIT_FAILURE);
    }

    fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &sys, idle, &ioWait, &irq, &softirq, &steal);
    fclose(fp);

    *total = user + nice + sys + *idle + ioWait + irq + softirq + steal;
}

double get_cpu_utilization(int tdelay) {
    unsigned long long idle1, total1;
    unsigned long long idle2, total2;

    get_cpu_times(&idle1, &total1);
    sleep(tdelay);
    get_cpu_times(&idle2, &total2);

    double idleDelta = idle2 - idle1;
    double totalDelta = total2 - total1;

    return 1.0 - idleDelta / totalDelta;
}

double get_cpu_utilization2() {
    unsigned long long idle1, total1;
    unsigned long long idle2, total2;

    get_cpu_times(&idle1, &total1);
    sleep(1);
    get_cpu_times(&idle2, &total2);

    double idleDelta = idle2 - idle1;
    double totalDelta = total2 - total1;

    return 1.0 - idleDelta / totalDelta;
}

//this function prints the users and sessions using the utmp file
//the utmp file is a file that stores information about the users and sessions
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

void refresh23(int samples, int tdelay){
    int memory_pipe[2], users_pipe[2], machine_pipe[2], cpu_pipe[2];
    pid_t memory_pid, users_pid, machine_pid, cpu_pid;

    if (pipe(memory_pipe) == -1 || pipe(users_pipe) == -1 || pipe(machine_pipe) == -1 || pipe(cpu_pipe) == -1) {
        fprintf(stderr,"Pipe failed");
        exit(EXIT_FAILURE);
    }

    memory_pid = fork();
    if (memory_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (memory_pid == 0) {
        // Memory child process
        mem_info_t mem_info[samples];
        close(memory_pipe[READ_END]);
        // Perform memory utilization task and write results to pipe
        // ...
        //write(memory_pipe[WRITE_END], &mem_info[i], sizeof(mem_info_t));
        close(memory_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    users_pid = fork();
    if (users_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (users_pid == 0) {
        // Users child process
        close(users_pipe[READ_END]);
        // Perform connected users task and write results to pipe
        // ...
        close(users_pipe[WRITE_END]);
        exit(EXIT_SUCCESS);
    }

    machine_pid = fork();
    if (machine_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (machine_pid == 0) {
        // Machine child process
        logSessional(machine_pipe);
        exit(EXIT_SUCCESS);
    }

    cpu_pid = fork();
    if (cpu_pid < 0) {
        fprintf(stderr,"Fork failed");
        exit(EXIT_FAILURE);
    } else if (cpu_pid == 0) {
        // CPU child process
        close(cpu_pipe[READ_END]);
        
        cpu_sample_t cpu_utilization[samples];
        
        for (int i = 0; i < samples; i++) {
            cpu_utilization[i].utilization = 100 * get_cpu_utilization(tdelay);
            cpu_utilization[i].num_bars = (int)(cpu_utilization[i].utilization / 0.32);
            
            write(cpu_pipe[WRITE_END], &cpu_utilization[i], sizeof(cpu_sample_t));
            sleep(tdelay);
        }
        
        close(cpu_pipe[WRITE_END]);
        
        exit(EXIT_SUCCESS);
    }

    // Parent process
    close(memory_pipe[WRITE_END]);
    close(users_pipe[WRITE_END]);
    close(machine_pipe[WRITE_END]);
    close(cpu_pipe[WRITE_END]);


    // Read results from pipes and print them
    struct session_info info;
    cpu_sample_t cpu_utilization;
    mem_info_t mem_info;
    char users[1024];
    char machine[1024];

    for (int i = 0; i < samples; i++) {
        printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
        read(machine_pipe[READ_END], &info, sizeof(info));
        printf("### Sessions/users ### \n");
        printf("%s", info.users[i]);
    //     // Read memory utilization result from pipe and print it
    //     read(memory_pipe[READ_END], &mem_info, sizeof(mem_info_t));
    //     // ...
    //     // printMemoryUtilization(...);

    //     // Read connected users result from pipe and print it
    //     read(users_pipe[READ_END], users, sizeof(users));
    //     printf("%s\n", users);

    //     // Read machine information result from pipe and print it
    //     read(machine_pipe[READ_END], machine, sizeof(machine));
    //     printf("%s\n", machine);

    //     // Read CPU utilization result from pipe and print it
    //     read(cpu_pipe[READ_END], &cpu_utilization, sizeof(cpu_sample_t));
    //     printf("Number of cores: %d \n", get_nprocs());
    //     printf("total cpu use = %.2f%%\n", cpu_utilization.utilization);
    
    //     if (cpu_utilization.num_bars > 0) {
    //         printf("\t");
    //         for (int j = 0; j < cpu_utilization.num_bars; j++) {
    //             printf("|");
    //         }
    //         printf(" %.2f\n", cpu_utilization.utilization);
    //     } else {
    //         printf("\t %.2f\n", cpu_utilization.utilization);
    // }

        //sleep(tdelay);
    }
}

