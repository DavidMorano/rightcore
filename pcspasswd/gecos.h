/* gecos */

/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	GECOS_INCLUDE
#define	GECOS_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#ifndef	UINT
#define	UINT	unsigned int
#endif


/* object defines */

#define	GECOS		struct gecos_head
#define	GECOS_VALUE	struct gecos_value

#define	GECOS_ITEMLEN	256


enum gecosvals {
	gecosval_organization,
	gecosval_realname,
	gecosval_account,
	gecosval_bin,
	gecosval_office,
	gecosval_wphone,
	gecosval_hphone,
	gecosval_printer,
	gecosval_overlast
} ;

struct gecos_value {
	const char	*vp ;
	int		vl ;
} ;

struct gecos_head {
	struct gecos_value	vals[gecosval_overlast] ;
} ;


#if	(! defined(GECOS_MASTER)) || (GECOS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	gecos_start(GECOS *,const char *,int) ;
extern int	gecos_compose(GECOS *,char *,int) ;
extern int	gecos_getval(GECOS *,int,const char **) ;
extern int	gecos_finish(GECOS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* GECOS_MASTER */

#endif /* GECOS_INCLUDE */


