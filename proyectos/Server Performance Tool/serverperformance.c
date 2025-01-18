#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
   unsigned long user;
   unsigned long nice;
   unsigned long system;
   unsigned long idle;
   unsigned long iowait;
   unsigned long irq;
   unsigned long softirq;
   unsigned long steal;
   unsigned long guest;
   unsigned long guest_nice;
} CPUStats;

void get_cpu_stats(CPUStats *stats) {
   FILE *file = fopen("/proc/stat", "r");
   if (!file) {
       perror("Error al abrir /proc/stat");
       exit(1);
   }

   char line[256];
   if (fgets(line, sizeof(line), file)) {
       sscanf(line, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
              &stats->user, &stats->nice, &stats->system, &stats->idle,
              &stats->iowait, &stats->irq, &stats->softirq, &stats->steal,
              &stats->guest, &stats->guest_nice);
   }
   fclose(file);
}

float calculate_cpu_usage(CPUStats *prev, CPUStats *curr) {
   unsigned long prev_idle = prev->idle + prev->iowait;
   unsigned long curr_idle = curr->idle + curr->iowait;

   unsigned long prev_total = prev_idle + prev->user + prev->nice + prev->system +
                            prev->irq + prev->softirq + prev->steal;
   unsigned long curr_total = curr_idle + curr->user + curr->nice + curr->system +
                            curr->irq + curr->softirq + curr->steal;

   unsigned long total_diff = curr_total - prev_total;
   unsigned long idle_diff = curr_idle - prev_idle;

   return total_diff ? (float)(total_diff - idle_diff) * 100 / total_diff : 0;
}

int main() {
   FILE *mem_file;
   char line[256];
   unsigned long mem_total = 0, mem_free = 0, mem_available = 0;
   unsigned long buffers = 0, cached = 0, sreclaimable = 0, swap_total = 0, swap_free = 0;
   unsigned long active = 0, inactive = 0, shmem = 0;

   CPUStats prev_stats, curr_stats;
   get_cpu_stats(&prev_stats);
   sleep(1);

   get_cpu_stats(&curr_stats);
   float cpu_usage = calculate_cpu_usage(&prev_stats, &curr_stats);

   printf("\n=== ESTADÍSTICAS DE CPU ===\n");
   printf("Uso de CPU: %.1f%%\n", cpu_usage);

   FILE *cpu_info = fopen("/proc/cpuinfo", "r");
   if (cpu_info) {
       int cpu_count = 0;
       char cpu_model[256] = "";
       float mhz = 0.0;

       while (fgets(line, sizeof(line), cpu_info)) {
           if (strncmp(line, "model name", 10) == 0) {
               char *model = strchr(line, ':');
               if (model) {
                   strcpy(cpu_model, model + 2);
                   cpu_model[strlen(cpu_model)-1] = '\0';
               }
               cpu_count++;
           }
           if (strncmp(line, "cpu MHz", 7) == 0) {
               sscanf(line, "cpu MHz : %f", &mhz);
           }
       }
       printf("Modelo CPU: %s\n", cpu_model);
       printf("Núcleos CPU: %d\n", cpu_count);
       printf("Frecuencia CPU: %.0f MHz\n", mhz);
       fclose(cpu_info);
   }

   mem_file = fopen("/proc/meminfo", "r");
   if (!mem_file) {
       perror("Error al abrir /proc/meminfo");
       return 1;
   }

   while (fgets(line, sizeof(line), mem_file)) {
       if (strncmp(line, "MemTotal:", 9) == 0) {
           sscanf(line, "MemTotal: %lu kB", &mem_total);
       } else if (strncmp(line, "MemFree:", 8) == 0) {
           sscanf(line, "MemFree: %lu kB", &mem_free);
       } else if (strncmp(line, "MemAvailable:", 12) == 0) {
           sscanf(line, "MemAvailable: %lu kB", &mem_available);
       } else if (strncmp(line, "Buffers:", 8) == 0) {
           sscanf(line, "Buffers: %lu kB", &buffers);
       } else if (strncmp(line, "Cached:", 7) == 0) {
           sscanf(line, "Cached: %lu kB", &cached);
       } else if (strncmp(line, "SwapTotal:", 10) == 0) {
           sscanf(line, "SwapTotal: %lu kB", &swap_total);
       } else if (strncmp(line, "SwapFree:", 9) == 0) {
           sscanf(line, "SwapFree: %lu kB", &swap_free);
       } else if (strncmp(line, "Active:", 7) == 0) {
           sscanf(line, "Active: %lu kB", &active);
       } else if (strncmp(line, "Inactive:", 9) == 0) {
           sscanf(line, "Inactive: %lu kB", &inactive);
       } else if (strncmp(line, "Shmem:", 6) == 0) {
           sscanf(line, "Shmem: %lu kB", &shmem);
       } else if (strncmp(line, "SReclaimable:", 12) == 0) {
           sscanf(line, "SReclaimable: %lu kB", &sreclaimable);
       }
   }

   fclose(mem_file);

   unsigned long cached_total = cached + sreclaimable + buffers;
   unsigned long mem_used = mem_total - mem_free - cached_total;
   unsigned long swap_used = swap_total - swap_free;
   double mem_used_percent = (double)mem_used / mem_total * 100;
   double swap_used_percent = swap_total ? ((double)swap_used / swap_total * 100) : 0;

   printf("\n=== ESTADÍSTICAS DE MEMORIA ===\n");
   printf("\nRAM:\n");
   printf("Memoria Total:    %8.1f MB\n", mem_total / 1024.0);
   printf("Memoria Usada:    %8.1f MB (%.1f%%)\n", mem_used / 1024.0, mem_used_percent);
   printf("Memoria Libre:    %8.1f MB\n", mem_free / 1024.0);
   printf("Memoria Disponible:%8.1f MB\n", mem_available / 1024.0);

   printf("\nCACHÉ Y BUFFERS:\n");
   printf("Total en Caché:  %8.1f MB\n", cached_total / 1024.0);

   printf("\nSWAP:\n");
   printf("Swap Total:      %8.1f MB\n", swap_total / 1024.0);
   printf("Swap Usado:      %8.1f MB (%.1f%%)\n", swap_used / 1024.0, swap_used_percent);
   printf("Swap Libre:      %8.1f MB\n", swap_free / 1024.0);

   printf("\nESTADOS DE MEMORIA:\n");
   printf("Activa:          %8.1f MB\n", active / 1024.0);
   printf("Inactiva:        %8.1f MB\n", inactive / 1024.0);
   printf("Memoria Compartida:%8.1f MB\n", shmem / 1024.0);

   return 0;
}