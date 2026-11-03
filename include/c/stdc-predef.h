/* AltC <stdc-predef.h> - predefined __STDC_* macros (glibc-compatible).  */

#ifndef _STDC_PREDEF_H
#define _STDC_PREDEF_H 1

/* IEEE 754 floating point.  Complex arithmetic is not advertised because
   AltC does not implement _Complex.  */
#ifndef __STDC_IEC_559__
# define __STDC_IEC_559__ 1
#endif
#ifndef __STDC_IEC_60559_BFP__
# define __STDC_IEC_60559_BFP__ 201404L
#endif

/* wchar_t values are ISO 10646 code points.  */
#ifndef __STDC_ISO_10646__
# define __STDC_ISO_10646__ 201706L
#endif

#endif /* stdc-predef.h */
