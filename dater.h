/* dater */

/* dater manipulation object */


/* revision history:

	= 1998-02-03, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DATER_INCLUDE
#define	DATER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/timeb.h>		/* for 'struct timeb' */

#include	<vsystem.h>
#include	<date.h>
#include	<localmisc.h>


/* object defines */

#define	DATER_MAGIC	0x26213711
#define	DATER		struct dater_head
#define	DATER_FL	struct dater_flags
#define	DATER_ZINFO	struct dater_zinfo
#define	DATER_ZNAMESIZE	8		/* maximum TZ name length */

/* dater-type-strings (DTSes) */

#define	DATER_DTSSTD		0		/* email envelope */
#define	DATER_DTSENV		DATER_DTSSTD	/* email envelope */
#define	DATER_DTSHDR		1		/* email header */
#define	DATER_DTSMSG		DATER_DTSHDR	/* message header */
#define	DATER_DTSSTRDIG		2		/* string of digits */
#define	DATER_DTSLOGZ		3		/* 'logz' type */
#define	DATER_DTSGMLOGZ		4		/* 'logz' type for GMT */
#define	DATER_DTSCTIME		DATER_DTSENV	/* same as UNIX 'ctime' */
#define	DATER_DTSEND		5		/* *end* */


#ifdef	COMMENT
struct timeb {
	time_t		time ; 		/* time, seconds since the epoch */
	unsigned short 	millitm ;	/* 1000 msec of additional accuracy */
	short		timezone ;	/* timezone, minutes west of GMT */
	short		dstflag ;	/* DST flag */
} ;
#endif /* COMMENT */

struct dater_zinfo {
	int		zoff ;		/* minutes west of GMT */
	int		isdst ;
	char		zname[DATER_ZNAMESIZE + 1] ;
} ;

struct dater_flags {
	uint		zname:1 ;	/* we have a timezone name string */
	uint		zoff:1 ;	/* we have a timezone offset */
	uint		tzset:1 ;	/* has it been called? */
	uint		cb:1 ;		/* have current time-offset */
	uint		czn:1 ;		/* have current zone-name */
	uint		cyear:1 ;	/* have current year */
} ;

struct dater_head {
	uint		magic ;
	struct timeb	cb ;		/* current */
	struct timeb	b ;
	DATER_FL	f ;
	short		cyear ;		/* current */
	char		cname[DATER_ZNAMESIZE] ;
	char		zname[DATER_ZNAMESIZE] ;
} ;


#ifdef	COMMENT
typedef struct dater_head	dater ;
#endif


#if	(! defined(DATER_MASTER)) || (DATER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dater_start(DATER *,struct timeb *,cchar *,int) ;
extern int dater_startcopy(DATER *,DATER *) ;
extern int dater_setcopy(DATER *,DATER *) ;
extern int dater_setstd(DATER *,cchar *,int) ;
extern int dater_setmsg(DATER *,cchar *,int) ;
extern int dater_setstrdig(DATER *,cchar *,int) ;
extern int dater_setlogz(DATER *,cchar *,int) ;
extern int dater_settouch(DATER *,cchar *,int) ;
extern int dater_settoucht(DATER *,cchar *,int) ;
extern int dater_settmzon(DATER *,struct tm *,int,cchar *,int) ;
extern int dater_settmzo(DATER *,struct tm *,int) ;
extern int dater_settmzn(DATER *,struct tm *,cchar *,int) ;
extern int dater_settimezn(DATER *,time_t,cchar *,int) ;
extern int dater_settimezon(DATER *,time_t,int,cchar *,int) ;
extern int dater_setzinfo(DATER *,DATER_ZINFO *) ;
extern int dater_tzinfo(DATER *,DATER_ZINFO *) ;
extern int dater_mkdatestr(DATER *,int,char *,int) ;
extern int dater_mkstd(DATER *,char *,int) ;
extern int dater_mkenv(DATER *,char *,int) ;
extern int dater_mkmsg(DATER *,char *,int) ;
extern int dater_mkhdr(DATER *,char *,int) ;
extern int dater_mkstrdig(DATER *,char *,int) ;
extern int dater_mklogz(DATER *,char *,int) ;
extern int dater_gettime(DATER *,time_t *) ;
extern int dater_getzoneoff(DATER *,int *) ;
extern int dater_getzonename(DATER *,char *,int) ;
extern int dater_getzinfo(DATER *,DATER_ZINFO *) ;
extern int dater_getdate(DATER *,DATE *) ;
extern int dater_diff(DATER *,DATER *,time_t *) ;
extern int dater_finish(DATER *) ;

#ifdef	COMMENT
extern int dater_nzones(DATER *) ;
extern int dater_zinfo(DATER *,DATER_ZINFO *,int) ;
#endif /* COMMENT */

#ifdef	__cplusplus
}
#endif

#endif /* DATER_MASTER */

#endif /* DATER_INCLUDE */


