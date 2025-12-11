#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

int parse_octal(const char *mode_str, mode_t *new_mode) {
    if (strlen(mode_str) > 4) return 0;
    char *end;
    long val = strtol(mode_str, &end, 8);
    if (*end != '\0' || val < 0 || val > 07777) return 0;
    *new_mode = (mode_t)val;
    return 1;
}

int parse_symbolic(const char *mode_str, mode_t *new_mode, mode_t current_mode) {
    mode_t result = current_mode;
    const char *p = mode_str;
    
    while (*p) {
        mode_t who = 0;
        
        while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
            if (*p == 'u') who |= S_IRWXU;
            if (*p == 'g') who |= S_IRWXG;
            if (*p == 'o') who |= S_IRWXO;
            if (*p == 'a') who |= (S_IRWXU | S_IRWXG | S_IRWXO);
            p++;
        }
        
        if (who == 0) who = S_IRWXU | S_IRWXG | S_IRWXO;
        
        if (*p != '+' && *p != '-' && *p != '=') return 0;
        char op = *p;
        p++;
        
        mode_t perm = 0;
        while (*p && *p != ',') {
            switch (*p) {
                case 'r': perm |= S_IRUSR | S_IRGRP | S_IROTH; break;
                case 'w': perm |= S_IWUSR | S_IWGRP | S_IWOTH; break;
                case 'x': perm |= S_IXUSR | S_IXGRP | S_IXOTH; break;
                default: return 0;
            }
            p++;
        }
        
        mode_t affected = perm & who;
        
        if (op == '+') {
            result |= affected;
        } else if (op == '-') {
            result &= ~affected;
        } else if (op == '=') {
            result &= ~who;
            result |= affected;
        }
        
        if (*p == ',') p++;
    }
    
    *new_mode = result;
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mode> <file>...\n", argv[0]);
        exit(1);
    }

    const char *mode_str = argv[1];
    mode_t new_mode = 0;
    int is_octal = 1;

    for (int i = 0; mode_str[i] != '\0'; i++) {
        if (mode_str[i] < '0' || mode_str[i] > '7') {
            is_octal = 0;
            break;
        }
    }

    struct stat st;
    if (!is_octal) {
        if (stat(argv[2], &st) == -1) {
            perror(argv[2]);
            exit(1);
        }
        if (!parse_symbolic(mode_str, &new_mode, st.st_mode)) {
            fprintf(stderr, "%s: Invalid mode: %s\n", argv[0], mode_str);
            exit(1);
        }
    } else {
        if (!parse_octal(mode_str, &new_mode)) {
            fprintf(stderr, "%s: Invalid mode: %s\n", argv[0], mode_str);
            exit(1);
        }
    }

    for (int i = 2; i < argc; i++) {
        if (chmod(argv[i], new_mode) == -1) {
            perror(argv[i]);
        }
    }

    return 0;
}