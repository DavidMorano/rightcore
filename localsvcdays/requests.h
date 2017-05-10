/* requests */


/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	REQUESTS_INCLUDE
#define	REQUESTS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vecobj.h>
#include	<localmisc.h>		/* for 'uint' */


#define	REQUESTS		struct requests_head
#define	REQUESTS_ITEM		struct requests_item


struct requests_item {
	int		ro ;
	int		rs ;
} ;

struct requests_head {
	vecobj		ents ;
} ;


typedef struct requests_head	requests ;
typedef struct requests_item	requests_item ;


#if	(! defined(REQUESTS_MASTER)) || (REQUESTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int requests_start(REQUESTS *) ;
extern int requests_finish(REQUESTS *) ;
extern int requests_count(REQUESTS *) ;
extern int requests_del(REQUESTS *,int) ;
extern int requests_add(REQUESTS *,REQUESTS_ITEM *) ;
extern int requests_get(REQUESTS *,int,REQUESTS_ITEM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* REQUESTS_MASTER */

#endif /* REQUESTS_INCLUDE */


