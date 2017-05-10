/* msumsg */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	MSUMSG_INCLUDE
#define	MSUMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


struct msumsg_status {
	uint	msglen ;
	uint	tag ;
	uint	pid ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

struct msumsg_getstatus {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct msumsg_getsysmisc {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct msumsg_sysmisc {
	uint	msglen ;
	uint	tag ;
	uint	pid ;
	uint	utime ;			/* data update time */
	uint	btime ;
	uint	ncpu ;
	uint	nproc ;
	uint	la[3] ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

struct msumsg_exit {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct msumsg_mark {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct msumsg_report {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;


/* request types */
enum msumsgtypes {
	msumsgtype_status,		/* 0 */
	msumsgtype_getstatus,		/* 1 */
	msumsgtype_getsysmisc,		/* 2 */
	msumsgtype_exit,		/* 3 */
	msumsgtype_mark,		/* 4 */
	msumsgtype_report,		/* 5 */
	msumsgtype_sysmisc,		/* 6 */
	msumsgtype_overlast		/* 7 */
} ;


/* response codes */
enum msumsgrcs {
	msumsgrc_ok,
	msumsgrc_invalid,
	msumsgrc_notavail,
	msumsgrc_overlast
} ;


#if	(! defined(MSUMSG_MASTER)) || (MSUMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int msumsg_getstatus(struct msumsg_getstatus *,int,char *,int) ;
extern int msumsg_status(struct msumsg_status *,int,char *,int) ;
extern int msumsg_getsysmisc(struct msumsg_getsysmisc *,int,char *,int) ;
extern int msumsg_sysmisc(struct msumsg_sysmisc *,int,char *,int) ;
extern int msumsg_mark(struct msumsg_mark *,int,char *,int) ;
extern int msumsg_report(struct msumsg_report *,int,char *,int) ;
extern int msumsg_exit(struct msumsg_exit *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MSUMSG_MASTER */

#endif /* MSUMSG_INCLUDE */


