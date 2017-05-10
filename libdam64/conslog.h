/* conslog */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CONSLOG_INCLUDE
#define	CONSLOG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/log.h>
#include	<sys/strlog.h>
#include	<sys/syslog.h>
#include	<stdarg.h>

#include	<vecstr.h>
#include	<localmisc.h>


/* object defines */

#define	CONSLOG_MAGIC		0x13f3c201
#define	CONSLOG			struct conslog_head
#define	CONSLOG_LINELEN		80


struct conslog_head {
	uint		magic ;
	time_t		ti_open ;
	time_t		ti_write ;
	int		logfac ;
	int		lfd ;
	int		c ;
} ;


typedef struct conslog_head	conslog ;


#if	(! defined(CONSLOG_MASTER)) || (CONSLOG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int conslog_open(CONSLOG *,int) ;
extern int conslog_write(CONSLOG *,int,const char *,int) ;
extern int conslog_printf(CONSLOG *,int,const char *,...) ;
extern int conslog_vprintf(CONSLOG *,int,const char *,va_list) ;
extern int conslog_check(CONSLOG *,time_t) ;
extern int conslog_count(CONSLOG *) ;
extern int conslog_close(CONSLOG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CONSLOG_MASTER */

#endif /* CONSLOG_INCLUDE */


