/* attachso */

/* attach a shared-object to the current process */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was originally written for hardware CAD support.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine implements an interface (a trivial one) that attempts to
        load a named module.

	Synopsis:

	int attachso(dnames,oname,exts,syms,dlmode,ropp)
	const char	*dnames[] ;
	const char	*oname ;
	const char	*exts[] ;
	const char	*syms[] ;
	int		dlmode ;
	void		**ropp ;

	Arguments:

	dnames		string array of directories to search
	oname		root-name of SO object file
	exts		array of externsions to consider
	syms		array of symbols
	dlmode		DLOPEN mode
	ropp		pointer to result object pointer

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<ids.h>
#include	<localmisc.h>


/* local defines */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	DNAME_SELF	"*SELF*"


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	mksofname(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isSpecialObject(void *) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotLib(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		id:1 ;
} ;

struct subinfo {
	void		**ropp ;
	const char	**dnames ;
	const char	*oname ;
	const char	**exts ;
	const char	**syms ;
	void		*sop ;
	SUBINFO_FL	f ;
	IDS		id ;
	int		dlmode ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar **,cchar *,
			cchar **,cchar **,int,void **) ;
static int	subinfo_soload(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *,int) ;

static int	subinfo_sofind(SUBINFO *) ;
static int	subinfo_socheck(SUBINFO *,IDS *,const char *) ;
static int	subinfo_checksyms(SUBINFO *) ;
static int	subinfo_modclose(SUBINFO *) ;


/* global variables */


/* local variables */

static const char	*defexts[] = {
	"so",
	"o",
	"",
	NULL
} ;

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int attachso(dnames,oname,exts,syms,m,ropp)
const char	*dnames[] ;
const char	*oname ;
const char	*exts[] ;
const char	*syms[] ;
int		m ;
void		**ropp ;
{
	SUBINFO		si ;
	int		rs ;
	int		rs1 ;
	int		f_abort ;

	if (dnames == NULL) return SR_FAULT ;
	if (oname == NULL) return SR_FAULT ;

	if (oname[0] == '\0') return SR_INVALID ;

	if ((rs = subinfo_start(&si,dnames,oname,exts,syms,m,ropp)) >= 0) {

	    rs = subinfo_soload(&si) ;
	    f_abort = (rs < 0) ;

#if	CF_DEBUGS
	    debugprintf("attachso: _soload() rs=%d\n",rs) ;
#endif

	    f_abort = (rs < 0) ;
	    rs1 = subinfo_finish(&si,f_abort) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("attachso: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (attachso) */


/* private subroutines */


static int subinfo_start(sip,dnames,oname,exts,syms,m,ropp)
SUBINFO		*sip ;
const char	*dnames[] ;
const char	oname[] ;
const char	*exts[] ;
const char	*syms[] ;
int		m ;
void		**ropp ;
{

	if (exts == NULL) exts = defexts ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->dnames = dnames ;
	sip->oname = oname ;
	sip->exts = exts ;
	sip->syms = syms ;
	sip->dlmode = m ;
	sip->ropp = ropp ;

	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip,f_abort)
SUBINFO		*sip ;
int		f_abort ;
{

	if (f_abort && (sip->ropp != NULL)) {
	    void	*sop = (void *) *(sip->ropp) ;
	    if ((sop != NULL) && (! isSpecialObject(sop))) dlclose(sop) ;
	    *(sip->ropp) = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_soload(sip)
SUBINFO		*sip ;
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("attachso/soload: ent\n") ;
#endif

	if ((rs = subinfo_sofind(sip)) >= 0) {
	    if (sip->ropp != NULL) {
		*(sip->ropp) = sip->sop ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("attachso/soload: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_soload) */


static int subinfo_sofind(sip)
SUBINFO		*sip ;
{
	IDS		id ;
	int		rs ;
	int		rs1 ;

	if ((rs = ids_load(&id)) >= 0) {
	    const int	soperm = (X_OK | R_OK) ;
	    int		i ;
	    int		f_open = FALSE ;
	    const char	**dnames = sip->dnames ;
	    const char	*dname ;
	    for (i = 0 ; dnames[i] != NULL ; i += 1) {
	        dname = dnames[i] ;
	        if (dname[0] != '\0') {
		    if (strcmp(dname,DNAME_SELF) == 0) {
			sip->sop = RTLD_SELF ;
			rs = subinfo_checksyms(sip) ;
		    } else {
		        struct ustat	sb ;
	                rs = u_stat(dname,&sb) ;
	                if ((rs >= 0) && (! S_ISDIR(sb.st_mode))) 
			    rs = SR_NOTDIR ;
	                if (rs >= 0) rs = sperm(&id,&sb,soperm) ;
	                if (rs >= 0) rs = subinfo_socheck(sip,&id,dname) ;
		    } /* end if */
		    if (rs >= 0) f_open = TRUE ;
	            if ((rs >= 0) || (! isNotLib(rs))) break ;
	        } /* end if */
	    } /* end for */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_open) 
		subinfo_modclose(sip) ;
	} /* end if (IDs) */

	return rs ;
}
/* end subroutine (subinfo_sofind) */


static int subinfo_socheck(sip,idp,dname)
SUBINFO		*sip ;
IDS		*idp ;
const char	dname[] ;
{
	struct ustat	sb ;
	const int	soperm = (R_OK | X_OK) ;
	int		rs = SR_OK ;
	int		rs1 = SR_NOTFOUND ;
	int		j ;
	int		f = FALSE ;
	const char	**exts = sip->exts ;
	char		sofname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("attachso/socheck: dname=%s\n",dname) ;
#endif

	sip->sop = NULL ;
	for (j = 0 ; (exts[j] != NULL) ; j += 1) {
	    cchar	*ext = exts[j] ;

#if	CF_DEBUGS
	    debugprintf("attachso/socheck: ext=%s\n",ext) ;
#endif

	    if ((rs1 = mksofname(sofname,dname,sip->oname,ext)) >= 0) {

	        rs1 = u_stat(sofname,&sb) ;
	        if ((rs1 >= 0) && (! S_ISREG(sb.st_mode)))
	            rs1 = SR_ISDIR ;

	        if (rs1 >= 0)
	            rs1 = sperm(idp,&sb,soperm) ;

#if	CF_DEBUGS
	        debugprintf("attachso/socheck: sperm() rs=%d\n",
	            rs1) ;
#endif

	        if (rs1 >= 0) {

	            sip->sop = dlopen(sofname,sip->dlmode) ;

#if	CF_DEBUGS
	            debugprintf("attachso/socheck: "
	                "dlopen() sop=%p >%s<\n",
	                sip->sop,
	                ((sip->sop != NULL) ? "ok" : dlerror() )) ;
#endif

	            if (sip->sop == NULL) rs1 = SR_NOENT ;

	            if (rs1 >= 0) {

	                rs1 = subinfo_checksyms(sip) ;

#if	CF_DEBUGS
	                debugprintf("attachso/socheck: "
	                    "subinfo_sotest() rs=%d\n",rs1) ;
#endif

	                if (rs1 < 0) {
	                    if (sip->sop != NULL) {
	    		        if (! isSpecialObject(sip->sop))
	                            dlclose(sip->sop) ;
	                        sip->sop = NULL ;
	                    }
	                    if (isOneOf(termrs,rs1)) rs = rs1 ;
	                } else
	                    f = TRUE ;

	            } /* end if */
	        } /* end if (file and perms) */

	    } /* end if (filename formed) */

	    if (sip->sop != NULL) break ;
	    if (rs < 0) break ;
	} /* end for (exts) */

	if (rs >= 0) {
	    if (sip->sop == NULL) rs = rs1 ;
	} else
	    sip->sop = NULL ;

#if	CF_DEBUGS
	debugprintf("attachso/socheck: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_socheck) */


static int subinfo_checksyms(sip)
SUBINFO		*sip ;
{
	int		rs = SR_OK ;
	const char	**syms = sip->syms ;

#if	CF_DEBUGS
	debugprintf("attachso/checksyms: ent\n") ;
#endif

	if (sip->sop == NULL) return SR_FAULT ;

	if (syms != NULL) {
	    const void	*symp ;
	    int		i ;
	    for (i = 0 ; (rs >= 0) && (syms[i] != NULL) ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("attachso/checksyms: sym%u=%s\n",
	            i,syms[i]) ;
#endif

	        symp = dlsym(sip->sop,syms[i]) ;
	        if (symp == NULL) rs = SR_NOTFOUND ;

	    } /* end for */
	} /* end if (syms) */

#if	CF_DEBUGS
	debugprintf("attachso/checksyms: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_checksyms) */


static int subinfo_modclose(SUBINFO *sip)
{
	if (sip->sop != NULL) {
	    if (! isSpecialObject(sip->sop)) {
		dlclose(sip->sop) ;
	    }
	    sip->sop = NULL ;
	}
	return SR_OK ;
}
/* end subroutine (subinfo_modclose) */


