/* AltC <inttypes.h> - format conversion of integer types
   (glibc-compatible, x86-64 LP64).  */

#ifndef _INTTYPES_H
#define _INTTYPES_H 1

#include <features.h>
#include <stdint.h>

#ifndef ____gwchar_t_defined
typedef int __gwchar_t;
# define ____gwchar_t_defined 1
#endif

#define __PRI64_PREFIX "l"
#define __PRIPTR_PREFIX "l"

#define PRId8 "d"
#define PRId16 "d"
#define PRId32 "d"
#define PRId64 __PRI64_PREFIX "d"
#define PRIdLEAST8 "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 __PRI64_PREFIX "d"
#define PRIdFAST8 "d"
#define PRIdFAST16 __PRIPTR_PREFIX "d"
#define PRIdFAST32 __PRIPTR_PREFIX "d"
#define PRIdFAST64 __PRI64_PREFIX "d"
#define PRIdMAX __PRI64_PREFIX "d"
#define PRIdPTR __PRIPTR_PREFIX "d"
#define PRIi8 "i"
#define PRIi16 "i"
#define PRIi32 "i"
#define PRIi64 __PRI64_PREFIX "i"
#define PRIiLEAST8 "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 __PRI64_PREFIX "i"
#define PRIiFAST8 "i"
#define PRIiFAST16 __PRIPTR_PREFIX "i"
#define PRIiFAST32 __PRIPTR_PREFIX "i"
#define PRIiFAST64 __PRI64_PREFIX "i"
#define PRIiMAX __PRI64_PREFIX "i"
#define PRIiPTR __PRIPTR_PREFIX "i"
#define PRIo8 "o"
#define PRIo16 "o"
#define PRIo32 "o"
#define PRIo64 __PRI64_PREFIX "o"
#define PRIoLEAST8 "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 __PRI64_PREFIX "o"
#define PRIoFAST8 "o"
#define PRIoFAST16 __PRIPTR_PREFIX "o"
#define PRIoFAST32 __PRIPTR_PREFIX "o"
#define PRIoFAST64 __PRI64_PREFIX "o"
#define PRIoMAX __PRI64_PREFIX "o"
#define PRIoPTR __PRIPTR_PREFIX "o"
#define PRIu8 "u"
#define PRIu16 "u"
#define PRIu32 "u"
#define PRIu64 __PRI64_PREFIX "u"
#define PRIuLEAST8 "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 __PRI64_PREFIX "u"
#define PRIuFAST8 "u"
#define PRIuFAST16 __PRIPTR_PREFIX "u"
#define PRIuFAST32 __PRIPTR_PREFIX "u"
#define PRIuFAST64 __PRI64_PREFIX "u"
#define PRIuMAX __PRI64_PREFIX "u"
#define PRIuPTR __PRIPTR_PREFIX "u"
#define PRIx8 "x"
#define PRIx16 "x"
#define PRIx32 "x"
#define PRIx64 __PRI64_PREFIX "x"
#define PRIxLEAST8 "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 __PRI64_PREFIX "x"
#define PRIxFAST8 "x"
#define PRIxFAST16 __PRIPTR_PREFIX "x"
#define PRIxFAST32 __PRIPTR_PREFIX "x"
#define PRIxFAST64 __PRI64_PREFIX "x"
#define PRIxMAX __PRI64_PREFIX "x"
#define PRIxPTR __PRIPTR_PREFIX "x"
#define PRIX8 "X"
#define PRIX16 "X"
#define PRIX32 "X"
#define PRIX64 __PRI64_PREFIX "X"
#define PRIXLEAST8 "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 __PRI64_PREFIX "X"
#define PRIXFAST8 "X"
#define PRIXFAST16 __PRIPTR_PREFIX "X"
#define PRIXFAST32 __PRIPTR_PREFIX "X"
#define PRIXFAST64 __PRI64_PREFIX "X"
#define PRIXMAX __PRI64_PREFIX "X"
#define PRIXPTR __PRIPTR_PREFIX "X"

#define SCNd8 "hh" "d"
#define SCNd16 "h" "d"
#define SCNd32 "d"
#define SCNd64 __PRI64_PREFIX "d"
#define SCNdLEAST8 "hh" "d"
#define SCNdLEAST16 "h" "d"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 __PRI64_PREFIX "d"
#define SCNdFAST8 "hh" "d"
#define SCNdFAST16 __PRIPTR_PREFIX "d"
#define SCNdFAST32 __PRIPTR_PREFIX "d"
#define SCNdFAST64 __PRI64_PREFIX "d"
#define SCNdMAX __PRI64_PREFIX "d"
#define SCNdPTR __PRIPTR_PREFIX "d"
#define SCNi8 "hh" "i"
#define SCNi16 "h" "i"
#define SCNi32 "i"
#define SCNi64 __PRI64_PREFIX "i"
#define SCNiLEAST8 "hh" "i"
#define SCNiLEAST16 "h" "i"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 __PRI64_PREFIX "i"
#define SCNiFAST8 "hh" "i"
#define SCNiFAST16 __PRIPTR_PREFIX "i"
#define SCNiFAST32 __PRIPTR_PREFIX "i"
#define SCNiFAST64 __PRI64_PREFIX "i"
#define SCNiMAX __PRI64_PREFIX "i"
#define SCNiPTR __PRIPTR_PREFIX "i"
#define SCNo8 "hh" "o"
#define SCNo16 "h" "o"
#define SCNo32 "o"
#define SCNo64 __PRI64_PREFIX "o"
#define SCNoLEAST8 "hh" "o"
#define SCNoLEAST16 "h" "o"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 __PRI64_PREFIX "o"
#define SCNoFAST8 "hh" "o"
#define SCNoFAST16 __PRIPTR_PREFIX "o"
#define SCNoFAST32 __PRIPTR_PREFIX "o"
#define SCNoFAST64 __PRI64_PREFIX "o"
#define SCNoMAX __PRI64_PREFIX "o"
#define SCNoPTR __PRIPTR_PREFIX "o"
#define SCNu8 "hh" "u"
#define SCNu16 "h" "u"
#define SCNu32 "u"
#define SCNu64 __PRI64_PREFIX "u"
#define SCNuLEAST8 "hh" "u"
#define SCNuLEAST16 "h" "u"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 __PRI64_PREFIX "u"
#define SCNuFAST8 "hh" "u"
#define SCNuFAST16 __PRIPTR_PREFIX "u"
#define SCNuFAST32 __PRIPTR_PREFIX "u"
#define SCNuFAST64 __PRI64_PREFIX "u"
#define SCNuMAX __PRI64_PREFIX "u"
#define SCNuPTR __PRIPTR_PREFIX "u"
#define SCNx8 "hh" "x"
#define SCNx16 "h" "x"
#define SCNx32 "x"
#define SCNx64 __PRI64_PREFIX "x"
#define SCNxLEAST8 "hh" "x"
#define SCNxLEAST16 "h" "x"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 __PRI64_PREFIX "x"
#define SCNxFAST8 "hh" "x"
#define SCNxFAST16 __PRIPTR_PREFIX "x"
#define SCNxFAST32 __PRIPTR_PREFIX "x"
#define SCNxFAST64 __PRI64_PREFIX "x"
#define SCNxMAX __PRI64_PREFIX "x"
#define SCNxPTR __PRIPTR_PREFIX "x"

