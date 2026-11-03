/* AltC <termios.h> - terminal I/O (glibc-compatible, x86-64 Linux).
   Flag values are written in octal like the kernel's definitions.  */

#ifndef _TERMIOS_H
#define _TERMIOS_H 1

#include <features.h>
#include <_altc/types.h>

#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif

__BEGIN_DECLS

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32
struct termios
  {
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t c_line;
    cc_t c_cc[NCCS];
    speed_t c_ispeed;
    speed_t c_ospeed;
#define _HAVE_STRUCT_TERMIOS_C_ISPEED 1
#define _HAVE_STRUCT_TERMIOS_C_OSPEED 1
  };

#define ADDRB 04000000000
#define B0 0000000
#define B1000000 0010010
#define B110 0000003
#define B115200 0010002
#define B1152000 0010011
#define B1200 0000011
#define B134 0000004
#define B150 0000005
#define B1500000 0010012
#define B1800 0000012
#define B19200 0000016
#define B200 0000006
#define B2000000 0010013
#define B230400 0010003
#define B2400 0000013
#define B2500000 0010014
#define B300 0000007
#define B3000000 0010015
#define B3500000 0010016
#define B38400 0000017
#define B4000000 0010017
#define B460800 0010004
#define B4800 0000014
#define B50 0000001
#define B500000 0010005
#define B57600 0010001
#define B576000 0010006
#define B600 0000010
#define B75 0000002
#define B921600 0010007
#define B9600 0000015
#define BRKINT 0000002
#define BS0 0000000
#define BS1 0020000
#define BSDLY 0020000
#define CBAUD 000000010017
#define CBAUDEX 000000010000
#define CIBAUD 002003600000
#define CLOCAL 0004000
#define CMSPAR 010000000000
#define CR0 0000000
#define CR1 0001000
#define CR2 0002000
#define CR3 0003000
#define CRDLY 0003000
#define CREAD 0000200
#define CRTSCTS 020000000000
#define CS5 0000000
#define CS6 0000020
#define CS7 0000040
#define CS8 0000060
#define CSIZE 0000060
#define CSTOPB 0000100
#define ECHO 0000010
#define ECHOCTL 0001000
#define ECHOE 0000020
#define ECHOK 0000040
#define ECHOKE 0004000
#define ECHONL 0000100
#define ECHOPRT 0002000
#define EXTA B19200
#define EXTB B38400
#define EXTPROC 0200000
#define FF0 0000000
#define FF1 0100000
#define FFDLY 0100000
#define FLUSHO 0010000
#define HUPCL 0002000
#define ICANON 0000002
#define ICRNL 0000400
#define IEXTEN 0100000
#define IGNBRK 0000001
#define IGNCR 0000200
#define IGNPAR 0000004
#define IMAXBEL 0020000
#define INLCR 0000100
#define INPCK 0000020
#define ISIG 0000001
#define ISTRIP 0000040
#define IUCLC 0001000
#define IUTF8 0040000
#define IXANY 0004000
#define IXOFF 0010000
#define IXON 0002000
#define NL0 0000000
#define NL1 0000400
#define NLDLY 0000400
#define NOFLSH 0000200
#define OCRNL 0000010
#define OFDEL 0000200
#define OFILL 0000100
#define OLCUC 0000002
#define ONLCR 0000004
#define ONLRET 0000040
#define ONOCR 0000020
#define OPOST 0000001
#define PARENB 0000400
#define PARMRK 0000010
#define PARODD 0001000
#define PENDIN 0040000
#define TAB0 0000000
#define TAB1 0004000
#define TAB2 0010000
#define TAB3 0014000
#define TABDLY 0014000
#define TCIFLUSH 0
#define TCIOFF 2
#define TCIOFLUSH 2
#define TCION 3
#define TCOFLUSH 1
#define TCOOFF 0
#define TCOON 1
#define TCSADRAIN 1
#define TCSAFLUSH 2
#define TCSANOW 0
#define TIOCSER_TEMT 0x01
#define TOSTOP 0000400
#define VDISCARD 13
#define VEOF 4
#define VEOL 11
#define VEOL2 16
#define VERASE 2
#define VINTR 0
#define VKILL 3
#define VLNEXT 15
#define VMIN 6
#define VQUIT 1
#define VREPRINT 12
#define VSTART 8
#define VSTOP 9
#define VSUSP 10
#define VSWTC 7
#define VT0 0000000
#define VT1 0040000
#define VTDLY 0040000
#define VTIME 5
#define VWERASE 14
#define XCASE 0000004
#define XTABS 0014000

#define CCEQ(val, c) ((c) == (val) && (val) != _POSIX_VDISABLE)

extern speed_t cfgetospeed (const struct termios *__termios_p);
extern speed_t cfgetispeed (const struct termios *__termios_p);
extern int cfsetospeed (struct termios *__termios_p, speed_t __speed);
extern int cfsetispeed (struct termios *__termios_p, speed_t __speed);
extern int cfsetspeed (struct termios *__termios_p, speed_t __speed);

extern int tcgetattr (int __fd, struct termios *__termios_p);
extern int tcsetattr (int __fd, int __optional_actions,
                      const struct termios *__termios_p);

extern void cfmakeraw (struct termios *__termios_p);
extern int tcsendbreak (int __fd, int __duration);
extern int tcdrain (int __fd);
extern int tcflush (int __fd, int __queue_selector);
extern int tcflow (int __fd, int __action);
extern __pid_t tcgetsid (int __fd);

#include <sys/ttydefaults.h>

__END_DECLS

#endif /* termios.h */
