/* Layout check: prints size, alignment and member offsets of the types
   declared by the AltC headers.  Built once against include/c and once
   against the system glibc headers; the outputs must be identical.  */

#define _GNU_SOURCE 1
#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <fenv.h>
#include <float.h>
#include <getopt.h>
#include <glob.h>
#include <grp.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <pwd.h>
#include <sched.h>
#include <semaphore.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
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
#include <threads.h>
#include <time.h>
#include <uchar.h>
#include <unistd.h>
#include <utime.h>
#include <wchar.h>
#include <wctype.h>

#define T(t) \
  printf ("type %-24s size %3zu align %2zu signed %d\n", #t, sizeof (t), \
          _Alignof (t), (int) ((t) -1 < (t) 0))
#define TS(t) \
  printf ("type %-24s size %3zu align %2zu\n", #t, sizeof (t), _Alignof (t))
#define M(t, m) \
  printf ("  %-24s .%-22s off %3zu size %3zu\n", #t, #m, offsetof (t, m), \
          sizeof (((t *) 0)->m))

int
main (void)
{
  /* Integer and scalar typedefs.  */
  T (size_t); T (ssize_t); T (ptrdiff_t); T (wchar_t); T (wint_t);
  T (int8_t); T (int16_t); T (int32_t); T (int64_t);
  T (uint8_t); T (uint16_t); T (uint32_t); T (uint64_t);
  T (int_least8_t); T (int_least16_t); T (int_least32_t); T (int_least64_t);
  T (uint_least8_t); T (uint_least16_t); T (uint_least32_t);
  T (uint_least64_t);
  T (int_fast8_t); T (int_fast16_t); T (int_fast32_t); T (int_fast64_t);
  T (uint_fast8_t); T (uint_fast16_t); T (uint_fast32_t); T (uint_fast64_t);
  T (intptr_t); T (uintptr_t); T (intmax_t); T (uintmax_t);
  T (char16_t); T (char32_t);
  T (off_t); T (off64_t); T (pid_t); T (uid_t); T (gid_t); T (mode_t);
  T (dev_t); T (ino_t); T (ino64_t); T (nlink_t); T (blksize_t);
  T (blkcnt_t); T (fsblkcnt_t); T (fsfilcnt_t); T (id_t); T (key_t);
  T (useconds_t); T (suseconds_t); T (clock_t); T (time_t); T (clockid_t);
  T (socklen_t); T (sa_family_t); T (in_addr_t); T (in_port_t);
  T (sig_atomic_t); T (rlim_t); T (nfds_t); T (speed_t); T (tcflag_t);
  T (cc_t); T (wctype_t); T (fexcept_t); T (pthread_t); T (pthread_key_t);
  T (pthread_once_t); T (pthread_spinlock_t); T (u_char); T (u_long);
  T (quad_t); T (u_int64_t); T (register_t); T (loff_t); T (caddr_t);
  T (float_t); T (double_t); T (thrd_t); T (tss_t); T (__fd_mask);
  T (fd_mask); T (Lmid_t); T (error_t); T (__cpu_mask);
  TS (timer_t); TS (wctrans_t); TS (max_align_t);
  TS (va_list); TS (__gnuc_va_list);

  /* stdio / stdlib / wchar.  */
  TS (FILE); TS (fpos_t); TS (fpos64_t); TS (mbstate_t);
  M (FILE, _flags); M (FILE, _IO_read_ptr); M (FILE, _fileno);
  M (FILE, _short_backupbuf); M (FILE, _old_offset); M (FILE, _cur_column);
  M (FILE, _vtable_offset); M (FILE, _shortbuf); M (FILE, _lock);
  M (FILE, _offset); M (FILE, _codecvt); M (FILE, _wide_data);
  M (FILE, _freeres_list); M (FILE, _freeres_buf); M (FILE, _prevchain);
  M (FILE, _mode); M (FILE, _unused2);
  M (fpos_t, __pos); M (fpos_t, __state);
  M (mbstate_t, __count); M (mbstate_t, __value);
  TS (cookie_io_functions_t);
  TS (div_t); M (div_t, quot); M (div_t, rem);
  TS (ldiv_t); M (ldiv_t, quot); M (ldiv_t, rem);
  TS (lldiv_t); M (lldiv_t, quot); M (lldiv_t, rem);
  TS (imaxdiv_t); M (imaxdiv_t, quot); M (imaxdiv_t, rem);
  TS (struct random_data); M (struct random_data, fptr);
  M (struct random_data, rand_type); M (struct random_data, end_ptr);
  TS (struct drand48_data); M (struct drand48_data, __c);
  M (struct drand48_data, __a);

  /* locale.  */
  TS (struct lconv); M (struct lconv, decimal_point);
  M (struct lconv, int_frac_digits); M (struct lconv, n_sign_posn);
  M (struct lconv, int_n_sign_posn);
  TS (struct __locale_struct); M (struct __locale_struct, __ctype_b);
  M (struct __locale_struct, __names);

  /* time.  */
  TS (struct tm); M (struct tm, tm_sec); M (struct tm, tm_isdst);
  M (struct tm, tm_gmtoff); M (struct tm, tm_zone);
  TS (struct timespec); M (struct timespec, tv_sec);
  M (struct timespec, tv_nsec);
  TS (struct timeval); M (struct timeval, tv_usec);
  TS (struct timezone); M (struct timezone, tz_dsttime);
  TS (struct itimerspec); M (struct itimerspec, it_value);
  TS (struct itimerval); M (struct itimerval, it_value);
  TS (struct tms); M (struct tms, tms_cstime);
  TS (struct utimbuf); M (struct utimbuf, modtime);

  /* setjmp / signal.  */
  TS (jmp_buf); TS (sigjmp_buf); TS (struct __jmp_buf_tag);
  M (struct __jmp_buf_tag, __mask_was_saved);
  M (struct __jmp_buf_tag, __saved_mask);
  TS (sigset_t); TS (siginfo_t); M (siginfo_t, si_signo);
  M (siginfo_t, si_errno); M (siginfo_t, si_code); M (siginfo_t, si_pid);
  M (siginfo_t, si_uid); M (siginfo_t, si_status); M (siginfo_t, si_utime);
  M (siginfo_t, si_stime); M (siginfo_t, si_value); M (siginfo_t, si_addr);
  M (siginfo_t, si_addr_lsb); M (siginfo_t, si_lower);
  M (siginfo_t, si_pkey); M (siginfo_t, si_band); M (siginfo_t, si_fd);
  M (siginfo_t, si_timerid); M (siginfo_t, si_overrun);
  M (siginfo_t, si_call_addr); M (siginfo_t, si_syscall);
  M (siginfo_t, si_arch);
  TS (union sigval); TS (struct sigevent);
  M (struct sigevent, sigev_value); M (struct sigevent, sigev_signo);
  M (struct sigevent, sigev_notify);
  M (struct sigevent, sigev_notify_function);
  M (struct sigevent, sigev_notify_attributes);
  TS (struct sigaction); M (struct sigaction, sa_handler);
  M (struct sigaction, sa_sigaction); M (struct sigaction, sa_mask);
  M (struct sigaction, sa_flags); M (struct sigaction, sa_restorer);
  TS (stack_t); M (stack_t, ss_sp); M (stack_t, ss_flags);
  M (stack_t, ss_size);
  TS (struct sigstack); M (struct sigstack, ss_onstack);

  /* fenv.  */
  TS (fenv_t); M (fenv_t, __control_word); M (fenv_t, __eip);
  M (fenv_t, __cs_selector); M (fenv_t, __data_offset);
  M (fenv_t, __data_selector); M (fenv_t, __mxcsr);
  TS (femode_t); M (femode_t, __mxcsr);

  /* pthread / threads / semaphore / sched.  */
  TS (pthread_attr_t); TS (pthread_mutex_t); TS (pthread_mutexattr_t);
  TS (pthread_cond_t); TS (pthread_condattr_t); TS (pthread_rwlock_t);
  TS (pthread_rwlockattr_t); TS (pthread_barrier_t);
  TS (pthread_barrierattr_t);
  M (pthread_mutex_t, __data.__kind); M (pthread_mutex_t, __data.__list);
  M (pthread_rwlock_t, __data.__flags);
  M (pthread_rwlock_t, __data.__rwelision);
  M (pthread_cond_t, __data.__g_signals);
  TS (__pthread_unwind_buf_t); TS (struct _pthread_cleanup_buffer);
  TS (mtx_t); TS (cnd_t); TS (once_flag); TS (sem_t);
  TS (struct sched_param); TS (cpu_set_t);

  /* files.  */
  TS (struct stat); M (struct stat, st_dev); M (struct stat, st_ino);
  M (struct stat, st_nlink); M (struct stat, st_mode);
  M (struct stat, st_uid); M (struct stat, st_gid); M (struct stat, st_rdev);
  M (struct stat, st_size); M (struct stat, st_blksize);
  M (struct stat, st_blocks); M (struct stat, st_atim);
  M (struct stat, st_mtim); M (struct stat, st_ctim);
  TS (struct stat64); M (struct stat64, st_blocks);
  M (struct stat64, st_ctim);
  TS (struct flock); M (struct flock, l_type); M (struct flock, l_whence);
  M (struct flock, l_start); M (struct flock, l_len); M (struct flock, l_pid);
  TS (struct flock64); M (struct flock64, l_pid);
  TS (struct f_owner_ex); TS (struct file_handle);
  printf ("  file_handle.f_handle off %zu\n", offsetof (struct file_handle, f_handle));
  TS (struct dirent); M (struct dirent, d_ino); M (struct dirent, d_off);
  M (struct dirent, d_reclen); M (struct dirent, d_type);
  M (struct dirent, d_name);
  TS (struct dirent64); M (struct dirent64, d_name);
  TS (glob_t); M (glob_t, gl_pathc); M (glob_t, gl_pathv);
  M (glob_t, gl_offs); M (glob_t, gl_flags); M (glob_t, gl_closedir);
  M (glob_t, gl_stat);
  TS (struct passwd); M (struct passwd, pw_uid); M (struct passwd, pw_gid);
  M (struct passwd, pw_shell);
  TS (struct group); M (struct group, gr_gid); M (struct group, gr_mem);
  TS (struct option); M (struct option, has_arg); M (struct option, flag);
  M (struct option, val);
  TS (Dl_info); M (Dl_info, dli_saddr);

  /* process / system.  */
  TS (struct rlimit); M (struct rlimit, rlim_max);
  TS (struct rusage); M (struct rusage, ru_stime);
  M (struct rusage, ru_maxrss); M (struct rusage, ru_nivcsw);
  TS (struct utsname); M (struct utsname, nodename);
  M (struct utsname, machine); M (struct utsname, domainname);
  TS (fd_set); TS (struct pollfd); M (struct pollfd, events);
  M (struct pollfd, revents);
  TS (struct winsize); M (struct winsize, ws_col);
  M (struct winsize, ws_ypixel);
  TS (struct termio); M (struct termio, c_cc);
  TS (struct termios); M (struct termios, c_iflag);
  M (struct termios, c_lflag); M (struct termios, c_line);
  M (struct termios, c_cc); M (struct termios, c_ispeed);
  M (struct termios, c_ospeed);
  TS (struct iovec); M (struct iovec, iov_len);

  /* sockets.  */
  TS (struct sockaddr); M (struct sockaddr, sa_data);
  TS (struct sockaddr_storage); M (struct sockaddr_storage, ss_family);
  TS (struct msghdr); M (struct msghdr, msg_name);
  M (struct msghdr, msg_namelen); M (struct msghdr, msg_iov);
  M (struct msghdr, msg_iovlen); M (struct msghdr, msg_control);
  M (struct msghdr, msg_controllen); M (struct msghdr, msg_flags);
  TS (struct cmsghdr); M (struct cmsghdr, cmsg_level);
  M (struct cmsghdr, cmsg_type);
  TS (struct mmsghdr); M (struct mmsghdr, msg_len);
  TS (struct ucred); M (struct ucred, gid);
  TS (struct linger); M (struct linger, l_linger);
  TS (struct sockaddr_un); M (struct sockaddr_un, sun_path);
  TS (struct in_addr); TS (struct in6_addr);
  TS (struct sockaddr_in); M (struct sockaddr_in, sin_family);
  M (struct sockaddr_in, sin_port); M (struct sockaddr_in, sin_addr);
  M (struct sockaddr_in, sin_zero);
  TS (struct sockaddr_in6); M (struct sockaddr_in6, sin6_port);
  M (struct sockaddr_in6, sin6_flowinfo); M (struct sockaddr_in6, sin6_addr);
  M (struct sockaddr_in6, sin6_scope_id);
  TS (struct ip_mreq); TS (struct ip_mreqn); M (struct ip_mreqn, imr_ifindex);
  TS (struct ip_mreq_source); TS (struct ipv6_mreq);
  M (struct ipv6_mreq, ipv6mr_interface);
  TS (struct group_req); M (struct group_req, gr_group);
  TS (struct group_source_req); M (struct group_source_req, gsr_source);
  TS (struct in_pktinfo); TS (struct in6_pktinfo); TS (struct ip6_mtuinfo);
  TS (struct hostent); M (struct hostent, h_addrtype);
  M (struct hostent, h_addr_list);
  TS (struct netent); M (struct netent, n_net);
  TS (struct servent); M (struct servent, s_port); M (struct servent, s_proto);
  TS (struct protoent); M (struct protoent, p_proto);
  TS (struct addrinfo); M (struct addrinfo, ai_flags);
  M (struct addrinfo, ai_addrlen); M (struct addrinfo, ai_addr);
  M (struct addrinfo, ai_canonname); M (struct addrinfo, ai_next);
  TS (struct gaicb); M (struct gaicb, __return);

  /* A few expression-valued macros worth checking explicitly.  */
  printf ("MB_CUR_MAX type size %zu\n", sizeof (MB_CUR_MAX));
  printf ("SIG_ERR %p SIG_IGN %p\n", (void *) SIG_ERR, (void *) SIG_IGN);
  printf ("MAP_FAILED %p\n", MAP_FAILED);
  printf ("PTHREAD_CANCELED %p\n", PTHREAD_CANCELED);
  printf ("RTLD_NEXT %p\n", RTLD_NEXT);
  printf ("HUGE_VAL %g HUGE_VALF %g HUGE_VALL %Lg\n", HUGE_VAL,
          (double) HUGE_VALF, HUGE_VALL);
  printf ("INFINITY %g NAN %g\n", (double) INFINITY, (double) NAN);
  printf ("FLT_MAX %a DBL_MAX %a LDBL_MAX %La\n", (double) FLT_MAX, DBL_MAX,
          LDBL_MAX);
  printf ("FLT_MIN %a DBL_MIN %a LDBL_MIN %La\n", (double) FLT_MIN, DBL_MIN,
          LDBL_MIN);
  printf ("FLT_EPSILON %a DBL_EPSILON %a LDBL_EPSILON %La\n",
          (double) FLT_EPSILON, DBL_EPSILON, LDBL_EPSILON);
  printf ("FLT_TRUE_MIN %a DBL_TRUE_MIN %a LDBL_TRUE_MIN %La\n",
          (double) FLT_TRUE_MIN, DBL_TRUE_MIN, LDBL_TRUE_MIN);
  printf ("M_PI %.20g M_PIl %.25Lg\n", M_PI, M_PIl);
  {
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mr = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;
    pthread_cond_t c = PTHREAD_COND_INITIALIZER;
    struct in6_addr a6 = IN6ADDR_LOOPBACK_INIT;
    once_flag of = ONCE_FLAG_INIT;
    unsigned char *p;
    size_t i;
    printf ("init mutex:");
    for (p = (unsigned char *) &m, i = 0; i < sizeof m; i++)
      printf (" %02x", p[i]);
    printf ("\ninit rmutex:");
    for (p = (unsigned char *) &mr, i = 0; i < sizeof mr; i++)
      printf (" %02x", p[i]);
    printf ("\ninit rwlock:");
    for (p = (unsigned char *) &rw, i = 0; i < sizeof rw; i++)
      printf (" %02x", p[i]);
    printf ("\ninit cond:");
    for (p = (unsigned char *) &c, i = 0; i < sizeof c; i++)
      printf (" %02x", p[i]);
    printf ("\nin6 loopback:");
    for (p = (unsigned char *) &a6, i = 0; i < sizeof a6; i++)
      printf (" %02x", p[i]);
    printf ("\nonce %d\n", of.__data);
  }
  {
    fd_set s;
    cpu_set_t cs;
    FD_ZERO (&s);
    FD_SET (3, &s);
    FD_SET (70, &s);
    printf ("fdset 3:%d 4:%d 70:%d\n", FD_ISSET (3, &s), FD_ISSET (4, &s),
            FD_ISSET (70, &s));
    CPU_ZERO (&cs);
    CPU_SET (1, &cs);
    CPU_SET (65, &cs);
    printf ("cpuset 1:%d 2:%d 65:%d count %d\n", CPU_ISSET (1, &cs),
            CPU_ISSET (2, &cs), CPU_ISSET (65, &cs), CPU_COUNT (&cs));
  }
  printf ("bswap %x %x %llx\n", (unsigned) htobe16 (0x1234),
          (unsigned) be32toh (0x12345678u),
          (unsigned long long) htobe64 (0x0102030405060708ull));
  printf ("W %d %d %d %d\n", WIFEXITED (0x0100), WEXITSTATUS (0x0100),
          WIFSIGNALED (0x0009), WTERMSIG (0x0009));
  printf ("S_ISDIR %d %d\n", S_ISDIR (0040755), S_ISREG (0040755));
  printf ("isalpha %d %d\n", isalpha ('a') != 0, isdigit ('a') != 0);
  printf ("classify %d %d %d %d %d\n", fpclassify (1.0), fpclassify (0.0f),
          fpclassify (INFINITY), fpclassify ((long double) NAN),
          fpclassify (DBL_MIN / 2));
  printf ("isnan %d %d %d isinf %d signbit %d %d %d isfinite %d\n",
          isnan (NAN) != 0, isnan (1.0L) != 0, isnan ((float) NAN) != 0,
          isinf (-HUGE_VALL) != 0, signbit (-0.0) != 0,
          signbit (-1.0f) != 0, signbit (1.0L) != 0, isfinite (1.0) != 0);
  printf ("isless %d isunordered %d\n", isless (1.0, 2.0),
          isunordered (NAN, 1.0) != 0);
  return 0;
}
