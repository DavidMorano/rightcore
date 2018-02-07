/* tmpx_getuserterms */

/* get all of the terminals where the given user is logged in */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
        This subroutine was originally written. It was prompted by the failure
        of other terminal message programs from finding the proper controlling
        terminal.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will find and return the names of all of the controlling
        terminals for the specified username, if there are any.

	Synopsis:

	int tmpx_getuserterms(op,lp,username)
	TMPX		*op ;
	VECSTR		*lp ;
	const char	username[] ;

	Arguments:

	op		pointer to TMPX object
	lp		pointer to VECSTR to receive terminals
	username	user to find controlling terminals for

	Returns:

	<0		error
	>=0		number of entries returned


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<vecitem.h>
#include	<tmpx.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#define	DEVDNAME	"/dev/"

#define	TERMENTRY	struct xtermentry


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

TERMENTRY {
	const char	*devpath ;
	time_t		atime ;
} ;


/* forward references */

static int	entry_start(TERMENTRY *,char *,int,time_t) ;
static int	entry_finish(TERMENTRY *) ;

static int	mktermfname(char *,int,const char *,int) ;
static int	getatime(const char *,time_t *) ;
static int	revsortfunc(TERMENTRY **,TERMENTRY **) ;


/* local variables */


/* exported subroutines */


int tmpx_getuserterms(TMPX *op,VECSTR *lp,cchar *username)
{
	VECITEM		el ;
	int		rs ;
	int		rs1 ;
	int		ddnl ;
	int		n = 0 ;
	const char	*devdname = DEVDNAME ;
	char		termfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (lp == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	if (username[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("tmpx_getuserterms: u=%s\n",username) ;
#endif

	ddnl = mkpath1(termfname,devdname) ;

	if ((rs = vecitem_start(&el,10,VECITEM_PSORTED)) >= 0) {
	    TERMENTRY		*ep ;
	    TMPX_ENT		ue ;
	    TMPX_CUR		cur ;
	    const int		llen = TMPX_LLINE ;
	    int			i ;
	    int			f ;

	    if ((rs = tmpx_curbegin(op,&cur)) >= 0) {
	        time_t	ti_access ;
		int	tlen ;

	        while (rs >= 0) {
	            rs1 = tmpx_fetchuser(op,&cur,&ue,username) ;
	            if (rs1 == SR_NOTFOUND) break ;
	            rs = rs1 ;
	            if (rs < 0) break ;

	            f = FALSE ;
	            f = f || (ue.ut_type != TMPX_TUSERPROC) ;
	            f = f || (ue.ut_line[0] == '\0') ;
#ifdef	COMMENT
	            f = f || (strncmp(username,ue.ut_user,llen) != 0) ;
#endif
	            if (f)
	                continue ;

	            tlen = mktermfname(termfname,ddnl,ue.ut_line,llen) ;

/* check the access time of this terminal and permissions */

	            if ((rs1 = getatime(termfname,&ti_access)) >= 0) {
	                TERMENTRY	te ;
			const int	ti = ti_access ;

	                if ((rs = entry_start(&te,termfname,tlen,ti)) >= 0) {
	                    const int	esize = sizeof(TERMENTRY) ;
	                    n += 1 ;
	                    rs = vecitem_add(&el,&te,esize) ;
	                    if (rs < 0) 
				entry_finish(&te) ;
			}

	            } /* end if (we had a better one) */

	        } /* end while (looping through entries) */

	        tmpx_curend(op,&cur) ;
	    } /* end if (cursor) */

	    if ((rs >= 0) && (n > 0)) {
	        if ((rs = vecitem_sort(&el,revsortfunc)) >= 0) {
	            for (i = 0 ; vecitem_get(&el,i,&ep) >= 0 ; i += 1) {
	                if (ep != NULL) {
	                    rs = vecstr_add(lp,ep->devpath,-1) ;
		        }
	                if (rs < 0) break ;
	            } /* end for */
		} /* end if (vecitem_sort) */
	    } /* end if (positive) */

/* free up */

	    for (i = 0 ; vecitem_get(&el,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            entry_finish(ep) ;
		}
	    } /* end for */

	    vecitem_finish(&el) ;
	} /* end if (vecitem) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (tmpx_getuserterms) */


/* local subroutines */


static int entry_start(TERMENTRY *ep,char *fp,int fl,time_t t)
{
	int		rs ;
	const char	*cp ;

	ep->atime = t ;
	rs = uc_mallocstrw(fp,fl,&cp) ;
	if (rs >= 0) ep->devpath = cp ;

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(TERMENTRY *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->devpath != NULL) {
	    rs1 = uc_free(ep->devpath) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->devpath = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


static int mktermfname(char *rbuf,int ddnl,cchar *sp,int sl)
{
	const int	rlen = MAXPATHLEN ;
	int		rs ;
	int		len = ddnl ;

	rs = storebuf_strw(rbuf,rlen,ddnl,sp,sl) ;
	len += rs ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mktermfname) */


static int getatime(cchar *termdev,time_t *rp)
{
	struct ustat	sb ;
	int		rs ;
	int		f = TRUE ;

	*rp = 0 ;
	if ((rs = u_stat(termdev,&sb)) >= 0) {
	    *rp = sb.st_atime ;
	    if ((sb.st_mode & S_IWGRP) != S_IWGRP) {
	        f = FALSE ;
	    }
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (getatime) */


static int revsortfunc(TERMENTRY **f1pp,TERMENTRY **f2pp)
{
	int		rc = 0 ;
	if ((f1pp != NULL) || (f2pp != NULL)) {
	    if (f1pp != NULL) {
	        if (f2pp != NULL) {
	            rc = ((*f2pp)->atime - (*f1pp)->atime) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	} 
	return rc ;
}
/* end subroutine (revsortfunc) */


