/* mailmsgfile */

/* create and cache message content files */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	MAILMSGFILE_INCLUDE
#define	MAILMSGFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	MAILMSGFILE_MAGIC	0x24182132
#define	MAILMSGFILE		struct mailmsgfile_head
#define	MAILMSGFILE_FL		struct mailmsgfile_flags
#define	MAILMSGFILE_MI		struct mailmsgfile_mi

#define	MAILMSGFILE_STRLEN	100
#define	MAILMSGFILE_TZNAMELEN	10

/* types */

#define	MAILMSGFILE_TTEMP	0
#define	MAILMSGFILE_TPERM	1


struct mailmsgfile_mi {
	const char	*a ;		/* memory allocation */
	const char	*mid ;
	const char	*mfname ;
	uint		nsize ;
	uint		vsize ;
	uint		nlines ;
	uint		vlines ;
} ;

struct mailmsgfile_flags {
	uint		mailnew:1 ;	/* new mail arrived */
	uint		files:1 ;	/* container initialized */
	uint		checkout:1 ;	/* thread is running */
} ;

struct mailmsgfile_head {
	uint		magic ;
	HDB		files ;
	MAILMSGFILE_FL	f ;
	const char	*tmpdname ;
	pthread_t	tid ;
	int		pagesize ;
	int		cols ;
	int		ind ;
	volatile int	f_checkdone ;	/* thread has completed */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgfile_start(MAILMSGFILE *,const char *,int,int) ;
extern int mailmsgfile_new(MAILMSGFILE *,int,const char *,int,offset_t,int) ;
extern int mailmsgfile_get(MAILMSGFILE *,const char *,const char **) ;
extern int mailmsgfile_msginfo(MAILMSGFILE *,MAILMSGFILE_MI **,const char *) ;
extern int mailmsgfile_finish(MAILMSGFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGFILE_INCLUDE */


