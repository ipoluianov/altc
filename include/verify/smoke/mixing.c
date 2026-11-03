/* Mixing test: AltC headers together with glibc headers that AltC does not
   ship (resolved through -idirafter /usr/include).  */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/prctl.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/xattr.h>
#include <sys/random.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <spawn.h>
#include <syslog.h>
#include <regex.h>
#include <iconv.h>
#include <langinfo.h>
#include <search.h>
#include <err.h>
#include <error.h>
#include <execinfo.h>
#include <link.h>
#include <malloc.h>
#include <mntent.h>
#include <aio.h>
#include <mqueue.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

int
main (void)
{
  int ep = epoll_create1 (EPOLL_CLOEXEC), efd = eventfd (0, 0);
  struct epoll_event ev;
  struct statvfs vfs;
  struct sysinfo si;
  regex_t re;
  sigset_t mask;
  uint64_t v = 1;
  ev.events = EPOLLIN;
  ev.data.fd = efd;
  epoll_ctl (ep, EPOLL_CTL_ADD, efd, &ev);
  write (efd, &v, sizeof v);
  printf ("epoll %d\n", epoll_wait (ep, &ev, 1, 1000));
  printf ("statvfs %d sysinfo %d\n", statvfs ("/", &vfs), sysinfo (&si));
  printf ("regcomp %d\n", regcomp (&re, "a+b", REG_EXTENDED));
  printf ("regexec %d\n", regexec (&re, "xaab", 0, NULL, 0));
  regfree (&re);
  sigemptyset (&mask);
  sigaddset (&mask, SIGUSR1);
  printf ("signalfd %d\n", signalfd (-1, &mask, 0) >= 0);
  printf ("if_nametoindex lo %d\n", if_nametoindex ("lo") > 0);
  printf ("codeset %s\n", nl_langinfo (CODESET));
  close (ep);
  close (efd);
  return 0;
}
