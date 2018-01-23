/* artlist */


/* Copyright © 1995,1998,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	ARTLIST_INCLUDE
#define	ARTLIST_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/timeb.h>

#include	<vechand.h>
#include	<dater.h>
#include	<localmisc.h>		/* extra types */


#define	ARTLIST			struct artlist_head
#define	ARTLIST_ENT		struct artlist_e
#define	ARTLIST_MAGIC		0x83465875
#define	ARTLIST_CURMAGIC	0x83465876


struct artlist_head {
	uint		magic ;
	DATER		mdate ;
	struct timeb	now ;
	vechand		arts ;
} ;

struct artlist_e {
	uint		magic ;
	const char	*ngdir ;	/* newsgroup directory */
	const char	*name ;		/* filename (overlaps w/ 'ngdir') */
	const char	*subject ;	/* header SUBJECT */
	const char	*replyto ;	/* header REPLYTO */
	const char	*from ;		/* header FEOM */
	const char	*newsgroups ;	/* header NEWSGROUPS */
	const char	*messageid ;	/* header MESSAGEID */
	const char	*articleid ;	/* header ARTICLEID */
	time_t		mtime ;		/* modification time (always has one) */
	time_t		atime ;		/* arrival time (envelope if any) */
	time_t		ptime ;		/* posting time (if any) */
	time_t		ctime ;		/* compose time (if any) */
	int		lines ;		/* lines in article body (if known) */
	int		clen ;		/* article body (if known) */
} ;


#if	(! defined(ARTLIST_MASTER)) || (ARTLIST_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	artlist_start(ARTLIST *,struct timeb *,const char *) ;
extern int	artlist_finish(ARTLIST *) ;
extern int	artlist_add(ARTLIST *,const char *,const char *) ;
extern int	artlist_sort(ARTLIST *,int,int) ;
extern int	artlist_get(ARTLIST *,int,cchar **,cchar **,time_t *) ;
extern int	artlist_getentry(ARTLIST *,int,ARTLIST_ENT **) ;

#ifdef	__cplusplus
}
#endif

#endif /* ARTLIST_MASTER */

#endif /* ARTLIST_INCLUDE */


