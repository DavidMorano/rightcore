/* termnote */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TERMNOTE_INCLUDE
#define	TERMNOTE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdarg.h>

#include	<tmpx.h>
#include	<ids.h>
#include	<logfile.h>
#include	<localmisc.h>


/* object defines */

#define	TERMNOTE_MAGIC		0x13f3c200
#define	TERMNOTE		struct termnote_head
#define	TERMNOTE_FL		struct termnote_flags
#define	TERMNOTE_BUFSIZE	LINEBUFLEN
#define	TERMNOTE_MAXLINES	20
#define	TERMNOTE_LOGSIZE	400000

/* options */
#define	TERMNOTE_OBELL	(1<<0)		/* ring terminal bell */
#define	TERMNOTE_OBIFF	(1<<1)		/* must have group-execute perm */
#define	TERMNOTE_OALL	(1<<2)		/* all users */


struct termnote_flags {
	uint		tx:1 ;
	uint		lf:1 ;
} ;

struct termnote_head {
	UINT		magic ;
	TERMNOTE_FL	init, open ;
	IDS		id ;
	TMPX		tx ;
	LOGFILE		lf ;
	const char	*pr ;
	const char	*nodename ;
	time_t		ti_check ;
	time_t		ti_tmpx ;
	time_t		ti_logcheck ;
	time_t		ti_write ;
	int		sn ;		/* serial-number */
	char		username[USERNAMELEN+1] ;
	char		logid[LOGIDLEN+1] ;
} ;


typedef struct termnote_head	termnote ;


#if	(! defined(TERMNOTE_MASTER)) || (TERMNOTE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int termnote_open(TERMNOTE *,const char *) ;
extern int termnote_printf(TERMNOTE *,const char **,int,int,const char *,...) ;
extern int termnote_vprintf(TERMNOTE *,const char **,int,int,
		const char *,va_list) ;
extern int termnote_write(TERMNOTE *,const char **,int,int,const char *,int) ;
extern int termnote_check(TERMNOTE *,time_t) ;
extern int termnote_close(TERMNOTE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMNOTE_MASTER */

#endif /* TERMNOTE_INCLUDE */


