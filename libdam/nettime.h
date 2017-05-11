/* nettime */


/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

#ifndef	NETTIME_INCLUDE
#define	NETTIME_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/time.h>


#define	NETTIME		struct nettime


struct nettime {
	struct timeval	trip ;		/* one-round-trip time */
	struct timeval	off ;		/* offset between net and us */
	int		proto ;
	int		pf ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int nettime(struct nettime *,int,int,const char *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* NETTIME_INCLUDE */


