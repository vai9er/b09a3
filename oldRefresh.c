// void Oldrefresh2(int samples, int tdelay){
//     clear_screen();
//     int j = 0;
//     mem_info_t mem_info = {0};
//     float memory_used[samples];
//     cpu_sample_t cpu_utilization[samples];
//     for (int i = 0; i < samples; i++) {
//         cpu_utilization[i].utilization = 100 * get_cpu_utilization(tdelay);
//         cpu_utilization[i].num_bars = (int)(cpu_utilization[i].utilization / 0.32);
//         printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
        
//         //step 1: get system information
//         struct sysinfo systemInfo;
//         sysinfo(&systemInfo);
    
//         // Get total and used memory
//         float memory_total = systemInfo.totalram;
//         memory_used[i] = systemInfo.totalram - systemInfo.freeram;

//         // Print memory usage in kilobytes
//         printf("Memory usage: %.0f kilobytes\n", (memory_used[i] / 1024)/1024);

//         printMemoryUtilization(i+1, memory_used, memory_total, &mem_info);

//         // Print memory usage in kilobytes after
//         for (j = i+1; j < samples; j++){
//             printf("\n");
//         }

//         printf("---------------------------------------\n");
//         printf("Number of cores: %d \n", get_nprocs());
//         printf("total cpu use = %.2f%%\n", cpu_utilization[i].utilization);
        
//         // printCPUUtilization(samples, tdelay);
//         for (int k = 0; k <= i; k++) {
//             if (cpu_utilization[k].num_bars > 0) {
//                 printf("\t");
//                 for (int j = 0; j < cpu_utilization[k].num_bars; j++) {
//                     printf("|");
//                 }
//                 printf(" %.2f\n", cpu_utilization[k].utilization);
//             } else {
//                 printf("\t %.2f\n", cpu_utilization[k].utilization);
//             }
//         }

//         //print the rest of the information
//         printUsers();
        
        
//         printMachineInfo();

//         clear_screen();
//     }
// }