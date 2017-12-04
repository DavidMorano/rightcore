/* simsg */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	SIMSG_INCLUDE
#define	SIMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


struct simsg_getsysmisc {
	uint	msglen ;
	uint	tag ;
	uchar	msgtype ;		/* message type */
} ;

struct simsg_sysmisc {
	uint	msglen ;
	uint	tag ;
	uint	mtime ;			/* data modification time */
	uint	la1m ;
	uint	la5m ;
	uint	la15m ;
	uint	btime ;
	uint	ncpu ;
	uint	nproc ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;


/* request types */
enum simsgtypes {
	simsgtype_getsysmisc,
	simsgtype_sysmisc,
	simsgtype_overlast
} ;


/* response codes */
enum simsgrcs {
	simsgrc_ok,
	simsgrc_invalid,
	simsgrc_notavail,
	simsgrc_overlast
} ;


#if	(! defined(SIMSG_MASTER)) || (SIMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int simsg_getsysmisc(struct simsg_getsysmisc *,int,char *,int) ;
extern int simsg_sysmisc(struct simsg_sysmisc *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SIMSG_MASTER */

#endif /* SIMSG_INCLUDE */


