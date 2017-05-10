/* sreq */

/* perform various functions on a job */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is responsible for providing means to store a job and the
        retrieve it later by its PID.


*******************************************************************************/


#define	SREQ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<vecitem.h>
#include	<localmisc.h>

#include	"sreq.h"


/* local defines */

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	chmods(const char *,mode_t) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	mkfile(cchar *,cchar **) ;


/* global variables */


/* exported subroutines */


int sreq_start(SREQ *jep,cchar *template,cchar *jobid,int ifd,int ofd)
{
	const time_t	dt = time(NULL) ;
	int		rs = SR_OK ;
	cchar		*cp ;

	memset(jep,0,sizeof(SREQ)) ;
	jep->ifd = ifd ;
	jep->ofd = ofd ;
	jep->efd = -1 ;
	jep->pid = -1 ;
	jep->atime = dt ;
	jep->stime = dt ;

	strwcpy(jep->jobid,jobid,SREQ_JOBIDLEN) ;

	if ((rs = mkfile(template,&cp)) >= 0) {
	    jep->efname = (char *) cp ;
	} /* end if */

	return rs ;
}
/* end subroutine (sreq_start) */


int sreq_finish(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (jep->efname != NULL) {
	    if (jep->efname[0] != '\0') {
		u_unlink(jep->efname) ;
		jep->efname[0] = '\0' ;
	    }
	    rs1 = uc_free(jep->efname) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->efname = NULL ;
	}

	jep->pid = -1 ;
	jep->jobid[0] = '\0' ;
	return rs ;
}
/* end subroutine (sreq_finish) */


/* private subroutines */


static int mkfile(cchar *template,cchar **rpp)
{
	int		rs ;
	int		tl = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	tbuf[0] = '\0' ;
	if ((rs = mktmpfile(tbuf,0666,template)) >= 0) {
	    int	tl = rs ;
	    rs = uc_mallocstrw(tbuf,tl,rpp) ;
	    if (rs < 0) {
	        u_unlink(tbuf) ;
		*rpp = NULL ;
	    } /* end if (error-recovery) */
	} /* end if (mktmpfile) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutines (mkfile) */


