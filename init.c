#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <time.h>

#define TIMELINE_FILE "/mnt/disk/timeline.log"
#define STATE_FILE "/mnt/disk/active_version.txt"

int total_versions = 0;
int current_version = 0;

void load_disk_state() {
    FILE *f = fopen(TIMELINE_FILE, "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) { total_versions++; }
        fclose(f);
    }
    
    current_version = total_versions; 
    FILE *sf = fopen(STATE_FILE, "r");
    if (sf) {
        fscanf(sf, "%d", &current_version);
        fclose(sf);
    }
}

void save_active_state() {
    FILE *sf = fopen(STATE_FILE, "w");
    if (sf) {
        fprintf(sf, "%d\n", current_version);
        fclose(sf);
        sync(); // Force hardware write
    }
}

void get_version_data(int target_v, char *out_data) {
    strcpy(out_data, "[EMPTY]");
    FILE *f = fopen(TIMELINE_FILE, "r");
    if (!f) return;
    
    char line[512];
    int v; long t;
    while (fgets(line, sizeof(line), f)) {
        char *token = strtok(line, "|");
        if (!token) continue;
        v = atoi(token);
        
        token = strtok(NULL, "|");
        if (!token) continue;
        t = atol(token);
        
        token = strtok(NULL, "\n"); 
        if (token && v == target_v) {
            strcpy(out_data, token);
            break;
        }
    }
    fclose(f);
}

int main() {
    mount("proc", "/proc", "proc", 0, NULL);
    mount("sysfs", "/sys", "sysfs", 0, NULL);
    mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);

    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\n======================================================\n");
    printf("   CHRONOS OS v1.0 - PERSISTENT TIME-TRAVEL EDITION\n");
    printf("======================================================\n");

    mkdir("/mnt", 0777);
    mkdir("/mnt/disk", 0777);
    
    if (mount("/dev/sda", "/mnt/disk", "ext4", 0, NULL) == 0) {
        printf("[Hardware] Hard Drive Mounted Successfully!\n");
        load_disk_state();
        printf("[Chronos] Loaded %d versions from disk. Active: v%d\n\n", total_versions, current_version);
    } else {
        printf("[WARNING] Disk Mount Failed.\n\n");
    }

    char input[256];
    while (1) {
        printf("chronos (v%d)# ", current_version);
        if (fgets(input, sizeof(input), stdin) == NULL) continue;
        input[strcspn(input, "\r\n")] = 0; 

        if (strcmp(input, "help") == 0) {
            printf("  status          - Show active version data\n");
            printf("  write <text>    - Save new data permanently\n");
            printf("  timeline        - View full disk history\n");
            printf("  rewind <number> - Time-travel to a past version\n");
            printf("  poweroff        - Safely shut down\n");
        } 
        else if (strcmp(input, "status") == 0) {
            char data[256];
            get_version_data(current_version, data);
            printf("\n[System Status]\n");
            printf("  Active Version : v%d (of %d total)\n", current_version, total_versions);
            printf("  Current Data   : \"%s\"\n\n", data);
        } 
        else if (strncmp(input, "write ", 6) == 0) {
            FILE *f = fopen(TIMELINE_FILE, "a"); 
            if (f) {
                total_versions++;
                current_version = total_versions;
                time_t now = time(NULL);
                fprintf(f, "%d|%ld|%s\n", total_versions, (long)now, input + 6);
                fclose(f);
                
                save_active_state(); 
                
                printf("[Chronos] Version v%d locked and synced to physical disk.\n", current_version);
            } else {
                printf("[Error] Cannot write to disk.\n");
            }
        } 
        else if (strcmp(input, "timeline") == 0) {
            printf("\n--- CHRONOS IMMUTABLE TIMELINE (DISK) ---\n");
            FILE *f = fopen(TIMELINE_FILE, "r");
            if (f) {
                char line[512];
                int v; long t;
                while (fgets(line, sizeof(line), f)) {
                    char *token = strtok(line, "|");
                    if(!token) continue;
                    v = atoi(token);
                    
                    token = strtok(NULL, "|");
                    if(!token) continue;
                    t = atol(token);
                    
                    token = strtok(NULL, "\n");
                    if (token) {
                        printf("  [v%d] Time: %ld | Data: \"%s\" %s\n", 
                            v, t, token, (v == current_version) ? "<-- ACTIVE" : "");
                    }
                }
                fclose(f);
            }
            printf("-----------------------------------------\n\n");
        } 
        else if (strncmp(input, "rewind ", 7) == 0) {
            int target = atoi(input + 7);
            if (target >= 1 && target <= total_versions) {
                current_version = target;
                save_active_state(); 
                
                char data[256];
                get_version_data(current_version, data);
                printf("[Chronos] TIME-TRAVEL COMPLETE! Restored to v%d: \"%s\"\n", current_version, data);
            } else {
                printf("[Chronos] Invalid version!\n");
            }
        } 
        else if (strcmp(input, "poweroff") == 0) {
            printf("Flushing hardware caches and shutting down...\n");
            sync(); 
            reboot(RB_POWER_OFF);
        } 
        else if (strlen(input) > 0) {
            printf("Unknown command. Type 'help'.\n");
        }
    }
    return 0;
}
