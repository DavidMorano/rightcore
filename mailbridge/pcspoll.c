/* pcspoll */

/* load management and interface for the PCSPOLLS object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-04-03, David A­D­ Morano
	This module was originally written.

	- 2008-10-07, David A­D­ Morano
	This module was modified to allow for the main part of it to be a
	loadable module.

*/

/* Copyright © 1998,2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that provides
	access to the PCSPOLLS object (which is dynamically loaded).


*******************************************************************************/


#define	PCSPOLL_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<dlfcn.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<modload.h>
#include	<pcsconf.h>
#include	<localmisc.h>

#include	"pcspoll.h"
#include	"pcspolls.h"


/* local defines */

#define	PCSPOLL_MODBNAME	"pcspolls"
#define	PCSPOLL_OBJNAME		"pcspolls"

#define	LIBCNAME	"lib"
#define	POLLSCNAME	PCSPOLLS_POLLCNAME

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */

static int	pcspoll_objloadbegin(PCSPOLL *,const char *,const char *) ;
static int	pcspoll_objloadend(PCSPOLL *) ;
static int	pcspoll_modloadopen(PCSPOLL *,const char *,const char *) ;
static int	pcspoll_loadcalls(PCSPOLL *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"start",
	"info",
	"cmd",
	"finish",
	NULL
} ;

enum subs {
	sub_start,
	sub_info,
	sub_cmd,
	sub_finish,
	sub_overlast
} ;


/* exported subroutines */


