#include "mygrep.h"
#include <stdio.h>
#include <string.h>

int mygrep(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "mygrep: need pattern argument\n");
        return 2;
    }
    
    char *search_pattern = argv[1];
    FILE *file = stdin;
    int use_stdin = 1;

    if (argc >= 3) {
        file = fopen(argv[2], "r");
        if (file == NULL) {
            fprintf(stderr, "mygrep: can't read '%s'\n", argv[2]);
            return 2;
        }
        use_stdin = 0;
    }

    char text_line[1024];
    int matches_count = 0;
    int line_number = 0;

    while (fgets(text_line, sizeof(text_line), file)) {
        line_number++;
        
        char *match_position = text_line;
        while (*match_position != '\0') {
            if (strncmp(match_position, search_pattern, strlen(search_pattern))) {
                match_position++;
            } else {
                printf("%s", text_line);
                matches_count++;
                break;
            }
        }
    }

    if (!use_stdin) {
        fclose(file);
    }

    if (matches_count > 0) {
        return 0;
    } else {
        return 1;
    }
}