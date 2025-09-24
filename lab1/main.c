#include <stdio.h>
#include <string.h>
#include "mycat.h"
#include "mygrep.h"


int main(int argc, char *argv[]) {
    if (argc > 0) {
        if (strstr(argv[0], "mycat") != NULL) {
            #ifdef COMPILE_CAT
            return mycat(argc, argv);
            #else
            printf("mycat not compiled\n");
            return 1;
            #endif
        }
        else if (strstr(argv[0], "mygrep") != NULL) {
            #ifdef COMPILE_GREP
            return mygrep(argc, argv);
            #else
            printf("mygrep not compiled\n");
            return 1;
            #endif
        }
    }
    
    printf("Use: mycat or mygrep\n");
    return 1;
}