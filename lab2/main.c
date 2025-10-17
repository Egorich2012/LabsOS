#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define MAX_FILES 1024
#define MAX_NAME_LEN 256

// Цветовые константы как у друга
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREEN "\033[32m"

struct FileInfo
{
    char name[MAX_NAME_LEN];
    struct stat st;
};

int compare_files(const void *f1, const void *f2)
{
    const struct FileInfo *file1 = (const struct FileInfo *)f1;
    const struct FileInfo *file2 = (const struct FileInfo *)f2;
    return strcmp(file1->name, file2->name);
}

// Функция получения цвета как у друга
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

// Функция прав доступа как у друга
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

void print_simple_format(struct FileInfo* files, int count, const char* path)
{
    for (int i = 0; i < count; i++)
    {
        const char* color = get_color(files[i].name, path);
        printf("%s%s%s  ", color, files[i].name, COLOR_RESET);
    }
    printf("\n");
}

void print_long_format(struct FileInfo* files, int count, const char* path)
{
    for (int i = 0; i < count; i++)
    {
        char time_buf[20];
        struct tm* timeinfo = localtime(&files[i].st.st_mtime);
        strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", timeinfo);
        
        const char* color = get_color(files[i].name, path);
        
        printf("%s %2d %6ld %s %s%s%s\n", 
               get_permissions(files[i].st.st_mode),
               (int)files[i].st.st_nlink,
               files[i].st.st_size,
               time_buf,
               color, files[i].name, COLOR_RESET);
    }
}

void list_dir(const char* path, bool flag_a, bool flag_l, bool print_header) {
    DIR* dp;
    struct dirent* entry;
    struct FileInfo files[MAX_FILES];
    int count = 0;
    
    dp = opendir(path);
    if (dp == NULL)
    {
        perror("myls");
        return;
    }
    
    while ((entry = readdir(dp)) != NULL && count < MAX_FILES) {
        if (!flag_a && entry->d_name[0] == '.')
        {
            continue;
        }
        
        strncpy(files[count].name, entry->d_name, sizeof(files[count].name) - 1);
        files[count].name[sizeof(files[count].name) - 1] = '\0';
        
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        
        if (stat(fullpath, &files[count].st) == -1)
        {
            continue;
        }
        
        count++;
    }
    
    closedir(dp);
    
    qsort(files, count, sizeof(struct FileInfo), compare_files);
    
    if (print_header)
    {
        printf("%s:\n", path);
    }
    
    if (flag_l)
    {
        print_long_format(files, count, path);
    }
    else
    {
        print_simple_format(files, count, path);
    }
}

int main(int argc, char* argv[]) {
    bool flag_a = false;
    bool flag_l = false;
    int opt;
    
    while ((opt = getopt(argc, argv, "al")) != -1)
    {
        switch (opt)
        {
            case 'a':
                flag_a = true;
                break;
            case 'l':
                flag_l = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-a] [-l] [directory...]\n", argv[0]);
                return 1;
        }
    }
    
    char* paths[100];
    int path_count = 0;
    
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
        
        list_dir(paths[p], flag_a, flag_l, false);
        
        if (p < path_count - 1) {
            printf("\n");
        }
    }
    
    return 0;
}