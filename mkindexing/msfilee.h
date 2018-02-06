/* msfilee */

/* machine status entry */
/* last modified %G% version %I% */


/* revision history:

	= 1999-06-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	MSFILEE_INCLUDE
#define	MSFILEE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>		/* for 'SYS_NMLN' */

#include	<localmisc.h>


/* object defines */

#define	MSFILEE_ALL		struct msfilee_all
#define	MSFILEE_LA		struct msfilee_la
#define	MSFILEE_ATIME		struct msfilee_atime
#define	MSFILEE_UTIME		struct msfilee_utime
#define	MSFILEE_DTIME		struct msfilee_dtime
#define	MSFILEE_STIME		struct msfilee_stime


/* entry field lengths */
#define	MSFILEE_LBOOTTIME	4
#define	MSFILEE_LATIME		4
#define	MSFILEE_LUTIME		4
#define	MSFILEE_LDTIME		4
#define	MSFILEE_LSTIME		4
#define	MSFILEE_LLA		(3 * 4)
#define	MSFILEE_LNPROC		4
#define	MSFILEE_LNUSER		4
#define	MSFILEE_LPMTOTAL	4
#define	MSFILEE_LPMAVAIL	4
#define	MSFILEE_LSPEED		4
#define	MSFILEE_LNCPU		4
#define	MSFILEE_LNODEHASH	4
#define	MSFILEE_LPID		4
#define	MSFILEE_LFLAGS		2
#define	MSFILEE_LNODENAME	SYS_NMLN	/* from <sys/utsname.h> */


/* entry field offsets */
/* do this carefully! */
/* there is no good automatic way to do this in C language (sigh) */
/* the C language does not have all of the advantages of assembly language! */

#define	MSFILEE_OBOOTTIME	0
#define	MSFILEE_OATIME		(MSFILEE_OBOOTTIME + MSFILEE_LBOOTTIME)
#define	MSFILEE_OUTIME		(MSFILEE_OATIME + MSFILEE_LATIME)
#define	MSFILEE_ODTIME		(MSFILEE_OUTIME + MSFILEE_LUTIME)
#define	MSFILEE_OSTIME		(MSFILEE_ODTIME + MSFILEE_LDTIME)
#define	MSFILEE_OLA		(MSFILEE_OSTIME + MSFILEE_LSTIME)
#define	MSFILEE_ONPROC		(MSFILEE_OLA + MSFILEE_LLA)
#define	MSFILEE_ONUSER		(MSFILEE_ONPROC + MSFILEE_LNPROC)
#define	MSFILEE_OPMTOTAL	(MSFILEE_ONUSER + MSFILEE_LNUSER)
#define	MSFILEE_OPMAVAIL	(MSFILEE_OPMTOTAL + MSFILEE_LPMTOTAL)
#define	MSFILEE_OSPEED		(MSFILEE_OPMAVAIL + MSFILEE_LPMAVAIL)
#define	MSFILEE_ONCPU		(MSFILEE_OSPEED + MSFILEE_LSPEED)
#define	MSFILEE_ONODEHASH	(MSFILEE_ONCPU + MSFILEE_LNCPU)
#define	MSFILEE_OPID		(MSFILEE_ONODEHASH + MSFILEE_LNODEHASH)
#define	MSFILEE_OFLAGS		(MSFILEE_OPID + MSFILEE_LPID)
#define	MSFILEE_ONODENAME	(MSFILEE_OFLAGS + MSFILEE_LFLAGS)
#define	MSFILEE_OVERLAST	(MSFILEE_ONODENAME + MSFILEE_LNODENAME)

#define	MSFILEE_SIZE		((MSFILEE_OVERLAST + 3) & (~ 3))


struct msfilee_all {
	uint	btime ;			/* node boot time */
	uint	atime ;			/* access time-stamp */
	uint	utime ;			/* update time-stamp */
	uint	dtime ;			/* disable time-stamp */
	uint	stime ;			/* speed time-stamp */
	uint	la[3] ;
	uint	nproc ;			/* current processes */
	uint	nuser ;			/* logged-in users */
	uint	pmtotal ;		/* physical memory total */
	uint	pmavail ;		/* physical memory available */
	uint	speed ;			/* machine speed (relative) */
	uint	ncpu ;
	uint	nodehash ;		/* hash of nodename */
	uint	pid ;			/* daemon PID */
	ushort	flags ;			/* flags */
	char	nodename[MSFILEE_LNODENAME + 1] ;
} ;

struct msfilee_la {
	uint	la[3] ;
} ;

struct msfilee_atime {
	uint	atime ;
} ;

struct msfilee_utime {
	uint	utime ;
} ;

struct msfilee_dtime {
	uint	dtime ;
} ;

struct msfilee_stime {
	uint	stime ;
} ;


#if	(! defined(MSFILEE_MASTER)) || (MSFILEE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int msfilee_all(struct msfilee_all *,int,char *,int) ;
extern int msfilee_atime(struct msfilee_atime *,int,char *,int) ;
extern int msfilee_utime(struct msfilee_utime *,int,char *,int) ;
extern int msfilee_dtime(struct msfilee_dtime *,int,char *,int) ;
extern int msfilee_stime(struct msfilee_stime *,int,char *,int) ;
extern int msfilee_la(struct msfilee_la *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSFILEE_MASTER */

#endif /* MSFILEE_INCLUDE */


