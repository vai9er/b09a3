#include "commonLibs.h"

// void clear_screen() {
//   printf("\033[2J");  // clear entire screen
//   printf("\033[%d;%dH", 0, 0);  // move cursor to the top-left corner
// }



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
