/* modload */

/* module-load management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_FORCEPR	0		/* force having a PR */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that attempts to
        load another named module.

	Description:

	int modload_open(op,pr,modfname,modname,opts,syms)
	MODLOAD		*op ;
	const char	pr[] ;
	const char	modfname[] ;
	const char	modname[] ;
	int		opts ;
	const char	*syms[] ;

	Arguments:

	op		object pointer
	pr		program-root
	modfname	module file-name
	modname		module name (the name inside the so-file itself)
	opts		options
	syms		array of symbols

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#define	MODLOAD_MASTER		1


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
#include	<ids.h>
#include	<dirseen.h>
#include	<localmisc.h>

#include	"modload.h"


/* local defines */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	LIBCNAME	"lib"

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
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
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	mksofname(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		id:1 ;
} ;

struct subinfo {
	void		*op ;
	const char	*pr ;
	const char	*modfname ;
	const char	**syms ;
	SUBINFO_FL	f ;
	IDS		id ;
	int		opts ;
} ;


/* forward references */

static int	modload_objloadclose(MODLOAD *) ;

static int	subinfo_start(SUBINFO *,MODLOAD *,cchar *,
			cchar *,int,cchar **) ;
static int	subinfo_modload(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *,int) ;

static int	subinfo_objloadbegin(SUBINFO *) ;
static int	subinfo_objloadend(SUBINFO *) ;
static int	subinfo_sofind(SUBINFO *) ;
static int	subinfo_sofindprs(SUBINFO *,DIRSEEN *,int) ;
static int	subinfo_sofindsdirs(SUBINFO *,DIRSEEN *,int) ;
static int	subinfo_sofindpr(SUBINFO *,DIRSEEN *,int,cchar *) ;
static int	subinfo_sofindvar(SUBINFO *,DIRSEEN *,int) ;
static int	subinfo_socheckvarc(SUBINFO *,DIRSEEN *,cchar *,int,int) ;
static int	subinfo_sochecklib(SUBINFO *,DIRSEEN *,cchar *,int) ;
static int	subinfo_socheckliber(SUBINFO *,DIRSEEN *,cchar *,int) ;
static int	subinfo_sotest(SUBINFO *) ;
static int	subinfo_checksyms(SUBINFO *) ;


/* global variables */


/* local variables */

static cchar	*prnames[] = {
	"LOCAL",
	"NCMP",
	"PCS",
	"EXTRA",
	"PREROOT",
	NULL
} ;

static cchar	*sysprs[] = {
	"/usr/extra",
	"/usr/preroot",
	NULL
} ;

static cchar	*exts[] = {
	"so",
	"o",
	"",
	NULL
} ;

static cchar	*extdirs[] = {
	"sparcv9",
	"sparcv8",
	"sparcv7",
	"sparc",
	"",
	NULL
} ;


/* exported subroutines */


