/* MAILADDRQUOTE */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILADDRQUOTE_INCLUDE
#define	MAILADDRQUOTE_INCLUDE	1


#include	<envstandards.h>

#include	<bufstr.h>


#define	MAILADDRQUOTE		struct mailaddrquote_head

#define	MAILADDRQUOTE_LEN	100		/* default value */


struct mailaddrquote_flags {
	uint		qaddr:1 ;
} ;

struct mailaddrquote_head {
	BUFSTR		qaddr ;
	struct mailaddrquote_flags	f ;
} ;


#if	(! defined(MAILADDRQUOTE_MASTER)) || (MAILADDRQUOTE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailaddrquote_start(MAILADDRQUOTE *,const char *,int,const char **) ;
extern int mailaddrquote_finish(MAILADDRQUOTE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILADDRQUOTE_MASTER */

#endif /* MAILADDRQUOTE_INCLUDE */


