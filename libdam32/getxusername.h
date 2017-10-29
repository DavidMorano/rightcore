/* getxusername */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GETXUSERNAME_INCLUDE
#define	GETXUSERNAME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<vecstr.h>
#include	<localmisc.h>


#ifndef	GETXUSERNAME
#define	GETXUSERNAME		struct getxusername
#endif
#define	GETXUSERNAME_TTL	(2*3600)	/* for cache */


struct getxusername {
	struct passwd	*pwp ;		/* caller supplied */
	char		*pwbuf ;	/* caller supplied */
	char		*ubuf ;		/* caller supplied */
	vecstr		names ;		/* temporary storage */
	uid_t		uid ;		/* caller supplied */
	int		pwlen ;		/* caller supplied */
	int		ulen ;		/* caller supplied */
	int		unl ;		/* result of lookup */
	int		pwl ;		/* result of PASSWD lookup */
	uint		f_self:1 ;	/* it is us */
	uint		f_tried:1 ;	/* tried cache already */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int getxusername(struct getxusername *) ;
extern int getusername(char *,int,uid_t) ;
extern int getpwusername(struct passwd *,char *,int,uid_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* GETXUSERNAME_INCLUDE */