int modload_open(op,pr,modfname,modname,opts,syms)
MODLOAD		*op ;
const char	pr[] ;
const char	modfname[] ;
const char	modname[] ;
int		opts ;
const char	*syms[] ;
{
	SUBINFO		si ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (modfname == NULL) return SR_FAULT ;
	if (modfname == NULL) return SR_FAULT ;

#if	CF_FORCEPR
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_NOTFOUND ;
#endif /* CF_FORCEPR */

	if (modfname[0] == '\0') return SR_INVALID ;
	if (modfname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(MODLOAD)) ;
	op->modname = modname ;

	if ((rs = subinfo_start(&si,op,pr,modfname,opts,syms)) >= 0) {
	    if ((rs = subinfo_modload(&si)) >= 0) {
#if	CF_DEBUGS
	        debugprintf("modload_open: subinfo_modload() rs=%d\n",rs) ;
#endif
	        op->magic = MODLOAD_MAGIC ;
	    }
	    rs1 = subinfo_finish(&si,(rs<0)) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("modload_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (modload_open) */


/* free up the entire vector string data structure object */
int modload_close(MODLOAD *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MODLOAD_MAGIC) return SR_NOTOPEN ;

	rs1 = modload_objloadclose(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (modload_close) */


int modload_getsym(MODLOAD *op,cchar *symname,const void **vpp)
{
	int		rs = SR_OK ;
	const void	*rp ;

	if (op == NULL) return SR_FAULT ;
	if (symname == NULL) return SR_FAULT ;

	if (op->magic != MODLOAD_MAGIC) return SR_NOTOPEN ;

	if (symname[0] == '\0') return SR_INVALID ;

	if (op->sop != NULL) {
	    rp = dlsym(op->sop,symname) ;
	    if (rp == NULL)
	        rs = SR_NOTFOUND ;
	} else
	    rs = SR_NOTFOUND ;

	if (vpp != NULL) {
	    *vpp = (rs >= 0) ? rp : NULL ;
	}

	return rs ;
}
/* end subroutine (modload_getsym) */


int modload_getmv(MODLOAD *op,int vi)
{
	int		rs = SR_OK ;
	int		v = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MODLOAD_MAGIC) return SR_NOTOPEN ;

	if (vi < 0) return SR_INVALID ;

	if (op->midp != NULL) {
	    MODLOAD_MID	*mip = op->midp ;
	    v = mip->mv[vi] ;
	} else
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (modload_getmv) */


int modload_getmva(MODLOAD *op,int *mva,int mvn)
{
	int		rs = SR_OK ;
	int		v = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (mva == NULL) return SR_FAULT ;

	if (op->magic != MODLOAD_MAGIC) return SR_NOTOPEN ;

	if (op->midp != NULL) {
	    MODLOAD_MID	*mip = op->midp ;
	    int		i ;
	    v = mip->mv[0] ;
	    for (i = 0 ; i < mvn ; i += 1) {
	        mva[i] = mip->mv[i] ;
	    }
	} else
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (modload_getmva) */


/* private subroutines */


static int modload_objloadclose(MODLOAD *op)
{
	if (op->sop != NULL) {
	    if ((op->sop != RTLD_DEFAULT) && (op->sop != RTLD_SELF)) {
	        dlclose(op->sop) ;
	    }
	    op->sop = NULL ;
	}
	return SR_OK ;
}
/* end subroutine (modload_objloadclose) */


static int subinfo_start(sip,op,pr,modfname,opts,syms)
SUBINFO		*sip ;
MODLOAD		*op ;
const char	pr[] ;
const char	modfname[] ;
int		opts ;
const char	*syms[] ;
{
	int		rs ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->op = op ;
	sip->pr = pr ;
	sip->modfname = modfname ;
	sip->opts = opts ;
	sip->syms = syms ;
	rs = ids_load(&sip->id) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip,int f_abort)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (f_abort) {
	    MODLOAD	*op = (MODLOAD *) sip->op ;
	    rs1 = modload_objloadclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_modload(SUBINFO *sip)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("modload/subinfo_modload: ent\n") ;
#endif

	if ((rs = subinfo_objloadbegin(sip)) >= 0) {
	    rs = subinfo_checksyms(sip) ;
	    if (rs < 0)
	        subinfo_objloadend(sip) ;
	} /* end if (objload-begin) */

#if	CF_DEBUGS
	debugprintf("modload/subinfo_modload: ret rs=%d\n", rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_modload) */


static int subinfo_objloadbegin(SUBINFO *sip)
{
	int		rs ;

	    rs = subinfo_sofind(sip) ;

#if	CF_DEBUGS
	debugprintf("modload/subinfo_objloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_objloadbegin) */


static int subinfo_objloadend(SUBINFO *sip)
{
	MODLOAD		*op = (MODLOAD *) sip->op ;
	int		rs ;

	rs = modload_objloadclose(op) ;

	return rs ;
}
/* end subroutine (subinfo_objloadend) */


static int subinfo_sofind(SUBINFO *sip)
{
	DIRSEEN		ds ;
	int		dlm = RTLD_LAZY ;
	int		rs ;
	int		rs1 ;

	dlm |= (sip->opts & MODLOAD_OAVAIL) ? RTLD_GLOBAL : RTLD_LOCAL ;

	    if ((rs = dirseen_start(&ds)) >= 0) {

	        if (sip->pr != NULL) {
	            rs = subinfo_sofindpr(sip,&ds,dlm,sip->pr) ;
		}

	        if ((rs < 0) && isNotPresent(rs)) {
	            if (sip->opts & MODLOAD_OLIBVAR) {
	                rs = subinfo_sofindvar(sip,&ds,dlm) ;
		    }
	        }

	        if ((rs < 0) && isNotPresent(rs)) {
	            if (sip->opts & MODLOAD_OPRS) {
	                rs = subinfo_sofindprs(sip,&ds,dlm) ;
		    }
	        }

	        if ((rs < 0) && isNotPresent(rs)) {
	            if (sip->opts & MODLOAD_OSDIRS) {
	                rs = subinfo_sofindsdirs(sip,&ds,dlm) ;
		    }
	        }

	        rs1 = dirseen_finish(&ds) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (dirseen) */

	return rs ;
}
/* end subroutine (subinfo_sofind) */


static int subinfo_sofindprs(SUBINFO *sip,DIRSEEN *dsp,int dlm)
{
	int		rs ;
	char		dn[MAXHOSTNAMELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("subinfo_sofindprs: ent\n") ;
#endif

	if ((rs = getnodedomain(NULL,dn)) >= 0) {
	    const int	prlen = MAXPATHLEN ;
	    int		i ;
	    char	prbuf[MAXPATHLEN + 1] ;
	    for (i = 0 ; prnames[i] != NULL ; i += 1) {
	        if ((rs = mkpr(prbuf,prlen,prnames[i],dn)) >= 0) {
	            rs = subinfo_sofindpr(sip,dsp,dlm,prbuf) ;
	        }
	        if ((rs >= 0) || (! isNotPresent(rs))) break ;
	    } /* end for */
	} /* end if (getnodedomain) */

#if	CF_DEBUGS
	debugprintf("subinfo_sofindprs: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_sofindprs) */


static int subinfo_sofindpr(SUBINFO *sip,DIRSEEN *dsp,int dlm,cchar *pr)
{
	int		rs ;
	char		libdname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("subinfo_sofindpr: ent\n") ;
#endif

	if ((rs = mkpath2(libdname,pr,LIBCNAME)) >= 0) {
	    const int	rsn = SR_NOTFOUND ;
	    if ((rs = dirseen_havename(dsp,libdname,-1)) == rsn) {
	        USTAT	sb ;
		if ((rs = u_stat(libdname,&sb)) >= 0) {
		    if (S_ISDIR(sb.st_mode)) {
	    	        if ((rs = dirseen_havedevino(dsp,&sb)) == rsn) {
	    		    rs = subinfo_sochecklib(sip,dsp,libdname,dlm) ;
			    if ((rs < 0) && isNotPresent(rs)) {
	        		char	pbuf[MAXPATHLEN + 1] ;
	    		        int pl = pathclean(pbuf,libdname,-1) ;
	    		        if (pl >= 0)
	        		    dirseen_add(dsp,pbuf,pl,&sb) ;
			    }
			} else if (rs >= 0) {
	        	    rs = SR_NOENT ;
			}
		    } else {
	    		rs = SR_NOTDIR ;
		    }
	       } /* end if (stat) */
	    } else if (rs >= 0) {
	        rs = SR_NOENT ;
	    }
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (subinfo_sofindpr) */


static int subinfo_sofindsdirs(SUBINFO *sip,DIRSEEN *dsp,int dlm)
{
	int		rs = SR_NOENT ;
	int		i ;
	const char	*dirname ;

#if	CF_DEBUGS
	debugprintf("subinfo_sofindsdirs: ent\n") ;
#endif

	for (i = 0 ; sysprs[i] != NULL ; i += 1) {
	    dirname = sysprs[i] ;
	    rs = subinfo_sofindpr(sip,dsp,dlm,dirname) ;
	    if ((rs >= 0) || (! isNotPresent(rs))) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (subinfo_sofindsdirs) */


static int subinfo_sofindvar(SUBINFO *sip,DIRSEEN *dsp,int dlm)
{
	int		rs = SR_NOENT ;
	int		sl ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("subinfo_sofindvar: ent\n") ;
#endif

	if ((sp = getenv(VARLIBPATH)) != NULL) {
	    cchar	*tp ;
	    while ((tp = strpbrk(sp,":;")) != NULL) {
	        sl = (tp-sp) ;
	        if (sl > 0) {
	            rs = subinfo_socheckvarc(sip,dsp,sp,sl,dlm) ;
	        } /* end if (non-zero length) */
	        sp = (tp + 1) ;
	        if ((rs >= 0) || (! isNotPresent(rs))) break ;
	    } /* end for */
	    if ((rs < 0) && isNotPresent(rs)) {
	        if (sp[0] != '\0') {
	            rs = subinfo_socheckvarc(sip,dsp,sp,-1,dlm) ;
		}
	    }
	} /* end if (getenv) */

	return rs ;
}
/* end subroutine (subinfo_sofindvar) */


static int subinfo_socheckvarc(SUBINFO *sip,DIRSEEN *dsp,
		cchar *ldnp,int ldnl,int dlm)
{
	int		rs ;
	const char	*pp ;
	char		pathbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("modload/subinfo_socheckvarc: libdname=%t\n",ldnp,ldnl) ;
#endif

	pp = (const char *) pathbuf ;
	if ((rs = pathclean(pathbuf,ldnp,ldnl)) >= 0) {
	    const int	rsn = SR_NOTFOUND ;
	    const int	pl = rs ;
	    if ((rs = dirseen_havename(dsp,pp,pl)) == rsn) {
	        struct ustat	sb ;
	        if ((rs = u_stat(pp,&sb)) >= 0) {
	            if (S_ISDIR(sb.st_mode)) {
	                if ((rs = dirseen_havedevino(dsp,&sb)) == rsn) {
	                    rs = subinfo_sochecklib(sip,dsp,pp,dlm) ;
	                    if ((rs < 0) && isNotPresent(rs)) {
	                        dirseen_add(dsp,pp,pl,&sb) ;
	                    }
	                } else if (rs >= 0) {
	                    rs = SR_NOENT ;
	                }
	            } else {
	                rs = SR_NOTDIR ;
	            }
	        } /* end if (stat) */
	    } else if (rs >= 0) {
	        rs = SR_NOENT ;
	    }
	} /* end if (pathclean) */

#if	CF_DEBUGS
	debugprintf("modload/subinfo_socheckvarc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_sofindvarc) */


static int subinfo_sochecklib(SUBINFO *sip,DIRSEEN *dsp,cchar *ldname,int dlm)
{
	struct ustat	sb ;
	MODLOAD		*op = (MODLOAD *) sip->op ;
	const int	am = (R_OK | X_OK) ;
	int		rs = SR_OK ;
	int		i ;
	const char	*ldnp ;
	char		subdname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("modload/subinfo_sochecklib: ldname=%s\n",ldname) ;
#endif

	if (dsp == NULL) return SR_FAULT ;

	op->sop = NULL ;
	for (i = 0 ; extdirs[i] != NULL ; i += 1) {
	    ldnp = ldname ;
	    if (extdirs[i][0] != '\0') {
	        ldnp = subdname ;
	        rs = mkpath2(subdname,ldname,extdirs[i]) ;
	    }
	    if (rs >= 0) {
	        if ((rs = u_stat(ldnp,&sb)) >= 0) {
	            if (S_ISDIR(sb.st_mode)) {
			IDS	*idp = &sip->id ;
			if ((rs = sperm(idp,&sb,am)) >= 0) {
		   	    rs = subinfo_socheckliber(sip,dsp,ldnp,dlm) ;
			} else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	debugprintf("modload/subinfo_sochecklib: sperm() rs=%d\n",rs) ;
	debugprintf("modload/subinfo_sochecklib: dn=%s\n",ldnp) ;
#endif
	    		    dirseen_add(dsp,ldnp,-1,&sb) ;
			    rs = SR_OK ;
			}
		    } else
			rs = SR_NOTDIR ;
		} else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	debugprintf("modload/subinfo_sochecklib: u_stat() rs=%d\n",rs) ;
	debugprintf("modload/subinfo_sochecklib: dn=%s\n",ldnp) ;
#endif
		    rs = SR_OK ;
		}
	    } /* end if (ok) */
	    if (op->sop != NULL) break ;
	    if (rs < 0) break ;
	} /* end for (extdirs) */
	if ((rs >= 0) && (op->sop == NULL)) rs = SR_LIBACC ;
#if	CF_DEBUGS
	debugprintf("modload/subinfo_sochecklib: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_sochecklib) */


/* ARGSUSED */
static int subinfo_socheckliber(SUBINFO *sip,DIRSEEN *dsp,cchar *ldnp,int dlm)
{
	MODLOAD		*op = sip->op ;
	IDS		*idp = &sip->id ;
	const int	am = (X_OK|R_OK) ;
	int		rs = SR_OK ;
	int		j ;
	cchar		*mfn = sip->modfname ;
	char		sfn[MAXPATHLEN + 1] ;
	for (j = 0 ; exts[j] != NULL ; j += 1) {
	    if ((rs = mksofname(sfn,ldnp,mfn,exts[j])) >= 0) {
	        struct ustat	sb ;
	        if ((rs = u_stat(sfn,&sb)) >= 0) {
		    if (S_ISREG(sb.st_mode)) {
			if ((rs = sperm(idp,&sb,am)) >= 0) {
			    void	*sop ;
			    if ((sop = dlopen(sfn,dlm)) != NULL) {
				op->sop = sop ;
				rs = subinfo_sotest(sip) ;
				if (rs < 0) {
				    dlclose(sop) ;
				    op->sop = NULL ;
				    if (isNotPresent(rs)) {
					rs = SR_OK ;
				    }
				} /* end if (error) */
			    } /* end if (dlopen) */
			} else if (isNotPresent(rs)) {
			    rs = SR_OK ;
			} /* end if (sperm) */
		    } else
			rs = SR_ISDIR ;
		} else if (isNotPresent(rs)) {
#if	CF_DEBUGS
		    debugprintf("modload/subinfo_socheckliber: "
			"u_stat() rs=%d\n",rs) ;
		    debugprintf("modload/subinfo_socheckliber: sdn=%s\n",sfn) ;
#endif
		    rs = SR_OK ;
		} /* end if (u_stat) */
	    } /* end if (mksofname) */
	    if (op->sop != NULL) break ;
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("modload/subinfo_socheckliber: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_socheckliber) */


static int subinfo_sotest(SUBINFO *sip)
{
	MODLOAD		*op = (MODLOAD *) sip->op ;
	int		rs = SR_NOTFOUND ;
	void		*vp ;

	if (op->sop == NULL) return SR_FAULT ;

	if ((vp = dlsym(op->sop,op->modname)) != NULL) {
	    MODLOAD_MID	*mip = (MODLOAD_MID *) vp ;
	    op->midp = mip ;
	    if (strcmp(mip->name,op->modname) == 0) {
		rs = SR_OK ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (subinfo_sotest) */


static int subinfo_checksyms(SUBINFO *sip)
{
	MODLOAD		*op = (MODLOAD *) sip->op ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("modload/subinfo_checksyms: ent\n") ;
#endif

	if (sip->syms != NULL) {
	    int		i ;
	    void	*p = NULL ;
	    for (i = 0 ; sip->syms[i] != NULL ; i += 1) {
#if	CF_DEBUGS
	        debugprintf("modload/subinfo_checksyms: sym%u=%s\n",
	            i,sip->syms[i]) ;
#endif
	        p = dlsym(op->sop,sip->syms[i]) ;
	        if (p == NULL) break ;
	    } /* end for */
	    if ((i > 0) && (p == NULL)) rs = SR_NOENT ;
	} /* end if (non-null) */

#if	CF_DEBUGS
	debugprintf("modload/subinfo_checksyms: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (subinfo_checksyms) */


