/* Smoke test: POSIX headers (files, directories, processes, terminals,
   sockets, misc).  */
#include <arpa/inet.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <getopt.h>
#include <glob.h>
#include <grp.h>
#include <libgen.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pwd.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <utime.h>

static int
cmpname (const struct dirent **a, const struct dirent **b)
{
  return strcmp ((*a)->d_name, (*b)->d_name);
}

int
main (void)
{
  char dir[] = "/tmp/altc_dirXXXXXX", path[PATH_MAX], buf[256];
  struct stat sb;
  DIR *d;
  struct dirent *de, **list;
  int fd, n, i, p[2], status, opt, sv[2];
  pid_t pid;
  char *argv[] = { "prog", "-a", "-b", "val", "--long", "x", "rest", NULL };
  struct option longopts[] = { { "long", required_argument, NULL, 'L' },
                               { NULL, 0, NULL, 0 } };
  struct utsname un;
  struct rlimit rl;
  struct rusage ru;
  struct tms tmsb;
  void *m;
  struct pollfd pfd;
  fd_set rfds;
  struct timeval tv;
  struct termios tio;
  struct winsize ws;
  glob_t g;
  struct in_addr ia;
  struct in6_addr ia6;
  struct addrinfo hints, *res;
  struct iovec iov[2];
  void *h;
  struct passwd *pw;
  struct group *gr;
  struct utimbuf ut;
  struct sockaddr_un sun;

  if (!mkdtemp (dir))
    return 1;
  snprintf (path, sizeof path, "%s/b.txt", dir);
  fd = open (path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  printf ("write %zd\n", write (fd, "hello", 5));
  iov[0].iov_base = (void *) " vec";
  iov[0].iov_len = 4;
  iov[1].iov_base = (void *) "tor";
  iov[1].iov_len = 3;
  printf ("writev %zd\n", writev (fd, iov, 2));
  close (fd);
  snprintf (path, sizeof path, "%s/a.txt", dir);
  fd = open (path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  printf ("pwrite %zd lseek %ld\n", pwrite (fd, "abcdef", 6, 0),
          (long) lseek (fd, 2, SEEK_SET));
  n = (int) read (fd, buf, 3);
  buf[n] = 0;
  printf ("read %s fcntl cloexec %d\n", buf,
          fcntl (fd, F_SETFD, FD_CLOEXEC) == 0
          && (fcntl (fd, F_GETFD) & FD_CLOEXEC) != 0);
  fstat (fd, &sb);
  printf ("fstat size %ld reg %d mode %o\n", (long) sb.st_size,
          S_ISREG (sb.st_mode), (unsigned) (sb.st_mode & 0777));
  close (fd);
  ut.actime = 1000;
  ut.modtime = 2000;
  utime (path, &ut);
  stat (path, &sb);
  printf ("utime mtime %ld atime %ld\n", (long) sb.st_mtime,
          (long) sb.st_atim.tv_sec);
  snprintf (buf, sizeof buf, "%s/sub", dir);
  mkdir (buf, 0755);
  snprintf (buf, sizeof buf, "%s/link", dir);
  symlink ("a.txt", buf);
  lstat (buf, &sb);
  printf ("lstat link %d\n", S_ISLNK (sb.st_mode));
  n = (int) readlink (buf, path, sizeof path - 1);
  path[n > 0 ? n : 0] = 0;
  printf ("readlink %s access %d\n", path, access (buf, R_OK));

  d = opendir (dir);
  n = 0;
  while ((de = readdir (d)) != NULL)
    n++;
  closedir (d);
  printf ("readdir entries %d\n", n);
  n = scandir (dir, &list, NULL, cmpname);
  for (i = 0; i < n; i++)
    {
      printf ("%s(%d) ", list[i]->d_name, list[i]->d_type == DT_DIR);
      free (list[i]);
    }
  free (list);
  printf ("\n");

  snprintf (buf, sizeof buf, "%s/*.txt", dir);
  if (glob (buf, 0, NULL, &g) == 0)
    {
      printf ("glob %zu %s\n", g.gl_pathc, basename (g.gl_pathv[0]));
      globfree (&g);
    }
  printf ("fnmatch %d %d\n", fnmatch ("*.c", "x.c", 0),
          fnmatch ("*.c", "x.h", 0) == FNM_NOMATCH);
  strcpy (buf, "/usr/lib/libx.so");
  printf ("dirname %s\n", dirname (buf));
  printf ("realpath %d\n", realpath (".", path) != NULL);

  if (pipe (p) == 0)
    {
      pid = fork ();
      if (pid == 0)
        {
          close (p[0]);
          write (p[1], "child", 5);
          _exit (3);
        }
      close (p[1]);
      pfd.fd = p[0];
      pfd.events = POLLIN;
      n = poll (&pfd, 1, 5000);
      printf ("poll %d revents %d\n", n, (pfd.revents & POLLIN) != 0);
      FD_ZERO (&rfds);
      FD_SET (p[0], &rfds);
      tv.tv_sec = 5;
      tv.tv_usec = 0;
      printf ("select %d\n", select (p[0] + 1, &rfds, NULL, NULL, &tv));
      n = (int) read (p[0], buf, sizeof buf);
      buf[n] = 0;
      waitpid (pid, &status, 0);
      printf ("child said %s exit %d %d\n", buf, WIFEXITED (status),
              WEXITSTATUS (status));
      close (p[0]);
    }

  opterr = 0;
  n = 7;
  while ((opt = getopt_long (n, argv, "ab:", longopts, NULL)) != -1)
    printf ("opt %c %s\n", opt, optarg ? optarg : "-");
  printf ("optind %d\n", optind);

  n = uname (&un);
  printf ("uname %d sysname %s\n", n, un.sysname);
  n = getrlimit (RLIMIT_NOFILE, &rl);
  printf ("getrlimit %d nofile>0 %d\n", n, rl.rlim_cur > 0);
  printf ("getrusage %d times %d\n", getrusage (RUSAGE_SELF, &ru),
          times (&tmsb) != (clock_t) -1);
  m = mmap (NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
            -1, 0);
  printf ("mmap %d\n", m != MAP_FAILED);
  memset (m, 1, 4096);
  printf ("munmap %d\n", munmap (m, 4096));
  fd = open ("/dev/null", O_RDWR);
  errno = 0;
  n = tcgetattr (fd, &tio);
  printf ("tcgetattr %d ENOTTY %d\n", n, errno == ENOTTY);
  printf ("TIOCGWINSZ %d\n", ioctl (fd, TIOCGWINSZ, &ws));
  close (fd);
  printf ("sysconf pagesize %ld getpagesize %d\n", sysconf (_SC_PAGESIZE),
          getpagesize ());
  printf ("sched_yield %d\n", sched_yield ());

  n = inet_pton (AF_INET, "10.1.2.3", &ia);
  printf ("inet_pton %d %08x\n", n, (unsigned) ntohl (ia.s_addr));
  printf ("inet_ntop %s\n", inet_ntop (AF_INET, &ia, buf, sizeof buf));
  inet_pton (AF_INET6, "::1", &ia6);
  printf ("loopback6 %d htons %x\n", IN6_IS_ADDR_LOOPBACK (&ia6),
          (unsigned) htons (0x1234));
  memset (&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICHOST;
  if (getaddrinfo ("127.0.0.1", NULL, &hints, &res) == 0)
    {
      printf ("getaddrinfo family %d len %u\n", res->ai_family,
              (unsigned) res->ai_addrlen);
      freeaddrinfo (res);
    }
  if (socketpair (AF_UNIX, SOCK_STREAM, 0, sv) == 0)
    {
      int one = 1;
      send (sv[0], "sock", 4, 0);
      n = (int) recv (sv[1], buf, sizeof buf, 0);
      buf[n] = 0;
      printf ("socketpair %s setsockopt %d\n", buf,
              setsockopt (sv[0], SOL_SOCKET, SO_REUSEADDR, &one,
                          sizeof one));
      close (sv[0]);
      close (sv[1]);
    }
  fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  {
    int one = 1;
    printf ("TCP_NODELAY %d\n",
            setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one));
  }
  close (fd);
  memset (&sun, 0, sizeof sun);
  sun.sun_family = AF_UNIX;
  strcpy (sun.sun_path, "/tmp/x");
  printf ("SUN_LEN %d\n", (int) SUN_LEN (&sun));

  h = dlopen ("libm.so.6", RTLD_NOW);
  if (h)
    {
      double (*cosp) (double);
      *(void **) (&cosp) = dlsym (h, "cos");
      printf ("dlsym cos(0) %.1f\n", cosp ? cosp (0.0) : -1.0);
      dlclose (h);
    }
  else
    printf ("dlopen failed %s\n", dlerror ());
  pw = getpwuid (0);
  gr = getgrgid (0);
  printf ("pw %s gr %s\n", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

  snprintf (buf, sizeof buf, "rm -rf %s", dir);
  printf ("cleanup %d\n", system (buf));
  return 0;
}
