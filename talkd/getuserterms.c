/* getuserterms */

/* get the name of the controlling terminal for the current session */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-01-10, David A­D­ Morano
        This subroutine was originally written. It was prompted by the failure
        of other terminal message programs from finding the proper controlling
        terminal.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will find and return the name of the controlling
        terminal for the given session ID.

	Synopsis:

	int getuserterms(lp,username)
	VECSTR		*lp ;
	const char	username[] ;

	Arguments:

	- listp		pointer to VECSTR to receive terminals
	- username	session ID to find controlling terminal for

	Returns:

	>=	number of entries returned
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<vecitem.h>
#include	<tmpx.h>
#include	<localmisc.h>


/* local defines */

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev/"
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct termentry {
	const char	*devpath ;
	time_t		atime ;
} ;


/* forward references */

static int	entry_start(struct termentry *,char *,int,time_t) ;
static int	entry_finish(struct termentry *) ;

static int	getatime(const char *,time_t *) ;
static int	revsortfunc(struct termentry **,struct termentry **) ;


/* local variables */


/* exported subroutines */


int getuserterms(lp,username)
VECSTR		*lp ;
const char	username[] ;
{
	TMPX		utmp ;
	TMPX_ENT	ue ;

	VECITEM		el ;

	time_t	ti_access ;

	int	rs ;
	int	rs1 ;
	int	i, tlen ;
	int	n = 0 ;
	int	f ;

	char	termfname[MAXPATHLEN + 1] ;


	if (lp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("getuserterms: entered, sid=%d\n",sid) ;
#endif

	if ((username == NULL) || (username[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("getuserterms: username=%s\n",username) ;
#endif

	rs = vecitem_start(&el,10,VECITEM_PSORTED) ;
	if (rs < 0)
	    goto bad0 ;

/* loop through */

	strcpy(termfname,DEVDNAME) ;

	if ((rs = tmpx_open(&utmp,NULL,O_RDONLY)) >= 0) {
	    TMPX_CUR	cur ;

#if	CF_DEBUGS
	    debugprintf("getuserterms: tmpx_open() rs=%d\n",rs) ;
#endif

	    if ((rs = tmpx_curbegin(&utmp,&cur)) >= 0) {

	        while (rs >= 0) {
	            rs1 = tmpx_fetchuser(&utmp,&cur,&ue,username) ;
		    if (rs1 == SR_NOTFOUND) break ;
		    rs = rs1 ;
		    if (rs < 0) break ;

#if	CF_DEBUGS
	            debugprintf("getuserterms: tmpx_fetchuser() rs=%d\n",rs) ;
#endif

		    f = FALSE ;
	            f = f || (ue.ut_type != TMPX_TUSERPROC) ;
	            f = f || (ue.ut_line[0] == '\0') ;
#ifdef	COMMENT
	            f = f || (strncmp(username,ue.ut_user,32) != 0)
#endif
		    if (f)
	                continue ;

	            tlen = strwcpy((termfname + 5),ue.ut_line,32) - termfname ;

/* check the access time of this terminal and if it is enable for notices */

	            rs = getatime(termfname,&ti_access) ;

	            if (rs >= 0) {
	                struct termentry	te ;

	                rs = entry_start(&te,termfname,tlen,ti_access) ;

	                if (rs >= 0) {
	                    int	size = sizeof(struct termentry) ;
	                    rs = vecitem_add(&el,&te,size) ;
	                }

	                if (rs >= 0) {
	                    n += 1 ;
	                } else
	                    entry_finish(&te) ;

	            } /* end if (we had a better one) */

	        } /* end while (looping through entries) */

	        tmpx_curend(&utmp,&cur) ;
	    } /* end if */

	    tmpx_close(&utmp) ;
	} /* end if (UTMPX open) */

	if ((rs >= 0) && (n > 0)) {
	    struct termentry	*ep ;

	    vecitem_sort(&el,revsortfunc) ;

	    for (i = 0 ; vecitem_get(&el,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            rs = vecstr_add(lp,ep->devpath,-1) ;
	            entry_finish(ep) ;
	        }
		if (rs < 0) break ;
	    } /* end for */

	} /* end if */

	{
	    struct termentry	*ep ;
	    for (i = 0 ; vecitem_get(&el,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            entry_finish(ep) ;
		}
	    } /* end for */
	}

	vecitem_finish(&el) ;

bad0:
ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getuserterms) */


/* local subroutines */


static int entry_start(ep,name,nlen,t)
struct termentry	*ep ;
char			name[] ;
int			nlen ;
time_t			t ;
{
	int	rs ;

	const char	*cp ;


	ep->atime = t ;
	rs = uc_mallocstrw(name,nlen,&cp) ;
	if (rs >= 0) ep->devpath = cp ;

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(ep)
struct termentry	*ep ;
{


	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->devpath != NULL) {
	    uc_free(ep->devpath) ;
	    ep->devpath = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (entry_finish) */


static int getatime(termdev,tp)
const char	termdev[] ;
time_t		*tp ;
{
	struct ustat	sb ;

	int	rs ;


	*tp = 0 ;
	if ((rs = u_stat(termdev,&sb)) >= 0) {

	    *tp = sb.st_atime ;
	    if ((sb.st_mode & S_IWGRP) != S_IWGRP)
	        rs = SR_INVALID ;

	} /* end if */

	return rs ;
}
/* end subroutine (getatime) */


static int revsortfunc(f1pp,f2pp)
struct termentry	**f1pp, **f2pp ;
{


	if ((f1pp == NULL) && (f2pp == NULL))
	    return 0 ;

	if (f1pp == NULL)
	    return 1 ;

	if (f2pp == NULL)
	    return -1 ;

	return ((*f2pp)->atime - (*f1pp)->atime) ;
}
/* end subroutine (revsortfunc) */



