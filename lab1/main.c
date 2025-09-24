#include <stdio.h>
#include <string.h>
#include "mycat.h"
#include "mygrep.h"

int main(int argc, char *argv[]) {
    if (argc > 0) {
        if (strstr(argv[0], "mycat") != NULL) {
            return mycat(argc, argv);
        }
        else if (strstr(argv[0], "mygrep") != NULL) {
            return mygrep(argc, argv);
        }
    }
    
    printf("Use: mycat or mygrep\n");
    return 1;
}