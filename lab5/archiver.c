#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define BUFFER_SIZE 4096
#define PATH_LIMIT 255

typedef struct {
    char filename[PATH_LIMIT];
    unsigned int permissions;
    long file_length;
    time_t modification_time;
} archive_entry;

void display_usage() {
    puts("Usage:");
    puts("  ./archiver <archive> --input <file>    Insert file into archive");
    puts("  ./archiver <archive> --extract <file>  Remove and extract file");
    puts("  ./archiver <archive> --stat            Display archive contents");
    puts("  ./archiver --help                      Display this message");
}

int insert_file(const char* archive_path, const char* source_path) {
    int archive_fd, source_fd;
    struct stat file_info;
    archive_entry entry_data;
    char data_buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    if (stat(source_path, &file_info) != 0) {
        perror("Failed to get file info");
        return -1;
    }
    
    source_fd = open(source_path, O_RDONLY);
    if (source_fd < 0) {
        perror("Failed to open source file");
        return -1;
    }
    
    archive_fd = open(archive_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (archive_fd < 0) {
        perror("Failed to open archive");
        close(source_fd);
        return -1;
    }
    
    memset(&entry_data, 0, sizeof(entry_data));
    strncpy(entry_data.filename, source_path, PATH_LIMIT - 1);
    entry_data.permissions = file_info.st_mode;
    entry_data.file_length = file_info.st_size;
    entry_data.modification_time = file_info.st_mtime;
    
    if (write(archive_fd, &entry_data, sizeof(entry_data)) != sizeof(entry_data)) {
        perror("Failed to write header");
        close(archive_fd);
        close(source_fd);
        return -1;
    }
    
    while ((bytes_read = read(source_fd, data_buffer, BUFFER_SIZE)) > 0) {
        if (write(archive_fd, data_buffer, bytes_read) != bytes_read) {
            perror("Failed to write data");
            close(archive_fd);
            close(source_fd);
            return -1;
        }
    }
    
    close(archive_fd);
    close(source_fd);
    printf("Added: %s\n", source_path);
    return 0;
}

int remove_and_extract(const char* archive_path, const char* target_filename) {
    int archive_fd, target_fd, temp_fd;
    archive_entry current_entry;
    char data_buffer[BUFFER_SIZE];
    char temp_filename[] = "temp_archive_XXXXXX";
    int file_found = 0;
    ssize_t bytes_processed;
    
    archive_fd = open(archive_path, O_RDONLY);
    if (archive_fd < 0) {
        perror("Failed to open archive");
        return -1;
    }
    
    temp_fd = mkstemp(temp_filename);
    if (temp_fd < 0) {
        perror("Failed to create temporary file");
        close(archive_fd);
        return -1;
    }
    
    while (read(archive_fd, &current_entry, sizeof(current_entry)) == sizeof(current_entry)) {
        if (strcmp(current_entry.filename, target_filename) == 0) {
            file_found = 1;
            
            target_fd = open(target_filename, O_WRONLY | O_CREAT | O_TRUNC, current_entry.permissions);
            if (target_fd < 0) {
                perror("Failed to create target file");
                close(archive_fd);
                close(temp_fd);
                unlink(temp_filename);
                return -1;
            }
            
            for (long bytes_remaining = current_entry.file_length; bytes_remaining > 0;) {
                bytes_processed = read(archive_fd, data_buffer, 
                    bytes_remaining < BUFFER_SIZE ? bytes_remaining : BUFFER_SIZE);
                if (bytes_processed <= 0) break;
                write(target_fd, data_buffer, bytes_processed);
                bytes_remaining -= bytes_processed;
            }
            
            struct utimbuf time_settings = {current_entry.modification_time, current_entry.modification_time};
            utime(target_filename, &time_settings);
            chmod(target_filename, current_entry.permissions);
            
            close(target_fd);
            printf("Extracted: %s\n", target_filename);
            
        } else {
            write(temp_fd, &current_entry, sizeof(current_entry));
            
            for (long bytes_remaining = current_entry.file_length; bytes_remaining > 0;) {
                bytes_processed = read(archive_fd, data_buffer, 
                    bytes_remaining < BUFFER_SIZE ? bytes_remaining : BUFFER_SIZE);
                if (bytes_processed <= 0) break;
                write(temp_fd, data_buffer, bytes_processed);
                bytes_remaining -= bytes_processed;
            }
        }
    }
    
    close(archive_fd);
    close(temp_fd);
    
    if (!file_found) {
        printf("File not found: %s\n", target_filename);
        unlink(temp_filename);
        return -1;
    }
    
    if (rename(temp_filename, archive_path) != 0) {
        perror("Failed to update archive");
        unlink(temp_filename);
        return -1;
    }
    
    printf("Removed from archive: %s\n", target_filename);
    return 0;
}

void display_archive_info(const char* archive_path) {
    int archive_fd;
    archive_entry current_entry;
    struct stat archive_info;
    int file_count = 0;
    long total_data = 0;
    
    if (stat(archive_path, &archive_info) != 0) {
        perror("Failed to get archive info");
        return;
    }
    
    archive_fd = open(archive_path, O_RDONLY);
    if (archive_fd < 0) {
        perror("Failed to open archive");
        return;
    }
    
    printf("Archive: %s\n", archive_path);
    printf("Archive size: %ld bytes\n\n", archive_info.st_size);
    printf("%-25s %12s %12s\n", "FILENAME", "SIZE", "PERMISSIONS");
    
    while (read(archive_fd, &current_entry, sizeof(current_entry)) == sizeof(current_entry)) {
        file_count++;
        total_data += current_entry.file_length;
        printf("%-25s %12ld %12o\n", current_entry.filename, 
               current_entry.file_length, current_entry.permissions);
        lseek(archive_fd, current_entry.file_length, SEEK_CUR);
    }
    
    printf("\nTotal files: %d, Combined data size: %ld bytes\n", file_count, total_data);
    
    close(archive_fd);
}

int main(int arg_count, char* arg_values[]) {
    if (arg_count < 2) {
        display_usage();
        return 1;
    }
    
    if (strcmp(arg_values[1], "--help") == 0 || strcmp(arg_values[1], "-h") == 0) {
        display_usage();
        return 0;
    }
    
    if (arg_count < 3) {
        display_usage();
        return 1;
    }
    
    char* archive_name = arg_values[1];
    char* operation = arg_values[2];
    
    if (strcmp(operation, "--input") == 0 || strcmp(operation, "-i") == 0) {
        if (arg_count < 4) {
            puts("Error: Missing filename argument");
            return 1;
        }
        return insert_file(archive_name, arg_values[3]);
    }
    else if (strcmp(operation, "--extract") == 0 || strcmp(operation, "-e") == 0) {
        if (arg_count < 4) {
            puts("Error: Missing filename argument");
            return 1;
        }
        return remove_and_extract(archive_name, arg_values[3]);
    }
    else if (strcmp(operation, "--stat") == 0 || strcmp(operation, "-s") == 0) {
        display_archive_info(archive_name);
        return 0;
    }
    else {
        puts("Error: Unknown command");
        display_usage();
        return 1;
    }
}