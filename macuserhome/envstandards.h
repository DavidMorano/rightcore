/* envstandards */

/* these are user-settable compile-time switches */


#ifndef	ENVSTANDARDS_INCLUDE
#define	ENVSTANDARDS_INCLUDE	1


#ifndef	OSNAME_Darwin
#define	OSNAME_Darwin			8
#endif

#ifndef	OSNUM
#define	OSNUM				OSNAME_Darwin
#endif

#ifndef	OSTYPE_SYSV
#define	OSTYPE_SYSV			1
#endif

#ifndef	_REENTRANT
#define	_REENTRANT			1
#endif

#ifndef	_POSIX_C_SOURCE
#define	_POSIX_C_SOURCE			199506L 
#endif

#ifndef	_XOPEN_SOURCE
#define	_XOPEN_SOURCE			500
#endif

#ifndef	_POSIX_PTHREAD_SEMANTICS
#define	_POSIX_PTHREAD_SEMANTICS	1 
#endif

#ifndef	_POSIX_PER_PROCESS_TIMER_SOURCE
#define	_POSIX_PER_PROCESS_TIMER_SOURCE	1
#endif

#ifndef	PTHREAD
#define	PTHREAD				1 /* pthreads are the default */
#endif


#include	<syshas.h>


#endif /* ENVSTANDARDS_INCLUDE */



