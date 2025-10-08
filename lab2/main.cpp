#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"

const char* get_color(const char* filename, const char* path) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
    
    DIR* test_dir = opendir(full_path);
    if (test_dir) {
        closedir(test_dir);
        return COLOR_BLUE;
    }
    
    if (strstr(filename, ".exe") != NULL || strstr(filename, ".bat") != NULL) {
        return COLOR_GREEN;
    }
    
    return COLOR_RESET;
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
        }
    }
    
    for (int i = optind; i < argc; i++) {
        paths[path_count++] = argv[i];
    }
    if (path_count == 0) {
        paths[path_count++] = ".";
    }
    
    DIR* dir = opendir(paths[0]);
    if (!dir) {
        perror("myls");
        return 1;
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
    
    for (int i = 0; i < file_count; i++) {
        const char* color = get_color(files[i], paths[0]);
        printf("%s%s%s ", color, files[i], COLOR_RESET);
        free(files[i]);
    }
    printf("\n");
    
    return 0;
}