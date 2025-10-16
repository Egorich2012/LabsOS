#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <getopt.h>

#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREEN "\033[32m"

const char* get_color(const char* filename, const char* path) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
    
    struct stat st;
    if (stat(full_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return COLOR_BLUE;
        if (st.st_mode & S_IXUSR) return COLOR_GREEN;
    }
    
    return COLOR_RESET;
}

char* get_permissions(mode_t mode) {
    static char permissions[11];
    strcpy(permissions, "----------");
    
    if (S_ISDIR(mode)) permissions[0] = 'd';
    
    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';
    
    return permissions;
}

int compare_strings(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

int main(int argc, char* argv[]) {
    int show_all = 0;
    int long_format = 0;
    char* paths[100];
    int path_count = 0;
    
    int opt;
    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch (opt) {
            case 'l': long_format = 1; break;
            case 'a': show_all = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-a] [path...]\n", argv[0]);
                return 1;
        }
    }
    
    for (int i = optind; i < argc; i++) {
        paths[path_count++] = argv[i];
    }
    if (path_count == 0) {
        paths[path_count++] = ".";
    }
    
    for (int p = 0; p < path_count; p++) {
        if (path_count > 1) {
            printf("%s:\n", paths[p]);
        }
        
        DIR* dir = opendir(paths[p]);
        if (!dir) {
            perror("myls");
            continue;
        }
        
        char* files[1000];
        int file_count = 0;
        struct dirent* entry;
        
        while ((entry = readdir(dir)) != NULL && file_count < 1000) {
            if (!show_all && entry->d_name[0] == '.') continue;
            files[file_count] = strdup(entry->d_name);
            file_count++;
        }
        closedir(dir);
        
        qsort(files, file_count, sizeof(char*), compare_strings);
        
        if (long_format) {
            for (int i = 0; i < file_count; i++) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", paths[p], files[i]);
                
                struct stat st;
                if (stat(full_path, &st) == 0) {
                    const char* color = get_color(files[i], paths[p]);
                    char time_buf[20];
                    struct tm* timeinfo = localtime(&st.st_mtime);
                    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
                    
                    printf("%s %2d %6ld %s %s%s%s\n", 
                           get_permissions(st.st_mode),
                           (int)st.st_nlink,
                           st.st_size,
                           time_buf,
                           color, files[i], COLOR_RESET);
                }
                free(files[i]);
            }
        } else {
            for (int i = 0; i < file_count; i++) {
                const char* color = get_color(files[i], paths[p]);
                printf("%s%s%s  ", color, files[i], COLOR_RESET);
                free(files[i]);
            }
            printf("\n");
        }
        
        if (p < path_count - 1) {
            printf("\n");
        }
    }
    
    return 0;
}