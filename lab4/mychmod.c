#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void display_usage(const char *program_name) {
    printf("Usage: %s MODE FILE...\n", program_name);
}

int parse_octal_mode(const char *mode_string) {
    char *end_ptr;
    long value = strtol(mode_string, &end_ptr, 8);
    if (*end_ptr != '\0' || value < 0 || value > 0777)
        return -1;
    return (int)value;
}

mode_t get_current_permissions(const char *filename) {
    struct stat file_info;
    if (stat(filename, &file_info) == -1)
        return (mode_t)-1;
    return file_info.st_mode & 0777;
}

int apply_symbolic_mode(const char *specification, mode_t current_mode) {
    mode_t new_mode = current_mode;
    const char *cursor = specification;
    mode_t affected_targets = 0;
    mode_t permission_bits = 0;
    char operation = 0;

    if (*cursor == '+' || *cursor == '-' || *cursor == '=') {
        affected_targets = S_IRWXU | S_IRWXG | S_IRWXO;
    } else {
        while (*cursor && *cursor != '+' && *cursor != '-' && *cursor != '=') {
            switch (*cursor) {
                case 'u': affected_targets |= S_IRWXU; break;
                case 'g': affected_targets |= S_IRWXG; break;
                case 'o': affected_targets |= S_IRWXO; break;
                case 'a': affected_targets |= S_IRWXU | S_IRWXG | S_IRWXO; break;
                default: return -1;
            }
            cursor++;
        }
    }

    if (*cursor == '\0') return -1;
    operation = *cursor++;

    while (*cursor) {
        switch (*cursor) {
            case 'r':
                if (affected_targets & S_IRWXU) permission_bits |= S_IRUSR;
                if (affected_targets & S_IRWXG) permission_bits |= S_IRGRP;
                if (affected_targets & S_IRWXO) permission_bits |= S_IROTH;
                break;
            case 'w':
                if (affected_targets & S_IRWXU) permission_bits |= S_IWUSR;
                if (affected_targets & S_IRWXG) permission_bits |= S_IWGRP;
                if (affected_targets & S_IRWXO) permission_bits |= S_IWOTH;
                break;
            case 'x':
                if (affected_targets & S_IRWXU) permission_bits |= S_IXUSR;
                if (affected_targets & S_IRWXG) permission_bits |= S_IXGRP;
                if (affected_targets & S_IRWXO) permission_bits |= S_IXOTH;
                break;
            default:
                return -1;
        }
        cursor++;
    }

    switch (operation) {
        case '+':
            new_mode |= permission_bits;
            break;
        case '-':
            new_mode &= ~permission_bits;
            break;
        case '=':
            if (affected_targets & S_IRWXU) new_mode &= ~S_IRWXU;
            if (affected_targets & S_IRWXG) new_mode &= ~S_IRWXG;
            if (affected_targets & S_IRWXO) new_mode &= ~S_IRWXO;
            new_mode |= permission_bits;
            break;
        default:
            return -1;
    }

    return new_mode;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        display_usage(argv[0]);
        return 1;
    }

    char *mode_argument = argv[1];
    int target_mode = -1;

    if (mode_argument[0] >= '0' && mode_argument[0] <= '7') {
        target_mode = parse_octal_mode(mode_argument);
        if (target_mode == -1) {
            fprintf(stderr, "mychmod: invalid numeric mode '%s'\n", mode_argument);
            return 1;
        }
        for (int i = 2; i < argc; i++) {
            if (chmod(argv[i], target_mode) == -1) {
                fprintf(stderr, "mychmod: cannot change permissions of '%s': %s\n",
                        argv[i], strerror(errno));
                return 1;
            }
        }
        return 0;
    }

    mode_t base_permissions = get_current_permissions(argv[2]);
    if (base_permissions == (mode_t)-1) {
        fprintf(stderr, "mychmod: cannot read permissions of '%s': %s\n",
                argv[2], strerror(errno));
        return 1;
    }

    target_mode = apply_symbolic_mode(mode_argument, base_permissions);
    if (target_mode == -1) {
        fprintf(stderr, "mychmod: invalid symbolic mode '%s'\n", mode_argument);
        return 1;
    }

    for (int i = 2; i < argc; i++) {
        if (chmod(argv[i], target_mode) == -1) {
            fprintf(stderr, "mychmod: cannot change permissions of '%s': %s\n",
                    argv[i], strerror(errno));
            continue;
        }
    }

    return 0;
}
