/* proguserlist */

/* user-list handling */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 1999-03-01, David A­D­ Morano
        This subroutine was largely borrowed from someplace and
        pseudo-generalized here.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we do some user-list handling, in the background.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<upt.h>
#include	<useraccdb.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#ifndef	LOGCNAME
#define	LOGCNAME	"log"
#endif

#ifndef	USERFSUF
#define	USERFSUF	"users"
#endif

#define	USERLIST	struct userlist


/* external subroutines */

extern int	matstr(cchar **,cchar *,int) ;
extern int	matpstr(cchar **,int,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	isNotPresent(int) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */

typedef int (*workthr)(void *) ;

struct userlist {
	pthread_t	tid ;		/* thread ID */
} ;


/* forward references */

static int	proguserlist_worker(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int proguserlist_begin(PROGINFO *pip)
{
	USERLIST	*ulp ;
	const int	size = sizeof(USERLIST) ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proguserlist_begin: ent\n") ;
#endif

	if ((rs = uc_malloc(size,&ulp)) >= 0) {
	    workthr	w = (workthr) proguserlist_worker ;
	    pthread_t	tid ;
	    memset(ulp,0,size) ;
	    pip->userlist = ulp ;
	    if ((rs = uptcreate(&tid,NULL,w,pip)) >= 0) {
	        ulp->tid = tid ;
	    }
	    if (rs < 0) {
	        uc_free(ulp) ;
	        pip->userlist = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proguserlist_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proguserlist_begin) */


int proguserlist_end(PROGINFO *pip)
{
	USERLIST	*ulp = (USERLIST *) pip->userlist ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proguserlist_end: ent\n") ;
#endif

	if (pip->userlist != NULL) {
	    pthread_t	tid = ulp->tid ;
	    int		trs ;

	    if ((rs1 = uptjoin(tid,&trs)) >= 0) {
	        if (pip->open.logprog) {
	            cchar	*fmt ;
	            if (trs > 0) {
	                fmt = "user-list file created" ;
	                proglog_printf(pip,fmt) ;
	            } else if (trs < 0) {
	                fmt = "inaccessible user-list file (%d)" ;
	                proglog_printf(pip,fmt,trs) ;
	            }
	        } /* end if */
	    } /* end if (thread-join) */
	    if (rs >= 0) rs = rs1 ;

	    rs1 = uc_free(ulp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->userlist = NULL ;
	} /* end if (enabled) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proguserlist_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proguserlist_end) */


/* local subroutines */


static int proguserlist_worker(PROGINFO *pip)
{
	const int	blen = REALNAMELEN ;
	int		rs ;
	int		rs1 ;
	int		f_created = FALSE ;
	cchar		*pr = pip->pr ;
	cchar		*sn = pip->searchname ;
	cchar		*nn = pip->nodename ;
	cchar		*un = pip->username ;
	cchar		*name = pip->name ;
	char		bbuf[REALNAMELEN+1] ;

	if ((rs = sncpy3(bbuf,blen,nn,"!",un)) >= 0) {
	    USERACCDB	udb ;
	    if ((rs = useraccdb_open(&udb,pr,sn)) >= 0) {
	        f_created = (rs > 0) ;
	        {
	            rs = useraccdb_update(&udb,bbuf,name) ; /* all for this */
	        }
	        rs1 = useraccdb_close(&udb) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (useraccdb) */
	} /* end if (sncpy3) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("proguserlist_worker: ret rs=%d f=%u\n",rs,f_created) ;
#endif

	return (rs >= 0) ? f_created : rs ;
}
/* end subroutine (proguserlist_worker) */


