#ifndef MYCAT_H
#define MYCAT_H

int parse_cat_flags(int argc, char *argv[], int *number_nonblank, int *number, int *show_ends);

int mycat(int argc, char *argv[]);

#endif