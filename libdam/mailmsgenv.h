/* mailmsgenv */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGENV_INCLUDE
#define	MAILMSGENV_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<localmisc.h>


/* object defines */

#define	MAILMSGENV		struct mailmsgenv_head

#define	MAILMSGENV_MAXLINELEN	1092

#ifndef	UINT
#define	UINT	unsigned int
#endif


struct mailmsgenv_flags {
	UINT		start:1 ;	/* was it a starting envelope? */
	UINT		init_time:1 ;	/* did we parse its time yet? */
	UINT		hastime:1 ;	/* did it have a time specification */
	UINT		haszone:1 ;	/* did it have a time zone? */
} ;

struct mailmsgenv_head {
	struct mailmsgenv_flags	f ;
	const char	*address ;
	const char	*origdate ;
	const char	*remote ;
	const char	*tzname ;
	time_t		daytime ;
	int		alen ;
} ;


typedef struct mailmsgenv_head	mailmsgenv ;


#if	(! defined(MAILMSGENV_MASTER)) || (MAILMSGENV_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgenv_start(MAILMSGENV *,const char *,int) ;
extern int mailmsgenv_isstart(MAILMSGENV *) ;
extern int mailmsgenv_getaddress(MAILMSGENV *,const char **) ;
extern int mailmsgenv_getremote(MAILMSGENV *,const char **) ;
extern int mailmsgenv_gettzname(MAILMSGENV *,const char **) ;
extern int mailmsgenv_gettime(MAILMSGENV *,time_t *) ;
extern int mailmsgenv_mkdatestr(MAILMSGENV *,char *,char *,int) ;
extern int mailmsgenv_mkenv(MAILMSGENV *,char *,int) ;
extern int mailmsgenv_finish(MAILMSGENV *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGENV_MASTER */

#endif /* MAILMSGENV_INCLUDE */


