#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <dirent.h>
#include <getopt.h>
#include <errno.h>
#include <sys/time.h>
#include <poll.h>

static int cmp_r(const void *a, const void *b, void *arg) { int dir = *(int *)arg; return dir * (*(const int *)a - *(const int *)b); }

int main(void) {
    const char *fn = "altc_posix_test.tmp";
    int fd = open(fn, O_CREAT | O_TRUNC | O_RDWR, 0644);
    printf("write %zd\n", write(fd, "hello posix\nsecond line\n", 24));
    printf("lseek %ld\n", (long)lseek(fd, 6, SEEK_SET));
    char buf[32] = {0}; printf("read %zd [%.5s]\n", read(fd, buf, 5), buf);
    struct stat st; fstat(fd, &st); printf("size %ld reg %d mode %o\n", (long)st.st_size, S_ISREG(st.st_mode), st.st_mode & 0777);
    close(fd);
    FILE *f = fopen(fn, "r"); char *line = NULL; size_t cap = 0; ssize_t n; while ((n = getline(&line, &cap, f)) > 0) printf("getline %zd: %s", n, line); free(line); fclose(f);
    printf("access %d %d\n", access(fn, R_OK), access("/nonexistent_altc", F_OK) == -1 && errno == ENOENT);
    unlink(fn);
    int *m = mmap(NULL, 4096 * 4, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); m[4095] = 77; printf("mmap %d\n", m[4095]); munmap(m, 4096 * 4);
    int pfd[2]; pipe(pfd); pid_t pid = fork();
    if (pid == 0) { close(pfd[0]); const char *msg = "from child"; write(pfd[1], msg, strlen(msg) + 1); _exit(7); }
    close(pfd[1]); char cb[32]; read(pfd[0], cb, sizeof cb); int status; waitpid(pid, &status, 0);
    printf("%s exit=%d\n", cb, WEXITSTATUS(status));
    char *argv[] = {"prog", "-a", "-b", "val", "--long", "x", "rest", NULL}; int argc = 7, c; optind = 1;
    static struct option lo[] = {{"long", required_argument, 0, 'L'}, {0, 0, 0, 0}};
    while ((c = getopt_long(argc, argv, "ab:", lo, NULL)) != -1) printf("opt %c %s\n", c, optarg ? optarg : "-");
    printf("rest %s\n", argv[optind]);
    char s[] = "k1=v1;k2=v2"; char *save; for (char *t = strtok_r(s, ";", &save); t; t = strtok_r(NULL, ";", &save)) printf("tok %s\n", t);
    int arr[] = {3, 1, 2}; int dir = -1; qsort_r(arr, 3, sizeof(int), cmp_r, &dir); printf("%d %d %d\n", arr[0], arr[1], arr[2]);
    DIR *d = opendir("/"); int found = 0; struct dirent *e; while ((e = readdir(d))) if (!strcmp(e->d_name, "usr")) found = 1; closedir(d); printf("found usr %d\n", found);
    struct timeval tv; printf("gettimeofday %d\n", gettimeofday(&tv, NULL));
    struct pollfd pf = {.fd = 0, .events = 0}; (void)pf;
    char cwd[4096]; printf("getcwd %d\n", getcwd(cwd, sizeof cwd) != NULL);
    printf("%s\n", "ok");
    return 0;
}
