/* mailenvelope */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILENVELOPE_INCLUDE
#define	MAILENVELOPE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<netdb.h>


#define	MAILENVELOPE		struct mailenvelope

#define	MAILENVELOPE_ADDRESSLEN		((2 * MAXHOSTNAMELEN) + 9)
#define	MAILENVELOPE_DATELEN		255
#define	MAILENVELOPE_REMOTELEN		MAXHOSTNAMELEN


struct mailenvelope {
	int	f_escape ;
	char	address[MAILENVELOPE_ADDRESSLEN + 1] ;
	char	date[MAILENVELOPE_DATELEN + 1] ;
	char	remote[MAILENVELOPE_REMOTELEN + 1] ;
} ;


#if	(! defined(MAILENVELOPE_MASTER)) || (MAILENVELOPE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailenvelope_parse(MAILENVELOPE *,char *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILENVELOPE_MASTER */

#endif /* MAILENVELOPE_INCLUDE */


