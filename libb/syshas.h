/* syshas */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-01, David A­D­ Morano
	Updated for lack of interfaces in MacOS Darwin

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	SYSHAS_INCLUDE
#define	SYSHAS_INCLUDE	1


#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)

/* Solaris: system has shadow password DB */
#define	SYSHAS_SHADOW		1

/* Solaris: system has transitionary 64-bit file operations */
#define	SYSHAS_TRANS64		1

/* Solaris: system has 'statvfs(2)' call and friends */
#define	SYSHAS_STATVFS		1

/* Solaris: system has 'poll(2)' call and friends */
#define	SYSHAS_POLL		1

/* Solaris: system has STREAMS framework */
#define	SYSHAS_STREAMS		1

/* Solaris: has POSIX real-time timers */
#define	SYSHAS_TIMER		1

/* Solaris: system has ACL framework */
#define	SYSHAS_ACL		1

/* Solaris: system has user attributes framework */
#define	SYSHAS_USERATTR		1

/* Solaris: has these two stupid environment manipulation subroutines */
#define	SYSHAS_SETENV		0
#define	SYSHAS_UNSETENV		0

/* Solaris: system information header for 'sysinfo(2)' */
#define	SYSHAS_SYSINFO		1

/* Solaris: system has a 'offset_t' data type (assumed to be 64 bits) */
#define	SYSHAS_OFFSET		1

/* Solaris: system has a 'off32_t' data type */
#define	SYSHAS_OFF32		1

/* Solaris: getcwd(3c) */
#define	SYSHAS_GETCWD		1

/* Solaris: AIO */
#define	SYSHAS_AIO		1

/* Solaris: typedefs */
#define	SYSHAS_USHORT		0
#define	SYSHAS_UINT		0

/* Solaris: tasks */
#define	SYSHAS_TASK		1

/* Solaris: projects */
#define	SYSHAS_PROJECT		1

/* Solaris: UTMPX */
#define	SYSHAS_UTMPX		1

/* Solaris: UTMP-name */
#define	SYSHAS_UTMPNAME		1
#define	SYSHAS_UTMPXNAME	1

/* Solaris: loadavg(3c) */
#define	SYSHAS_LOADAVG		1

/* Solaris: gethrtime(3c) */
#define	SYSHAS_HRTIME		1

/* Solaris: readdir_r(3c) */
#define	SYSHAS_READDIRR		1

/* Solaris: getpwxxx_r(3c) */
#define	SYSHAS_GETPWXXXR	1

/* Solaris: getpwent_r(3c) */
#define	SYSHAS_GETPWENTR	1

/* Solaris: getspxxx_r(3c) */
#define	SYSHAS_GETSPXXXR	1

/* Solaris: getspent_r(3c) */
#define	SYSHAS_GETSPENTR	1

/* Solaris: getgrxxx_r(3c) */
#define	SYSHAS_GETGRXXXR	1

/* Solaris: getgrent_r(3c) */
#define	SYSHAS_GETGRENTR	1

/* Solaris: getprotobyxxx_r(3c) */
#define	SYSHAS_GETPROTOXXXR	1

/* Solaris: getnetbyxxx_r(3c) */
#define	SYSHAS_GETNETXXXR	1

/* Solaris: gethostbyxxx_r(3c) */
#define	SYSHAS_GETHOSTXXXR	1

/* Solaris: getservbyxxx_r(3c) */
#define	SYSHAS_GETSERVXXXR	1

/* Solaris: localtime_r(3c) */
#define	SYSHAS_LOCALTIMER	1

/* Solaris: gmtime_r(3c) */
#define	SYSHAS_GMTIMER		1

/* Solaris: ttyname_r(3c) */
#define	SYSHAS_TTYNAMER		1

/* Solaris: postix_openpt(3c) */
#define	SYSHAS_OPENPT		0

/* Solaris: ptmx(9) */
#define	SYSHAS_PTMX		1

/* Solaris: POSIX shared memory ('shm(3rt)') */
#define	SYSHAS_PSHM		1

/* Solaris: POSIX semaphores ('sem(3rt)') */
#define	SYSHAS_PSEM		1

/* Solaris: POSIX message queues ('mq(3rt)') */
#define	SYSHAS_PMQ		1

/* Solaris: AUDIT - part of Solaris Basic-Security-Module (BSM) */
#define	SYSHAS_AUDIT		1

/* Solaris: get-directory-entries ('getdents(2)') */
#define	SYSHAS_GETDENTS		1

/* Solaris: XTI */
#define	SYSHAS_XTI		1

/* Solaris: robust mutexes */
#define	SYSHAS_MUTEXROBUST	1

/* Solaris®: strnlen(3c) */
#define	SYSHAS_STRNLEN		0


#elif	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)

/* Darwin: system has shadow password DB */
#define	SYSHAS_SHADOW		0

/* Darwin: system has transitionary 64-bit file operations */
#define	SYSHAS_TRANS64		0

/* Darwin: system has 'statvfs(2)' call and friends */
#if	defined(OSNUM) && (OSNUM >= 8)
#define	SYSHAS_STATVFS		1
#else
#define	SYSHAS_STATVFS		0
#endif

