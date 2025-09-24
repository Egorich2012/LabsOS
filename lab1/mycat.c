#include "mycat.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static void process_file(FILE *file, int n_flag, int b_flag, int e_flag) {
    char line[1024];
    int line_num = 1;
    
    while (fgets(line, sizeof(line), file)) {
        int print_number = 0;
        size_t len = strlen(line);
        
        if (b_flag) {
            if (len > 1 || (len == 1 && line[0] != '\n')) {
                print_number = 1;
            }
        } 
        else if (n_flag) {
            print_number = 1;
        }
        
        if (print_number) {
            printf("%6d\t", line_num++);
        } else if (b_flag || n_flag) {
            line_num++;
        }
        
        if (e_flag && len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            printf("%s$\n", line);
        } else {
            printf("%s", line);
        }
    }
}

int mycat(int argc, char *argv[]) {
    int n_flag = 0, b_flag = 0, e_flag = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            char *arg = argv[i];
            for (int j = 1; arg[j] != '\0'; j++) {
                if (arg[j] == 'n') n_flag = 1;
                else if (arg[j] == 'b') b_flag = 1;
                else if (arg[j] == 'E') e_flag = 1;
                else {
                    fprintf(stderr, "mycat: invalid option -- '%c'\n", arg[j]);
                    return 1;
                }
            }
        }
    }
    
    int file_count = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            FILE *file = fopen(argv[i], "r");
            if (!file) {
                fprintf(stderr, "mycat: cannot open '%s'\n", argv[i]);
                continue;
            }
            process_file(file, n_flag, b_flag, e_flag);
            fclose(file);
            file_count++;
        }
    }
    
    if (file_count == 0) {
        process_file(stdin, n_flag, b_flag, e_flag);
    }
    
    return 0;
}