/* opensvcpcs */

/* PCS utility subroutines */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This code module contains some PCS-related utility subroutines.


*******************************************************************************/


#ifndef	OPENSVCPCS_INCLUDE
#define	OPENSVCPCS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<bits.h>
#include	<filebuf.h>
#include	<pcsns.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */

#define	SUBPCS		struct subpcs
#define	SUBPCS_FL	struct subpcs_flags


/* local structures */

struct subpcs_flags {
	uint		full:1 ;
} ;

struct subpcs {
	cchar		*pr ;
	cchar		**envv ;
	SUBPCS_FL	f ;
	PCSNS		ns ;
	int		w ;
} ;


#if	(! defined(OPENSVCPCS_MASTER)) || (OPENSVCPCS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int subpcs_start(SUBPCS *,cchar *,cchar **,int) ;
extern int subpcs_finish(SUBPCS *) ;
extern int subpcs_af(SUBPCS *,FILEBUF *,cchar *) ;
extern int subpcs_def(SUBPCS *,FILEBUF *) ;
extern int subpcs_all(SUBPCS *,FILEBUF *) ;
extern int subpcs_args(SUBPCS *,FILEBUF *,ARGINFO *,BITS *,cchar *) ;
extern int subpcs_users(SUBPCS *,FILEBUF *,cchar *,int) ;
extern int subpcs_user(SUBPCS *,FILEBUF *,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* OPENSVCPCS_MASTER */

#endif /* OPENSVCPCS_INCLUDE */


