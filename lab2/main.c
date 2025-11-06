#define _GNU_SOURCE
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define RESET_COL "\033[0m"
#define FOLDER_COL "\033[1;34m"
#define RUNNABLE_COL "\033[1;32m"
#define SYMLINK_COL "\033[1;36m"

typedef struct {
    char *filename;
    struct stat meta;
} Item;

static int sorter(const void *x, const void *y);
static void show_access_rights(mode_t m, const char *p);
static void display_item(const Item *it, int detailed, int colored,
                        int nlinks_w, int user_w, int group_w,
                        int size_w, const char *p, uid_t u, gid_t g);
static void process_location(const char *loc, int detailed, int hidden, int multi);
static char *pick_color(const struct stat *s, int colored);
static int find_max_name_len(Item *list, int cnt);
static void compact_display(Item *list, int cnt, int colored, int width);

int main(int argc, char *argv[]) {
    int detailed = 0;
    int hidden = 0;
    int c;
    int colored = isatty(STDOUT_FILENO);

    while ((c = getopt(argc, argv, "la")) != -1) {
        switch (c) {
            case 'l':
                detailed = 1;
                break;
            case 'a':
                hidden = 1;
                break;
            default:
                fprintf(stderr, "How to use: %s [-la] [location...]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind == argc) {
        process_location(".", detailed, hidden, 0);
        return 0;
    }

    int multiple_locations = (argc - optind > 1);
    for (int i = optind; i < argc; i++) {
        struct stat s;
        if (stat(argv[i], &s) == -1) {
            perror(argv[i]);
            continue;
        }
        if (S_ISDIR(s.st_mode)) {
            process_location(argv[i], detailed, hidden, multiple_locations);
        } else {
            if (multiple_locations) {
                printf("%s:\n", argv[i]);
            }
            Item single = {.filename = argv[i], .meta = s};
            if (detailed) {
                display_item(&single, detailed, colored,
                            snprintf(NULL, 0, "%lu", (unsigned long)s.st_nlink),
                            snprintf(NULL, 0, "%d", (int)s.st_uid),
                            snprintf(NULL, 0, "%d", (int)s.st_gid),
                            snprintf(NULL, 0, "%lld", (long long)s.st_size),
                            argv[i], s.st_uid, s.st_gid);
            } else {
                const char *col = pick_color(&s, colored);
                printf("%s%s%s\n", col, argv[i], colored ? RESET_COL : "");
            }
            if (multiple_locations) {
                printf("\n");
            }
        }
    }
    return 0;
}

static int sorter(const void *x, const void *y) {
    const Item *fx = x;
    const Item *fy = y;
    return strcmp(fx->filename, fy->filename);
}

static void show_access_rights(mode_t m, const char *p) {
    putchar(S_ISDIR(m) ? 'd' : (S_ISLNK(m) ? 'l' : '-'));
    putchar(m & S_IRUSR ? 'r' : '-');
    putchar(m & S_IWUSR ? 'w' : '-');
    putchar(m & S_IXUSR ? 'x' : '-');
    putchar(m & S_IRGRP ? 'r' : '-');
    putchar(m & S_IWGRP ? 'w' : '-');
    putchar(m & S_IXGRP ? 'x' : '-');
    putchar(m & S_IROTH ? 'r' : '-');
    putchar(m & S_IWOTH ? 'w' : '-');
    putchar(m & S_IXOTH ? 'x' : '-');
#ifdef __APPLE__
    if (listxattr(p, NULL, 0, XATTR_NOFOLLOW) > 0) {
        putchar('@');
    } else {
        putchar(' ');
    }
#else
    putchar(' ');
#endif
}

static char *pick_color(const struct stat *s, int colored) {
    if (!colored) return "";
    if (S_ISDIR(s->st_mode)) return FOLDER_COL;
    if (S_ISLNK(s->st_mode)) return SYMLINK_COL;
    if (s->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return RUNNABLE_COL;
    return "";
}

static void display_item(const Item *it, int detailed, int colored,
                        int nlinks_w, int user_w, int group_w,
                        int size_w, const char *p, uid_t u, gid_t g) {
    const struct stat *s = &it->meta;
    const char *col = pick_color(s, colored);

    if (detailed) {
        show_access_rights(s->st_mode, p);
        printf("%*lu ", nlinks_w, (unsigned long)s->st_nlink);

        struct passwd *pw = getpwuid(u);
        char uid_buf[32];
        const char *user_name;
        if (pw) {
            user_name = pw->pw_name;
        } else {
            snprintf(uid_buf, sizeof(uid_buf), "%d", (int)u);
            user_name = uid_buf;
        }
        printf("%-*s ", user_w, user_name);

        struct group *gr = getgrgid(g);
        char gid_buf[32];
        const char *group_name;
        if (gr) {
            group_name = gr->gr_name;
        } else {
            snprintf(gid_buf, sizeof(gid_buf), "%d", (int)g);
            group_name = gid_buf;
        }
        printf("%-*s ", group_w, group_name);

        printf("%*lld ", size_w, (long long)s->st_size);

        char time_str[20];
        struct tm *tm_info = localtime(&s->st_mtime);
        if (tm_info) {
            strftime(time_str, sizeof(time_str), "%b %e %H:%M", tm_info);
        } else {
            strcpy(time_str, "??? ?? ??:??");
        }
        printf("%s ", time_str);
    }

    printf("%s%s%s", col, it->filename, colored ? RESET_COL : "");
    if (detailed) printf("\n");
}

static void process_location(const char *loc, int detailed, int hidden, int multi) {
    DIR *d = opendir(loc);
    if (!d) {
        perror(loc);
        return;
    }

    Item *items = NULL;
    int cnt = 0;
    long long total_space = 0;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (!hidden && e->d_name[0] == '.') continue;
        char full_loc[1024];
        snprintf(full_loc, sizeof(full_loc), "%s/%s", loc, e->d_name);
        struct stat s;
        if (lstat(full_loc, &s) == -1) continue;
        items = realloc(items, (cnt + 1) * sizeof(Item));
        items[cnt].filename = strdup(e->d_name);
        items[cnt].meta = s;
        total_space += s.st_blocks;
        cnt++;
    }
    closedir(d);

    qsort(items, cnt, sizeof(Item), sorter);

    if (multi) {
        printf("%s:\n", loc);
    }

    if (detailed) {
        printf("total %lld\n", total_space / 2);

        int max_nlinks_w = 1;
        int max_user_w = 1;
        int max_group_w = 1;
        int max_size_w = 1;

        for (int i = 0; i < cnt; i++) {
            int l = snprintf(NULL, 0, "%lu", (unsigned long)items[i].meta.st_nlink);
            if (l > max_nlinks_w) max_nlinks_w = l;

            struct passwd *pw = getpwuid(items[i].meta.st_uid);
            if (pw) {
                l = strlen(pw->pw_name);
            } else {
                l = snprintf(NULL, 0, "%d", (int)items[i].meta.st_uid);
            }
            if (l > max_user_w) max_user_w = l;

            struct group *gr = getgrgid(items[i].meta.st_gid);
            if (gr) {
                l = strlen(gr->gr_name);
            } else {
                l = snprintf(NULL, 0, "%d", (int)items[i].meta.st_gid);
            }
            if (l > max_group_w) max_group_w = l;

            l = snprintf(NULL, 0, "%lld", (long long)items[i].meta.st_size);
            if (l > max_size_w) max_size_w = l;
        }

        for (int i = 0; i < cnt; i++) {
            char full_loc[1024];
            snprintf(full_loc, sizeof(full_loc), "%s/%s", loc, items[i].filename);
            display_item(&items[i], detailed, isatty(STDOUT_FILENO),
                        max_nlinks_w, max_user_w, max_group_w, max_size_w,
                        full_loc, items[i].meta.st_uid, items[i].meta.st_gid);
        }
    } else {
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col <= 0) {
            w.ws_col = 80;
        }
        compact_display(items, cnt, isatty(STDOUT_FILENO), w.ws_col);
    }

    if (multi) {
        printf("\n");
    }

    for (int i = 0; i < cnt; i++) {
        free(items[i].filename);
    }
    free(items);
}

static int find_max_name_len(Item *list, int cnt) {
    int max = 0;
    for (int i = 0; i < cnt; i++) {
        int l = strlen(list[i].filename);
        if (l > max) max = l;
    }
    return max;
}

static void compact_display(Item *list, int cnt, int colored, int term_width) {
    if (cnt == 0) return;
    int max_l = find_max_name_len(list, cnt);
    int col_w = max_l + 2;
    if (col_w <= 0) col_w = 1;
    int cols = term_width / col_w;
    if (cols < 1) cols = 1;
    int rows = (cnt + cols - 1) / cols;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = c * rows + r;
            if (idx >= cnt) break;
            display_item(&list[idx], 0, colored, 0, 0, 0, 0, NULL, 0, 0);
            int name_l = strlen(list[idx].filename);
            for (int pad = name_l; pad < max_l + 1; pad++) {
                putchar(' ');
            }
        }
        printf("\n");
    }
}