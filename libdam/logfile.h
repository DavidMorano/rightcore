/* logfile */


/* revision history:

	= 1998-02-22, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LOGFILE_INCLUDE
#define	LOGFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdarg.h>

#include	<localmisc.h>


/* object defines */

#define	LOGFILE			struct logfile_head
#define	LOGFILE_MAGIC		0x13f3c200
#define	LOGFILE_BUFSIZE		(2 * 1024)
#define	LOGFILE_LOGSIZE		80000	/* default log-file length */
#define	LOGFILE_PERCENT		20	/* percent allowed over limit */
#define	LOGFILE_LOGIDLEN	15
#define	LOGFILE_LINELEN		80
#define	LOGFILE_FMTLEN		(LOGFILE_LINELEN - (LOGFILE_LOGIDLEN + 1))
#define	LOGFILE_USERLEN		(LOGFILE_LINELEN - (LOGFILE_LOGIDLEN + 1))

/* control codes */
#define	LOGFILE_CNOP		0
#define	LOGFILE_CSIZE		1


struct logfile_head {
	uint		magic ;
	const char	*fname ;
	char		*buf ;
	time_t		ti_open ;
	time_t		ti_data ;
	time_t		ti_write ;
	mode_t		operm ;
	int		oflags ;
	int		lfd ;
	int		logidlen ;
	int		bufsize ;
	int		len ;		/* length of buffer filled so far */
	int		percent ;
	char		logid[LOGFILE_LOGIDLEN + 1] ;
} ;


typedef struct logfile_head	logfile ;


#if	(! defined(LOGFILE_MASTER)) || (LOGFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int logfile_open(LOGFILE *,const char *,int,mode_t,const char *) ;
extern int logfile_setid(LOGFILE *,const char *) ;
extern int logfile_write(LOGFILE *,const char *,int) ;
extern int logfile_print(LOGFILE *,const char *,int) ;
extern int logfile_printf(LOGFILE *,const char *,...) ;
extern int logfile_vprintf(LOGFILE *,const char *,va_list) ;
extern int logfile_checksize(LOGFILE *,int) ;
extern int logfile_check(LOGFILE *,time_t) ;
extern int logfile_flush(LOGFILE *) ;
extern int logfile_chmod(LOGFILE *,mode_t) ;
extern int logfile_control(LOGFILE *,int,void *) ;
extern int logfile_close(LOGFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOGFILE_MASTER */

#endif /* LOGFILE_INCLUDE */


