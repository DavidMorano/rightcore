/* date */

/* date storage object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DATE_INCLUDE
#define	DATE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/timeb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* object defines */

#define	DATE		struct date_head

#define	DATE_ZNAMESIZE	8		/* maximum TZ name length */


struct date_head {
	LONG	time ;			/* UNIX® time */
	short	zoff ;			/* minutes west of GMT */
	short	isdst ;			/* is-daylight-savings time */
	char	zname[DATE_ZNAMESIZE] ;	/* time-zone abbreviation */
} ;


#if	(! defined(DATE_MASTER)) || (DATE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int date_start(DATE *,time_t,int,int,cchar *,int) ;
extern int date_copy(DATE *,DATE *) ;
extern int date_gettime(DATE *,time_t *) ;
extern int date_getzoff(DATE *,int *) ;
extern int date_getisdst(DATE *,int *) ;
extern int date_getzname(DATE *,char *,int) ;
extern int date_finish(DATE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DATE_MASTER */

#endif /* DATE_INCLUDE */