#if __GLIBC_USE (ISOC23)
#define PRIb8 "b"
#define PRIb16 "b"
#define PRIb32 "b"
#define PRIb64 __PRI64_PREFIX "b"
#define PRIbLEAST8 "b"
#define PRIbLEAST16 "b"
#define PRIbLEAST32 "b"
#define PRIbLEAST64 __PRI64_PREFIX "b"
#define PRIbFAST8 "b"
#define PRIbFAST16 __PRIPTR_PREFIX "b"
#define PRIbFAST32 __PRIPTR_PREFIX "b"
#define PRIbFAST64 __PRI64_PREFIX "b"
#define PRIbMAX __PRI64_PREFIX "b"
#define PRIbPTR __PRIPTR_PREFIX "b"
#define PRIB8 "B"
#define PRIB16 "B"
#define PRIB32 "B"
#define PRIB64 __PRI64_PREFIX "B"
#define PRIBLEAST8 "B"
#define PRIBLEAST16 "B"
#define PRIBLEAST32 "B"
#define PRIBLEAST64 __PRI64_PREFIX "B"
#define PRIBFAST8 "B"
#define PRIBFAST16 __PRIPTR_PREFIX "B"
#define PRIBFAST32 __PRIPTR_PREFIX "B"
#define PRIBFAST64 __PRI64_PREFIX "B"
#define PRIBMAX __PRI64_PREFIX "B"
#define PRIBPTR __PRIPTR_PREFIX "B"
#define SCNb8 "hh" "b"
#define SCNb16 "h" "b"
#define SCNb32 "b"
#define SCNb64 __PRI64_PREFIX "b"
#define SCNbLEAST8 "hh" "b"
#define SCNbLEAST16 "h" "b"
#define SCNbLEAST32 "b"
#define SCNbLEAST64 __PRI64_PREFIX "b"
#define SCNbFAST8 "hh" "b"
#define SCNbFAST16 __PRIPTR_PREFIX "b"
#define SCNbFAST32 __PRIPTR_PREFIX "b"
#define SCNbFAST64 __PRI64_PREFIX "b"
#define SCNbMAX __PRI64_PREFIX "b"
#define SCNbPTR __PRIPTR_PREFIX "b"
#endif

__BEGIN_DECLS

typedef struct
  {
    long int quot;
    long int rem;
  } imaxdiv_t;

extern intmax_t imaxabs (intmax_t __n) __attribute__ ((__const__));
extern imaxdiv_t imaxdiv (intmax_t __numer, intmax_t __denom)
     __attribute__ ((__const__));

extern intmax_t strtoimax (const char *__restrict __nptr,
                           char **__restrict __endptr, int __base);
extern uintmax_t strtoumax (const char *__restrict __nptr,
                            char **__restrict __endptr, int __base);
extern intmax_t wcstoimax (const __gwchar_t *__restrict __nptr,
                           __gwchar_t **__restrict __endptr, int __base);
extern uintmax_t wcstoumax (const __gwchar_t *__restrict __nptr,
                            __gwchar_t **__restrict __endptr, int __base);

/* C23 strto* accept the 0b prefix: glibc exports them as __isoc23_*.  */
#if __GLIBC_USE (C23_STRTOL)
extern intmax_t __isoc23_strtoimax (const char *__restrict __nptr,
                                    char **__restrict __endptr, int __base);
extern uintmax_t __isoc23_strtoumax (const char *__restrict __nptr,
                                     char **__restrict __endptr, int __base);
extern intmax_t __isoc23_wcstoimax (const __gwchar_t *__restrict __nptr,
                                    __gwchar_t **__restrict __endptr,
                                    int __base);
extern uintmax_t __isoc23_wcstoumax (const __gwchar_t *__restrict __nptr,
                                     __gwchar_t **__restrict __endptr,
                                     int __base);
# define strtoimax __isoc23_strtoimax
# define strtoumax __isoc23_strtoumax
# define wcstoimax __isoc23_wcstoimax
# define wcstoumax __isoc23_wcstoumax
#endif

__END_DECLS

#endif /* inttypes.h */
