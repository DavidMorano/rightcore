/* biblecur */

/* perform some output processsing activities */


/* revision history:

	= 2009-04-01, David A­D­ Morano
        This subroutine was written as an enhancement for adding back-matter
        (end pages) to the output document.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	BIBLECUR_INCLUDE
#define	BIBLECUR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"biblebook.h"


#define	BIBLECUR	struct biblecur
#define	BIBLECUR_FL	struct biblecur_flags


struct biblecur_flags {
	uint		newbook:1 ;
	uint		newchapter:1 ;
	uint		newverse:1 ;
} ;

struct biblecur {
	BIBLECUR_FL	f ;
	int		book ;
	int		chapter ;
	int		verse ;
	char		bookname[BIBLEBOOK_LEN + 1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int biblecur_start(struct biblecur *) ;
extern int biblecur_finish(struct biblecur *) ;
extern int biblecur_check(struct biblecur *,const char *,int) ;
extern int biblecur_newbook(struct biblecur *,BIBLEBOOK *) ;
extern int biblecur_newchapter(struct biblecur *) ;
extern int biblecur_newverse(struct biblecur *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLECUR_INCLUDE */


