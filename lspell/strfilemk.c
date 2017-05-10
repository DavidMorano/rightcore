/* strfilemk */

/* STRFILEMK management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 1998-12-01, David A­D­ Morano

	This module was originally written for hardware CAD support.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that
	provides access to the STRFILEMK object (which is dynamically
	loaded).


*******************************************************************************/


#define	STRFILEMK_MASTER	1


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
#include	<localmisc.h>

#include	"strfilemk.h"
#include	"strfilemks.h"


/* local defines */

#define	STRFILEMK_MODBNAME	"strfilemks"
#define	STRFILEMK_OBJNAME	"strfilemks"
#define	STRFILEMK_PRLOCAL	"LOCAL"

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
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mksofname(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	strfilemk_objloadbegin(STRFILEMK *,const char *,const char *) ;
static int	strfilemk_objloadend(STRFILEMK *) ;
static int	strfilemk_loadcalls(STRFILEMK *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"addfile",
	"abort",
	"chgrp",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_addfile,
	sub_abort,
	sub_chgrp,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int strfilemk_open(op,pr,dbname,of,om,n)
STRFILEMK	*op ;
const char	pr[] ;
const char	dbname[] ;
int		of ;
mode_t		om ;
int		n ;
{
	int	rs ;

	const char	*objname = STRFILEMK_OBJNAME ;


	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("strfilemk_open: ent dbname=%s\n",dbname) ;
#endif

	memset(op,0,sizeof(STRFILEMK)) ;

	if ((rs = strfilemk_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,dbname,of,om,n)) >= 0) {
		op->magic = STRFILEMK_MAGIC ;
	    }
	    if (rs < 0)
		strfilemk_objloadend(op) ;
	} /* end if (objloadbegin) */

#if	CF_DEBUGS
	debugprintf("strfilemk_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strfilemk_open) */


/* free up the entire vector string data structure object */
int strfilemk_close(op)
STRFILEMK	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != STRFILEMK_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("strfilemk_close: strfilemks_close() rs=%d\n",rs) ;
#endif

	rs1 = strfilemk_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("strfilemk_close: strfilemk_objloadend() rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (strfilemk_close) */


int strfilemk_addfile(op,sp,sl)
STRFILEMK	*op ;
const char	sp[] ;
int		sl ;
{
	int	rs ;


	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != STRFILEMK_MAGIC) return SR_NOTOPEN ;

	rs = (*op->call.addfile)(op->obj,sp,sl) ;

	return rs ;
}
/* end subroutine (strfilemk_addfile) */


int strfilemk_abort(op)
STRFILEMK	*op ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != STRFILEMK_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.abort != NULL)
	    rs = (*op->call.abort)(op->obj) ;

	return rs ;
}
/* end subroutine (strfilemk_abort) */


int strfilemk_chgrp(op,gid)
STRFILEMK	*op ;
gid_t		gid ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != STRFILEMK_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.chgrp != NULL)
	    rs = (*op->call.chgrp)(op->obj,gid) ;

	return rs ;
}
/* end subroutine (strfilemk_chgrp) */


/* private subroutines */


/* find and load the DB-access object */
static int strfilemk_objloadbegin(op,pr,objname)
STRFILEMK	*op ;
const char	*pr ;
const char	*objname ;
{
	MODLOAD	*lp = &op->loader ;

	int	rs ;


	{
		VECSTR		syms ;
	        const int	n = nelem(subs) ;
		int		opts = VECSTR_OCOMPACT ;
	        if ((rs = vecstr_start(&syms,n,opts)) >= 0) {
		    const int	symlen = SYMNAMELEN ;
	            int		i ;
		    int		snl ;
		    const char	**sv ;
		    char	symname[SYMNAMELEN + 1] ;

	            for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	                if (isrequired(i)) {
	                    rs = sncpy3(symname,symlen,objname,"_",subs[i]) ;
		            snl = rs ;
		            if (rs >= 0) 
			        rs = vecstr_add(&syms,symname,snl) ;
		        }
		        if (rs < 0) break ;
	            } /* end for */
        
	            if (rs >= 0)
	                rs = vecstr_getvec(&syms,&sv) ;
        
	            if (rs >= 0) {
	                const char	*modbname = STRFILEMK_MODBNAME ;
			opts = 0 ;
	                opts |= MODLOAD_OLIBVAR ;
			opts |= MODLOAD_OPRS ;
			opts |= MODLOAD_OSDIRS ;
	                rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
	            }

	            vecstr_finish(&syms) ;
	        } /* end if (allocation) */
		if (rs >= 0) {
		    int		mv[2] ;
		    if ((rs = modload_getmva(lp,mv,1)) >= 0) {
			void	*p ;
			op->objsize = mv[0] ;
			    if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
			        op->obj = p ;
			        rs = strfilemk_loadcalls(op,objname) ;
			        if (rs < 0) {
	    			    uc_free(op->obj) ;
	    			    op->obj = NULL ;
			        }
			    } /* end if (memory-allocation) */
		    } /* end if (modload_getmva) */
		    if (rs < 0)
			modload_close(lp) ;
		} /* end if (modload_open) */
	} /* end block */

	return rs ;
}
/* end subroutine (strfilemk_objloadbegin) */


static int strfilemk_objloadend(op)
STRFILEMK	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->obj != NULL) {
	    rs1 = uc_free(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    op->obj = NULL ;
	}

	rs1 = modload_close(&op->loader) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (strfilemk_objloadend) */


static int strfilemk_loadcalls(op,soname)
STRFILEMK	*op ;
const char	soname[] ;
{
	MODLOAD	*lp = &op->loader ;

	int	rs = SR_NOTFOUND ;
	int	i ;
	int	c = 0 ;

	char	symname[SYMNAMELEN + 1] ;

	const void	*snp ;


	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(symname,SYMNAMELEN,soname,"_",subs[i])) >= 0) {
	         if ((rs = modload_getsym(lp,symname,&snp)) == SR_NOTFOUND) {
		     snp = NULL ;
		     if (! isrequired(i)) rs = SR_OK ;
		}
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("strfilemk_loadcalls: call=%s %c\n",
		subs[i],
		((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {

	        c += 1 ;
		switch (i) {

		case sub_open:
		    op->call.open = (int (*)(void *,const char *,
			int,mode_t,int)) snp ;
		    break ;

		case sub_addfile:
		    op->call.addfile = (int (*)(void *,const char *,int)) snp ;
		    break ;

		case sub_abort:
		    op->call.abort = (int (*)(void *)) snp ;
		    break ;

		case sub_chgrp:
		    op->call.chgrp = (int (*)(void *,gid_t)) snp ;
		    break ;

		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (strfilemk_loadcalls) */


static int isrequired(i)
int	i ;
{
	int	f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_addfile:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


