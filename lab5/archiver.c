#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>

#define NAME_LIMIT 255

typedef struct {
    char filename[NAME_LIMIT];
    mode_t permissions;
    off_t filesize;
    time_t modtime;
} archive_entry;

void print_usage_info() {
    printf("Commands:\n");
    printf("  ./archiver archive -a filename       Insert file\n");
    printf("  ./archiver archive -x filename       Retrieve and delete file\n");
    printf("  ./archiver archive -l                Display contents\n");
    printf("  ./archiver --help                    Display this help\n");
}

int insert_file_to_archive(const char *archive_path, const char *target_file) {
    int archive_fd, source_fd;
    struct stat file_stat;
    archive_entry entry_data;
    char buffer_data[8192];
    ssize_t bytes_processed;
    
    if (stat(target_file, &file_stat) != 0) {
        perror("File check failed");
        return -1;
    }
    
    source_fd = open(target_file, O_RDONLY);
    if (source_fd < 0) {
        perror("Open source error");
        return -1;
    }
    
    archive_fd = open(archive_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (archive_fd < 0) {
        perror("Archive open error");
        close(source_fd);
        return -1;
    }
    
    memset(&entry_data, 0, sizeof(entry_data));
    strncpy(entry_data.filename, target_file, NAME_LIMIT-1);
    entry_data.permissions = file_stat.st_mode;
    entry_data.filesize = file_stat.st_size;
    entry_data.modtime = file_stat.st_mtime;
    
    if (write(archive_fd, &entry_data, sizeof(entry_data)) != sizeof(entry_data)) {
        perror("Header write error");
        close(archive_fd);
        close(source_fd);
        return -1;
    }
    
    while ((bytes_processed = read(source_fd, buffer_data, sizeof(buffer_data))) > 0) {
        if (write(archive_fd, buffer_data, bytes_processed) != bytes_processed) {
            perror("Data write error");
            close(archive_fd);
            close(source_fd);
            return -1;
        }
    }
    
    if (bytes_processed < 0) {
        perror("Read error");
    }
    
    close(archive_fd);
    close(source_fd);
    printf("Added: %s\n", target_file);
    return 0;
}

int retrieve_file_from_archive(const char *archive_path, const char *target_file) {
    int archive_fd, dest_fd, temp_fd;
    archive_entry entry_data;
    char buffer_data[8192];
    char temporary_name[] = "temp_archive_XXXXXX";
    int file_found = 0;
    ssize_t bytes_read;
    
    archive_fd = open(archive_path, O_RDONLY);
    if (archive_fd < 0) {
        perror("Archive open failed");
        return -1;
    }
    
    temp_fd = mkstemp(temporary_name);
    if (temp_fd < 0) {
        perror("Temp file creation");
        close(archive_fd);
        return -1;
    }
    
    while (read(archive_fd, &entry_data, sizeof(entry_data)) == sizeof(entry_data)) {
        if (strcmp(entry_data.filename, target_file) == 0) {
            file_found = 1;
            
            dest_fd = open(target_file, O_WRONLY | O_CREAT | O_TRUNC, entry_data.permissions);
            if (dest_fd < 0) {
                perror("Destination file create");
                close(archive_fd);
                close(temp_fd);
                remove(temporary_name);
                return -1;
            }
            
            off_t remaining = entry_data.filesize;
            while (remaining > 0) {
                size_t chunk = sizeof(buffer_data);
                if (remaining < chunk) chunk = remaining;
                
                bytes_read = read(archive_fd, buffer_data, chunk);
                if (bytes_read <= 0) break;
                
                if (write(dest_fd, buffer_data, bytes_read) != bytes_read) {
                    perror("File write");
                    close(dest_fd);
                    close(archive_fd);
                    close(temp_fd);
                    remove(temporary_name);
                    return -1;
                }
                remaining -= bytes_read;
            }
            
            struct utimbuf time_data = {entry_data.modtime, entry_data.modtime};
            utime(target_file, &time_data);
            chmod(target_file, entry_data.permissions);
            
            close(dest_fd);
            printf("Restored: %s\n", target_file);
            
        } else {
            write(temp_fd, &entry_data, sizeof(entry_data));
            
            off_t remaining = entry_data.filesize;
            while (remaining > 0) {
                size_t chunk = sizeof(buffer_data);
                if (remaining < chunk) chunk = remaining;
                
                bytes_read = read(archive_fd, buffer_data, chunk);
                if (bytes_read <= 0) break;
                
                if (write(temp_fd, buffer_data, bytes_read) != bytes_read) {
                    perror("Temp write");
                    close(archive_fd);
                    close(temp_fd);
                    remove(temporary_name);
                    return -1;
                }
                remaining -= bytes_read;
            }
        }
    }
    
    close(archive_fd);
    close(temp_fd);
    
    if (!file_found) {
        printf("Not in archive: %s\n", target_file);
        remove(temporary_name);
        return -1;
    }
    
    if (rename(temporary_name, archive_path) != 0) {
        perror("Archive replace");
        remove(temporary_name);
        return -1;
    }
    
    printf("Removed from archive: %s\n", target_file);
    return 0;
}

void list_archive_contents(const char *archive_path) {
    int archive_fd;
    archive_entry entry_data;
    struct stat archive_stat;
    int file_count = 0;
    off_t total_data = 0;
    
    if (stat(archive_path, &archive_stat) != 0) {
        perror("Archive stat");
        return;
    }
    
    archive_fd = open(archive_path, O_RDONLY);
    if (archive_fd < 0) {
        perror("Archive open");
        return;
    }
    
    printf("Archive file: %s\n", archive_path);
    printf("Total archive size: %ld bytes\n\n", archive_stat.st_size);
    printf("%-25s %12s %12s\n", "FILENAME", "SIZE(bytes)", "PERMISSIONS");
    
    while (read(archive_fd, &entry_data, sizeof(entry_data)) == sizeof(entry_data)) {
        file_count++;
        total_data += entry_data.filesize;
        printf("%-25s %12ld %12o\n", entry_data.filename, entry_data.filesize, entry_data.permissions);
        
        if (lseek(archive_fd, entry_data.filesize, SEEK_CUR) < 0) {
            perror("Seek error");
            break;
        }
    }
    
    printf("\nSummary: %d files, %ld bytes of data\n", file_count, total_data);
    
    close(archive_fd);
}

int process_arguments(int arg_count, char *arg_array[]) {
    if (arg_count < 2) {
        print_usage_info();
        return 1;
    }
    
    if (strcmp(arg_array[1], "--help") == 0) {
        print_usage_info();
        return 0;
    }
    
    if (arg_count < 3) {
        print_usage_info();
        return 1;
    }
    
    char *archive_filename = arg_array[1];
    char *operation = arg_array[2];
    
    if (strcmp(operation, "-a") == 0) {
        if (arg_count < 4) {
            printf("Specify filename to add\n");
            return 1;
        }
        return insert_file_to_archive(archive_filename, arg_array[3]);
    }
    else if (strcmp(operation, "-x") == 0) {
        if (arg_count < 4) {
            printf("Specify filename to extract\n");
            return 1;
        }
        return retrieve_file_from_archive(archive_filename, arg_array[3]);
    }
    else if (strcmp(operation, "-l") == 0) {
        list_archive_contents(archive_filename);
        return 0;
    }
    else {
        printf("Invalid operation\n");
        print_usage_info();
        return 1;
    }
}

int main(int arg_count, char *arg_array[]) {
    return process_arguments(arg_count, arg_array);
}