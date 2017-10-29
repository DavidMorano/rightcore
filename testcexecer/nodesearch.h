/* nodesearch */

/* search for a node name */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object supports searching for a node name in the cluster
        node list file.


*******************************************************************************/


#ifndef	NODESEARCH_INCLUDE
#define	NODESEARCH_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<limits.h>

#include	<hdbstr.h>
#include	<localmisc.h>

#include	"nodesfile.h"


/* local defines */

#define	NODESEARCH		struct nodesearch_head
#define	NODESEARCH_CUR		struct nodesearch_c


struct nodesearch_flags {
	uint		sw:1 ;
	uint		loaded:1 ;
} ;

struct nodesearch_c {
	NODESFILE_CUR	c1 ;
	HDBSTR_CUR	c2 ;
} ;

struct nodesearch_file {
	cchar	*fname ;
	time_t		mtime ;
	uino_t		ino ;
	dev_t		dev ;
} ;

struct nodesearch_head {
	NODESFILE	a ;		/* first choice */
	HDBSTR		b ;		/* second choice */
	struct nodesearch_flags	f ;
	struct nodesearch_file	fi ;
	time_t		ti_check ;	/* last check time */
	time_t		ti_load ;	/* last load time */
} ;


#if	(! defined(NODESEARCH_MASTER)) || (NODESEARCH_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	nodesearch_open(NODESEARCH *,cchar *,int,int) ;
extern int	nodesearch_search(NODESEARCH *,cchar *,int) ;
extern int	nodesearch_curbegin(NODESEARCH *,NODESEARCH_CUR *) ;
extern int	nodesearch_curend(NODESEARCH *,NODESEARCH_CUR *) ;
extern int	nodesearch_enum(NODESEARCH *,NODESEARCH_CUR *,char *,int) ;
extern int	nodesearch_check(NODESEARCH *,time_t) ;
extern int	nodesearch_close(NODESEARCH *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NODESEARCH_MASTER */

#endif /* NODESEARCH_INCLUDE */


