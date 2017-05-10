/*
 * Include file to find out what kind of system we are running.
 */


#ifndef SIGINT
#include <signal.h>
#endif

#ifdef __hpux
#define POSIX		1
#endif

#ifdef SUNOS5
#define POSIX		1			/* use setsid() */
#define JOBCTL 1		/* Berkeley-style job control available */
#define signal(s,f)	sigset(s,f)	/* use Berkeley signals */
#endif

#if	defined(SIGSTOP) && (! defined(SUNOS5))
#define JOBCTL 1		/* Berkeley-style job control available */
#define	BSD4_2 1		/* to compile for 4.2BSD/4.3BSD */
#endif


#ifndef SIGRET
#define SIGRET void
#endif



