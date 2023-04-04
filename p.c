#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
    double physical_used;
    double physical_total;
    double virtual_used;
    double virtual_total;
} MemUtiInfo;

void getMemUti(MemUtiInfo* mem_info) {
    FILE* fp;
    char buffer[256];
    double physical_total, physical_used, virtual_total, virtual_used;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        printf("Failed to open /proc/meminfo file.\n");
        return;
    }

    fgets(buffer, sizeof(buffer), fp);
    sscanf(buffer, "MemTotal: %lf kB", &physical_total);
    fgets(buffer, sizeof(buffer), fp);
    sscanf(buffer, "MemFree: %lf kB", &physical_used);
    fgets(buffer, sizeof(buffer), fp);
    sscanf(buffer, "SwapTotal: %lf kB", &virtual_total);
    fgets(buffer, sizeof(buffer), fp);
    sscanf(buffer, "SwapFree: %lf kB", &virtual_used);

    fclose(fp);

    mem_info->physical_total = physical_total / 1024.0;
    mem_info->physical_used = (physical_total - physical_used) / 1024.0;
    mem_info->virtual_total = virtual_total / 1024.0;
    mem_info->virtual_used = (virtual_total - virtual_used) / 1024.0;
}

void bruh2(int interval, int n_samples) {
    pid_t pid;
    int status;
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipefd[0]);

        MemUtiInfo mem_info;
        for (int i = 0; i < n_samples; i++) {
            getMemUti(&mem_info);
            write(pipefd[1], &mem_info, sizeof(mem_info));
            sleep(interval);
        }

        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    } else {
        close(pipefd[1]);

        MemUtiInfo mem_info;
        for (int i = 0; i < n_samples; i++) {
            read(pipefd[0], &mem_info, sizeof(mem_info));
            printf("Physical memory usage: %.2lf GB / %.2lf GB\n", mem_info.physical_used, mem_info.physical_total);
            printf("Virtual memory usage: %.2lf GB / %.2lf GB\n", mem_info.virtual_used, mem_info.virtual_total);
            sleep(interval);
        }

        close(pipefd[0]);

    }
}
