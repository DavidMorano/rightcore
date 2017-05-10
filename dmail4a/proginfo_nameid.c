/* proginfo_nameid */

/* utility for KSH built-in commands */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_UNAME	0		/* perform UNAME function */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines set the UNAME and IDS related fields in the PROGINFO
	object.

	Synopsis:

	int proginfo_nameid(pip)
	PROGINFO	*pip ;

	Arguments:

	pip		program information pointer

	Returns:

	>=0	length of PR
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<uinfo.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfprogroot(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int proginfo_nameidbegin(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_UNAME
	if ((rs = uname_start(&pip->un)) >= 0) {
	    pip->usysname = pip->un.sysname ;
	    pip->urelease = pip->un.release ;
	    pip->uversion = pip->un.version ;
	    pip->umachine = pip->un.machine ;
	    rs = ids_load(&pip->id)  ;
	    if (rs < 0) {
	        uname_finish(&pip->un) ;
	    }
	}
#else /* CF_UNAME */
	rs = ids_load(&pip->id)  ;
#endif /* CF_UNAME */

	return rs ;
}
/* end subroutine (proginfo_nameidbegin) */


int proginfo_nameidend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = ids_release(&pip->id) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_UNAME
	rs1 = uname_finish(&pip->un) ;
	if (rs >= 0) rs = rs1 ;
#endif /* CF_UNAME */

	return rs ;
}
/* end subroutine (proginfo_nameidend) */


