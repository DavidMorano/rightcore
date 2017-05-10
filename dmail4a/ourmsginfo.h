/* ourmsginfo */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OURMSGINFO_INCLUDE
#define	OURMSGINFO_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>
#include	<vecstr.h>
#include	<dater.h>


#define	OURMSGINFO		struct ourmsginfo_head


enum ourmsginfo_heads {
	ourmsginfohead_messageid,
	ourmsginfohead_errorsto,
	ourmsginfohead_sender,
	ourmsginfohead_from,
	ourmsginfohead_replyto,
	ourmsginfohead_to,
	ourmsginfohead_overlast
} ;

struct ourmsginfo_head {
	char	*efrom ;		/* envelope FROM */
	char	*mid ;			/* message-id */
	char	*h_returnpath ;
	char	*h_subject ;
	DATER	edate ;			/* envelope date */
	vecstr	head[ourmsginfohead_overlast] ;
	time_t	etime ;			/* envelope time */
	time_t	mtime ;			/* message time */
	uint	hif ;			/* header initialization flags */
	uint	offset ;		/* file offset */
	uint	mlen ;			/* message length */
	int	clen ;			/* content length */
	int	f_messageid ;		/* had a message-id */
	int	f_spam ;		/* was a spam */
} ;



#if	(! defined(OURMSGINFO_MASTER)) || (OURMSGINFO_MASTER == 0)

extern int	ourmsginfo_start(OURMSGINFO *,DATER *) ;
extern int	ourmsginfo_setefrom(OURMSGINFO *,const char *,int) ;
extern int	ourmsginfo_setedate(OURMSGINFO *,const char *,int) ;
extern int	ourmsginfo_setmlen(OURMSGINFO *,uint) ;
extern int	ourmsginfo_setclen(OURMSGINFO *,int) ;
extern int	ourmsginfo_setoffset(OURMSGINFO *,uint) ;
extern int	ourmsginfo_setspam(OURMSGINFO *) ;
extern int	ourmsginfo_setsubject(OURMSGINFO *,const char *,int) ;
extern int	ourmsginfo_setreturnpath(OURMSGINFO *,const char *,int) ;
extern int	ourmsginfo_addhead(OURMSGINFO *,int,const char *,int) ;
extern int	ourmsginfo_gethead(OURMSGINFO *,int,int,const char **) ;
extern int	ourmsginfo_finish(OURMSGINFO *) ;

#endif /* OURMSGINFO_MASTER */

#endif /* OURMSGINFO_INCLUDE */