int pcspoll_start(PCSPOLL *op,PCSCONF *pcp,cchar *sn)
{
	int		rs ;
	const char	*pr ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pcspoll_start: pcp={%p} sn=%s\n",pcp,sn) ;
#endif

	memset(op,0,sizeof(PCSPOLL)) ;

	if (pcp == NULL) return SR_FAULT ;
	if (sn == NULL) return SR_FAULT ;

	if (sn[0] == '\0') return SR_INVALID ;

	pr = pcp->pr ;
	if (pr == NULL)
	    return SR_BUGCHECK ;

	{
	    struct ustat	sb ;
	    const char	*lc = LIBCNAME ;
	    const char	*pc = POLLSCNAME ;
	    char	pollsdname[MAXPATHLEN+1] ;
	    if ((rs = mkpath3(pollsdname,pr,lc,pc)) >= 0) {
	        if ((u_stat(pollsdname,&sb) >= 0) && S_ISDIR(sb.st_mode)) {
		    const char	*objname = PCSPOLL_OBJNAME ;
	            if ((rs = pcspoll_objloadbegin(op,pr,objname)) >= 0) {
	                if ((rs = (*op->call.start)(op->obj,pcp,sn)) >= 0) {
			    op->f.loaded = TRUE ;
			}
	                if (rs < 0)
		            pcspoll_objloadend(op) ;
	            } /* end if (obj-mod loading) */
	        } /* end if (stat) */
		if (rs >= 0) op->magic = PCSPOLL_MAGIC ;
	    } /* end if (pollsdname) */
	} /* end block */

#if	CF_DEBUGS
	debugprintf("pcspoll_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcspoll_start) */


/* free up the entire vector string data structure object */
int pcspoll_finish(PCSPOLL *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSPOLL_MAGIC) return SR_NOTOPEN ;

	if (op->f.loaded) {
	    rs1 = (*op->call.finish)(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = pcspoll_objloadend(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (loaded) */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pcspoll_finish) */


int pcspoll_info(PCSPOLL *op,PCSPOLL_INFO *ip)
{
	int		rs = SR_NOSYS ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSPOLL_MAGIC) return SR_NOTOPEN ;

	if (ip != NULL) memset(ip,0,sizeof(PCSPOLL_INFO)) ;

	if (op->f.loaded) {
	    if (op->call.info != NULL) {
	        PCSPOLLS_INFO	ps ;
	        rs = (*op->call.info)(op->obj,&ps) ;
	        if (rs >= 0) {
	            n = rs ;
		    ip->dummy = ps.dummy ;
		}
	    }
	} else
	    rs = SR_OK ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (pcspoll_info) */


int pcspoll_cmd(PCSPOLL *op,int cmd)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSPOLL_MAGIC) return SR_NOTOPEN ;

	if (op->f.loaded) {
	    if (op->call.cmd != NULL) {
	        rs = (*op->call.cmd)(op->obj,cmd) ;
	    }
	} else
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (pcspoll_cmd) */


/* private subroutines */


/* find and load the DB-access object */
static int pcspoll_objloadbegin(PCSPOLL *op,cchar *pr,cchar *objname)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("pcspoll_objloadbegin: pr=%s\n",pr) ;
	debugprintf("pcspoll_objloadbegin: objname=%s\n",objname) ;
#endif

	if ((rs = pcspoll_modloadopen(op,pr,objname)) >= 0) {
	    MODLOAD	*lp = &op->loader ;
	    if ((rs = modload_getmv(lp,0)) >= 0) {
	        int	objsize = rs ;
	        void	*p ;
	        if ((rs = uc_malloc(objsize,&p)) >= 0) {
		    op->obj = p ;
	            rs = pcspoll_loadcalls(op,objname) ;
		    if (rs < 0) {
	    	        uc_free(op->obj) ;
	    	        op->obj = NULL ;
		    }
	        } /* end if (memory-allocation) */
	    } /* end if (getmv) */
	    if (rs < 0)
	       modload_close(lp) ;
	} /* end if (modload-ed) */

	return rs ;
}
/* end subroutine (pcspoll_objloadbegin) */


static int pcspoll_objloadend(PCSPOLL *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->obj != NULL) {
	    rs1 = uc_free(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    op->obj = NULL ;
	}

	rs1 = modload_close(&op->loader) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pcspoll_objloadend) */


static int pcspoll_modloadopen(PCSPOLL *op,cchar *pr,cchar *objname)
{
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		opts = VECSTR_OCOMPACT ;

	if ((rs = vecstr_start(&syms,n,opts)) >= 0) {
	    MODLOAD	*lp = &op->loader ;
 	    int		i ;
	    const char	*modbname ;
	    char	symname[SYMNAMELEN + 1] ;

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i]) ;
		    if (rs >= 0)
			rs = vecstr_add(&syms,symname,rs) ;
		}
		if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        cchar	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            modbname = PCSPOLL_MODBNAME ;
	            objname = PCSPOLL_OBJNAME ;
	            opts = (MODLOAD_OPRS | MODLOAD_OAVAIL) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		}
	    } /* end if (ok) */

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (allocation) */

	return rs ;
}
/* end subroutine (pcspoll_modloadopen) */


static int pcspoll_loadcalls(PCSPOLL *op,cchar objname[])
{
	MODLOAD		*lp = &op->loader ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	char		symname[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i]) ;
	    if (rs < 0) break ;

	    rs1 = modload_getsym(lp,symname,&snp) ;

	    if (rs1 == SR_NOTFOUND) {
		snp = NULL ;
		if (isrequired(i)) break ;
	    } else
		rs = rs1 ;

	    if (rs < 0) break ;

	    if (snp != NULL) {

	        c += 1 ;
		switch (i) {

		case sub_start:
		    op->call.start = 
			(int (*)(void *,PCSCONF *,const char *)) snp ;
		    break ;

		case sub_info:
		    op->call.info = (int (*)(void *,PCSPOLLS_INFO *)) snp ;
		    break ;

		case sub_cmd:
		    op->call.cmd = (int (*)(void *,int)) snp ;
		    break ;

		case sub_finish:
		    op->call.finish = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (pcspoll_loadcalls) */


static int isrequired(int i)
{
	int	f = FALSE ;
	switch (i) {
	case sub_start:
	case sub_finish:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