/* Darwin: system has 'poll(2)' call and friends */
#if	defined(OSNUM) && (OSNUM >= 8)
#define	SYSHAS_POLL		1
#else
#define	SYSHAS_POLL		0
#endif

/* Darwin: system has ACL framework */
#define	SYSHAS_ACL		0

/* Darwin: system has user attributes framework */
#define	SYSHAS_USERATTR		0

/* Darwin: has these two stupid environment manipulation subroutines */
#define	SYSHAS_SETENV		0
#define	SYSHAS_UNSETENV		0

/* Darwin: system information header for 'sysinfo(2)' */
#define	SYSHAS_SYSINFO		0

/* Darwin: system has a 'offset_t' data type (assumed to be 64 bits) */
#define	SYSHAS_OFFSET		0

/* Darwin: system has a 'off32_t' data type */
#define	SYSHAS_OFF32		0

/* Darwin: getcwd(3c) */
#define	SYSHAS_GETCWD		1

/* Darwin: AIO */
#if	defined(OSNUM) && (OSNUM >= 8)
#define	SYSHAS_AIO		1
#else
#define	SYSHAS_AIO		0
#endif

/* Darwin: typedefs */
#define	SYSHAS_USHORT		1
#define	SYSHAS_UINT		1

/* Darwin: tasks */
#define	SYSHAS_TASK		0

/* Darwin: projects */
#define	SYSHAS_PROJECT		0

/* Darwin: UTMPX */
#if	defined(OSNUM) && (OSNUM >= 8)
#define	SYSHAS_UTMPX		1
#else
#define	SYSHAS_UTMPX		0
#endif

/* Darwin: UTMP-name */
#define	SYSHAS_UTMPNAME		0
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_UTMPXNAME	1
#else
#define	SYSHAS_UTMPXNAME	0
#endif

/* Darwin: loadavg(3c) */
#define	SYSHAS_LOADAVG		1

/* Darwin: gethrtime(3c) */
#define	SYSHAS_HRTIME		0

/* Darwin: readdir_r(3c) */
#define	SYSHAS_READDIRR		1

/* Darwin: getpwxxx_r(3c) */
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_GETPWXXXR	1
#else
#define	SYSHAS_GETPWXXXR	0
#endif

/* Darwin: getpwent_r(3c) */
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_GETPWENTR	0	
#else
#define	SYSHAS_GETPWENTR	0
#endif

/* Darwin: getspxxx_r(3c) */
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_GETSPXXXR	0
#else
#define	SYSHAS_GETSPXXXR	0
#endif

/* Darwin: getspent_r(3c) */
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_GETSPENTR	0	
#else
#define	SYSHAS_GETSPENTR	0
#endif

/* Darwin: getgrxxx_r(3c) */
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_GETGRXXXR	1
#else
#define	SYSHAS_GETGRXXXR	0
#endif

/* Darwin: getgrent_r(3c) */
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_GETGRENTR	0
#else
#define	SYSHAS_GETGRENTR	0
#endif

/* Darwin: getprotobyxxx_r(3c) */
#define	SYSHAS_GETPROTOXXXR	0

/* Darwin: getnetbyxxx_r(3c) */
#define	SYSHAS_GETNETXXXR	0

/* Darwin: gethostbyxxx_r(3c) */
#define	SYSHAS_GETHOSTXXXR	0

/* Darwin: getservbyxxx_r(3c) */
#define	SYSHAS_GETSERVXXXR	0

/* Darwin: localtime_r(3c) */
#define	SYSHAS_LOCALTIMER	1

/* Darwin: gmtime_r(3c) */
#define	SYSHAS_GMTIMER		1

/* Darwin: ttyname_r(3c) */
#define	SYSHAS_TTYNAMER		0

/* Darwin: postix_openpt(3c) */
#define	SYSHAS_OPENPT		1

/* Darwin: ptmx(9) */
#if	defined(OSNUM) && (OSNUM >= 9)
#define	SYSHAS_PTMX		1
#else
#define	SYSHAS_PTMX		0
#endif

/* Darwin: POSIX shared memory ('shm(3rt)') */
#define	SYSHAS_PSHM		1

/* Darwin: POSIX semaphores ('sem(3rt)') */
#define	SYSHAS_PSEM		1

/* Darwin: POSIX message queues ('mq(3rt)') */
#define	SYSHAS_PMQ		0

/* Darwin: AUDIT - part of Solaris® Basic-Security-Module (BSM) */
#define	SYSHAS_AUDIT		0

/* Darwin: get-directory-entries ('getdents(2)') */
#define	SYSHAS_GETDENTS		0

/* Darwin: STREAMS */
#define	SYSHAS_STREAMS		0

/* Darwin: doe *not* have POSIX real-time timers */
#define	SYSHAS_TIMER		0

/* Darwin: XTI */
#define	SYSHAS_XTI		0

/* Solaris: robust mutexes */
/* Darwin: robust mutexes */
#define	SYSHAS_MUTEXROBUST	0

/* Darwin: strnlen(3c) */
#if	defined(OSNUM) && (OSNUM >= 13)
#define	SYSHAS_STRNLEN		1
#else
#define	SYSHAS_STRNLEN		0
#endif


#endif /* OSNAME */

#endif /* SYSHAS_INCLUDE */


