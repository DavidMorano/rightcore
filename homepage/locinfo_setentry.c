/* locinfo_setentry */

/* LOCINFO set-entry */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2011-01-25, David A­D­ Morano
        I had to separate this code due to AST-code conflicts over the system
        socket structure definitions.

*/

/* Copyright © 2004,2005,2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forms part of the LOCINFO set of subroutines (which
        server to facilitate local helper functions for KSH built-ins).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"shio.h"


/* local defines */

#ifndef	LOCINFO
#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	LOCINFO_FL	open ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	vecstr		*vsp = &lip->stores ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(vsp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int		oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(vsp,*epp) ;
	    }
	    if (vp != NULL) { 
		len = strnlen(vp,vl) ;
	        rs = vecstr_store(vsp,vp,len,epp) ;
	    } else {
		*epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(vsp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */


