#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

void apply_numeric_mode(const char *mode_string, const char *file_path) {
    int permission_value = strtol(mode_string, NULL, 8);
    if (chmod(file_path, permission_value) == -1) {
        perror("Error applying permissions");
        exit(EXIT_FAILURE);
    }
    printf("File %s permissions updated to %s\n", file_path, mode_string);
}

void parse_symbolic_permissions(const char *perm_string, const char *file_path) {
    struct stat file_info;
    if (stat(file_path, &file_info) == -1) {
        perror("Error reading file info");
        exit(EXIT_FAILURE);
    }
    
    mode_t current_perms = file_info.st_mode;
    mode_t updated_perms = current_perms;
    const char *parser = perm_string;

    while (*parser) {
        char scope = *parser;
        char operation;
        char permission_chars[4] = {0};

        if (strchr("ugoa", scope)) {
            parser++;
        } else {
            break;
        }
        operation = *parser++;

        int char_index = 0;
        while (*parser && strchr("rwx", *parser) && char_index < 3) {
            permission_chars[char_index++] = *parser++;
        }
        
        mode_t permission_mask = 0;

        for (int idx = 0; idx < char_index; idx++) {
            if (permission_chars[idx] == 'r') {
                if (scope == 'u' || scope == 'a') permission_mask |= S_IRUSR;
                if (scope == 'g' || scope == 'a') permission_mask |= S_IRGRP;
                if (scope == 'o' || scope == 'a') permission_mask |= S_IROTH;
            }
            if (permission_chars[idx] == 'w') {
                if (scope == 'u' || scope == 'a') permission_mask |= S_IWUSR;
                if (scope == 'g' || scope == 'a') permission_mask |= S_IWGRP;
                if (scope == 'o' || scope == 'a') permission_mask |= S_IWOTH;
            }
            if (permission_chars[idx] == 'x') {
                if (scope == 'u' || scope == 'a') permission_mask |= S_IXUSR;
                if (scope == 'g' || scope == 'a') permission_mask |= S_IXGRP;
                if (scope == 'o' || scope == 'a') permission_mask |= S_IXOTH;
            }
        }

        switch (operation) {
            case '+':
                updated_perms |= permission_mask;
                break;
            case '-':
                updated_perms &= ~permission_mask;
                break;
            case '=':
                if (scope == 'u' || scope == 'a') updated_perms &= ~(S_IRWXU);
                if (scope == 'g' || scope == 'a') updated_perms &= ~(S_IRWXG);
                if (scope == 'o' || scope == 'a') updated_perms &= ~(S_IRWXO);
                updated_perms |= permission_mask;
                break;
        }
    }
    
    if (chmod(file_path, updated_perms) == -1) {
        perror("Error updating permissions");
        exit(EXIT_FAILURE);
    }
    printf("File %s permissions updated to %s\n", file_path, perm_string);
}

void display_current_permissions(const char *file_path) {
    struct stat file_info;
    if (stat(file_path, &file_info) == -1) {
        perror("Error reading file status");
        return;
    }
    printf("Current permissions for %s: %o\n", file_path, file_info.st_mode & 0777);
}

int main(int arg_count, char *arg_values[]) {
    if (arg_count != 3) {
        fprintf(stderr, "Usage: %s <permissions> <filename>\n", arg_values[0]);
        exit(EXIT_FAILURE);
    }
    
    char *permission_string = arg_values[1];
    char *target_file = arg_values[2];

    display_current_permissions(target_file);

    if (permission_string[0] >= '0' && permission_string[0] <= '7') {
        apply_numeric_mode(permission_string, target_file);
    } else {
        parse_symbolic_permissions(permission_string, target_file);
    }
    
    display_current_permissions(target_file);
    
    return EXIT_SUCCESS;
}