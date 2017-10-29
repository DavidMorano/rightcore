/* maininfo */

/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_PROGINFO	0		/* use 'maininfo_xxx()' */


/* revision history:

	= 1998-07-28, David A­D­ Morano

	This subroutine was written for use as a front-end for Korn Shell (KSH)
	commands that are compiled as stand-alone programs.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end to make the various SHELL (KSH) built-in commands
	into stand-alone programs.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<upt.h>
#include	<localmisc.h>

#include	"maininfo.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char		*strnrchr(const char *,int,int) ;


/* external variables */


/* local structures */

typedef int (*thrsub)(void *) ;


/* forward references */

static int maininfo_utiler(MAININFO *) ;


/* local variables */


/* exported subroutines */


int maininfo_start(MAININFO *mip,int argc,const char **argv)
{
	int		rs ;
	const char	*argz = NULL ;

	memset(mip,0,sizeof(MAININFO)) ;

	if ((argc > 0) && (argv != NULL)) argz = argv[0] ;

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	if (argz == NULL) argz = getexecname() ;
#endif

	if ((rs = vecstr_start(&mip->stores,2,0)) >= 0) {
	    int		cl ;
	    const char	*cp ;

	    if ((cl = sfbasename(argz,-1,&cp)) > 0) {
		const char	**vpp = &mip->progname ;
		if (cp[0] == '-') {
		    mip->f.progdash = TRUE ;
		    cp += 1 ;
		    cl -= 1 ;
		}
		if (cl > 0) {
		    const char	*tp ;
		    if ((tp = strnrchr(cp,cl,'.')) != NULL) {
			cl = (tp-cp) ;
		    }
		}
		if (cl > 0) {
		    rs = maininfo_setentry(mip,vpp,cp,cl) ;
		} else
		    rs = SR_DOM ;
	    } else
		rs = SR_DOM ;

	    if (rs < 0) 
		vecstr_finish(&mip->stores) ;
	} /* end if */

	return rs ;
}
/* end subroutine (maininfo_start) */


int maininfo_finish(MAININFO *mip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mip == NULL) return SR_FAULT ;

	rs1 = vecstr_finish(&mip->stores) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (maininfo_finish) */


int maininfo_setentry(MAININFO *mip,cchar **epp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		oi = -1 ;
	int		len = 0 ;

	if (mip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_INVALID ;

	if (*epp != NULL) {
	    oi = vecstr_findaddr(&mip->stores,*epp) ;
	}
	if (vp != NULL) {
	    len = strnlen(vp,vl) ;
	    rs = vecstr_store(&mip->stores,vp,len,epp) ;
	} else {
	    *epp = NULL ;
	}
	if ((rs >= 0) && (oi >= 0)) {
	    vecstr_del(&mip->stores,oi) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (maininfo_setentry) */


int maininfo_utilbegin(MAININFO *op,int f_run)
{
	pthread_t	tid ;
	thrsub		w = (thrsub) maininfo_utiler ;
	int		rs = SR_OK ;

	if (f_run) {
	    if ((rs = uptcreate(&tid,NULL,w,op)) >= 0) {
	        op->tid = tid ;
	        op->f.utilout = TRUE ;
	    }
	} /* end if (run) */

	return rs ;
}
/* end subroutine (maininfo_utilbegin) */


int maininfo_utilend(MAININFO *op)
{
	int		rs = SR_OK ;

	if (op->f.utilout) {
	    int	trs = SR_OK ;
	    op->f.utilout = FALSE ;
	    if ((rs = uptjoin(op->tid,&trs)) >= 0) {
	        rs = trs ;
	    }
	}

	return rs ;
}
/* end subroutine (maininfo_utilend) */


int maininfo_srchname(MAININFO *mip,const char **rpp)
{
	int		rs = SR_OK ;
	const char	*srch = mip->progname ;
	if (rpp == NULL) return SR_FAULT ;
	*rpp = srch ;
	if (hasuc(srch,-1)) {
	    const int	slen = MAXNAMELEN ;
	    char	sbuf[MAXNAMELEN+1] ;
	    if ((rs = sncpylc(sbuf,slen,srch)) >= 0) {
		rs = maininfo_setentry(mip,rpp,sbuf,rs) ;
	    }
	} else {
	    rs = strlen(srch) ;
	}
	return rs ;
}
/* end subroutine (maininfo_srchname) */


/* private subroutines */


static int maininfo_utiler(MAININFO *mip)
{
	const int	of = (O_WRONLY|O_APPEND) ;
	int		rs = SR_OK ;
	int		rs1 ;
	const char	*fn = "here.txt" ;

	if ((rs1 = u_open(fn,of,0664)) >= 0) {
	    const int	wlen = LINEBUFLEN ;
	    int		fd = rs1 ;
	    const char	*fmt = "hello world!\n" ;
	    char	wbuf[LINEBUFLEN+1] ;
	    if ((rs = bufprintf(wbuf,wlen,fmt)) >= 0) {
		rs = u_write(fd,wbuf,rs) ;
	    }
	    u_close(fd) ;
	} /* end if (open) */

	mip->f_done = TRUE ;
	return rs ;
}
/* end subroutine (maininfo_utiler) */


