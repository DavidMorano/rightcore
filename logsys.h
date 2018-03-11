/* logsys */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LOGSYS_INCLUDE
#define	LOGSYS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/log.h>
#include	<sys/strlog.h>
#include	<sys/syslog.h>
#include	<stdarg.h>

#include	<vecstr.h>
#include	<localmisc.h>


#define	LOGSYS_MAGIC		0x13f3c201
#define	LOGSYS			struct logsys_head
#define	LOGSYS_LOGIDLEN		15
#define	LOGSYS_LINELEN		80
#define	LOGSYS_USERLEN		(LOGSYS_LINELEN - (LOGSYS_LOGIDLEN + 1))


struct logsys_head {
	uint		magic ;
	const char	*logtag ;
	time_t		ti_open ;
	time_t		ti_write ;
	int		logfac ;
	int		opts ;
	int		lfd ;
	int		logidlen ;
	int		n, c ;
	char		logid[LOGSYS_LOGIDLEN + 1] ;
} ;


typedef struct logsys_head	logsys ;


#if	(! defined(LOGSYS_MASTER)) || (LOGSYS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int logsys_open(LOGSYS *,int,const char *,const char *,int) ;
extern int logsys_setid(LOGSYS *,const char *) ;
extern int logsys_write(LOGSYS *,int,const char *,int) ;
extern int logsys_printf(LOGSYS *,int,const char *,...) ;
extern int logsys_vprintf(LOGSYS *,int,const char *,va_list) ;
extern int logsys_check(LOGSYS *,time_t) ;
extern int logsys_flush(LOGSYS *) ;
extern int logsys_close(LOGSYS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOGSYS_MASTER */

#endif /* LOGSYS_INCLUDE */


