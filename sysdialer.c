/* sysdialer */

/* sysdialer storage object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAMEMODULE	1		/* try to use same module */
#define	CF_PRCACHE	1		/* PR-cache */
#define	CF_LOOKSELF	0		/* allow for compiled-in sysdialers */


/* revision history:

	- 2003-11-04, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages what sysdialers have been loaded so far.


*******************************************************************************/


#define	SYSDIALER_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<dlfcn.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<fsdir.h>
#include	<ids.h>
#include	<dirseen.h>
#include	<localmisc.h>

#include	"sysdialer.h"


/* local defines */

#define	PRCACHE		struct sysdialer_prcache

#define	TO_FILECHECK	3

#define	NEXTS		3		/* number of extensions */

#define	LIBCNAME	"lib"

#ifndef	SYSDIALERDNAME
#define	SYSDIALERDNAME	"sysdialers"
#endif

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,cchar *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strrchr(const char *,int) ;


/* external variables */


/* local structures */

struct fext {
	const char	*exp ;
	int		exl ;
} ;


/* forward references */

static int	sysdialer_sofind(SYSDIALER *,cchar *,cchar *,SYSDIALER_ENT *) ;
static int	sysdialer_sofindprs(SYSDIALER *,IDS *,DIRSEEN *,
			const char *,SYSDIALER_ENT *) ;
static int	sysdialer_sofindpr(SYSDIALER *,IDS *,DIRSEEN *,const char *,
			const char *,SYSDIALER_ENT *) ;
static int	sysdialer_sofindvar(SYSDIALER *,IDS *,DIRSEEN *,
			const char *,SYSDIALER_ENT *) ;
static int	sysdialer_socheckvarc(SYSDIALER *,IDS *,DIRSEEN *,
			cchar *,int,
			cchar *,SYSDIALER_ENT *) ;
static int	sysdialer_sochecklib(SYSDIALER *,IDS *,DIRSEEN *,cchar *,
			cchar *,SYSDIALER_ENT *) ;

#if	CF_LOOKSELF
static int	sysdialer_sofindlocal(SYSDIALER *,IDS *,
			cchar *,SYSDIALER_ENT *) ;
#endif

#ifdef	COMMENT
static int	sysdialer_sotest(SYSDIALER *,const char *) ;
#endif

static int	prcache_start(PRCACHE *) ;
static int	prcache_lookup(PRCACHE *,int,const char **) ;
static int	prcache_finish(PRCACHE *) ;

static int	entry_start(SYSDIALER_ENT *,cchar *,cchar *,SYSDIALER_MOD *) ;
static int	entry_checkdir(SYSDIALER_ENT *,cchar *,cchar *) ;
static int	entry_loadcalls(SYSDIALER_ENT *,void *) ;
static int	entry_hasname(SYSDIALER_ENT *,void *,const char *) ;
static int	entry_finish(SYSDIALER_ENT *) ;

static int	vecstr_loadexts(vecstr *,const char *,const char *,int) ;

static int	vcmpname(SYSDIALER_ENT **,SYSDIALER_ENT **) ;

static int	getext(struct fext *,const char *,int) ;


/* local variables */

static const char	*prnames[] = {
	"LOCAL",
	"PCS",
	"NCMP",
	"EXTRA",
	NULL
} ;

static const char	*exts[] = {
	"so",
	"o",
	"",
	NULL
} ;

static const char	*de64[] = {
	"sparcv9",
	"sparc",
	"",
	NULL
} ;

static const char	*de32[] = {
	"sparcv8",
	"sparcv7",
	"sparc",
	"",
	NULL
} ;

#if	_LP64
static const char	*dirs64[] = {
	"syssysdialer/sparcv9",
	"syssysdialer/sparc",
	"syssysdialer",
	"sysdialers/sparcv9",
	"sysdialers/sparc",
	"sysdialers",
	NULL
} ;
#else /* _LP64 */
static const char	*dirs32[] = {
	"syssysdialer/sparcv8",
	"syssysdialer/sparcv7",
	"syssysdialer",
	"sysdialers/sparcv8",
	"sysdialers/sparcv7",
	"sysdialers/sparc",
	"sysdialers",
	NULL
} ;
#endif /* _LP64 */

static const char	*subs[] = {
	"open",
	"reade",
	"recve",
	"recvfrome",
	"recvmsge",
	"write",
	"send",
	"sendto",
	"sendmsg",
	"shutdown",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_reade,
	sub_recve,
	sub_recvfrome,
	sub_recvmsge,
	sub_write,
	sub_send,
	sub_sendto,
	sub_sendmsg,
	sub_shutdown,
	sub_close,
	sub_overlast
} ;

static const int	rsnotconnected[] = {
	SR_PFNOSUPPORT,
	SR_AFNOSUPPORT,
	SR_NETUNREACH,
	SR_NETDOWN,
	SR_HOSTUNREACH,
	SR_HOSTDOWN,
	SR_TIMEDOUT,
	SR_CONNREFUSED,
	SR_NETRESET,
	SR_CONNABORTED,
	SR_CONNRESET,
	SR_NOENT,
	SR_COMM,
	SR_PROTO,
	0
} ;


/* exported subroutines */


int sysdialer_start(SYSDIALER *op,cchar *pr,cchar **prs,cchar **dirs)
{
	int		rs = SR_OK ;
	int		i ;
	int		size ;
	int		opts ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	memset(op,0,sizeof(SYSDIALER)) ;

	rs = uc_mallocstrw(pr,-1,&cp) ;
	if (rs < 0)
	    goto bad0 ;

	op->pr = cp ;
	if (prs != NULL) {

	    rs = vecstr_start(&op->prlist,5,0) ;
	    if (rs < 0)
	        goto bad1 ;

	    op->f.vsprs = TRUE ;
	    for (i = 0 ; prs[i] != NULL ; i += 1) {
	        if (strcmp(prs[i],op->pr) != 0) {
	            rs = vecstr_add(&op->prlist,prs[i],-1) ;
	        }
	        if (rs < 0) break ;
	    } /* end if */

	    if (rs < 0)
	        goto bad2 ;

	} /* end if (had program roots) */

	if (dirs != NULL) {

	    rs = vecstr_start(&op->dirlist,10,0) ;
	    if (rs < 0)
	        goto bad2 ;

	    op->f.vsdirs = (rs >= 0) ;
	    for (i = 0 ; (rs >= 0) && (dirs[i] != NULL) ; i += 1) {
	        rs = vecstr_add(&op->dirlist,dirs[i],-1) ;
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0)
		vecstr_getvec(&op->dirlist,&op->dirs) ;

	    if (rs < 0)
	        goto bad3 ;

	} else {

#ifdef	_LP64
	    op->dirs = dirs64 ;
#else
	    op->dirs = dirs32 ;
#endif /* _LP64 */

	} /* end if */

	size = sizeof(SYSDIALER_ENT) ;
	opts = VECOBJ_OSORTED ;
	if ((rs = vecobj_start(&op->entries,size,10,opts)) >= 0) {
	    if ((rs = prcache_start(&op->pc)) >= 0){
	        op->magic = SYSDIALER_MAGIC ;
	    }
	    if (rs < 0)
	        vecobj_finish(&op->entries) ;
	} /* end if (vecobj_start) */

ret0:
	return rs ;

/* bad stuff */
bad3:
	vecstr_finish(&op->dirlist) ;

bad2:
	if (op->f.vsprs) {
	    vecstr_finish(&op->prlist) ;
	}

bad1:
	uc_free(op->pr) ;
	op->pr = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (sysdialer_start) */


/* free up and get out */
int sysdialer_finish(op)
SYSDIALER	*op ;
{
	SYSDIALER_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSDIALER_MAGIC) return SR_NOTOPEN ;

	rs1 = prcache_finish(&op->pc) ;
	if (rs >= 0) rs = rs1 ;

	for (i = 0 ; vecobj_get(&op->entries,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs1 = entry_finish(ep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	rs1 = vecobj_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	if (op->f.vsdirs) {
	    rs1 = vecstr_finish(&op->dirlist) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->f.vsprs) {
	    rs1 = vecstr_finish(&op->prlist) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (sysdialer_finish) */


/* load a sysdialer */
int sysdialer_loadin(SYSDIALER *op,cchar *name,SYSDIALER_ENT **depp)
{
	SYSDIALER_MOD	*mp ;
	SYSDIALER_ENT	se, e, *dep ;
	int		rs ;
	int		i ;
	int		size ;
	int		f_alloc = FALSE ;
	void		*dhp ;

	if ((name == NULL) || (name[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("sysdialer_loadin: name=%s\n",name) ;
#endif

	se.name = name ;
	rs = vecobj_search(&op->entries,&se,vcmpname,depp) ;

	if (rs >= 0) {
	    (*depp)->count += 1 ;
	    goto ret0 ;
	}

/* search for it in any modules that we have already loaded */

#if	CF_SAMEMODULE
	for (i = 0 ; (rs = vecobj_get(&op->entries,i,&dep)) >= 0 ; i += 1) {
	    if (dep != NULL) {
	        mp = dep->mp ;
	        dhp = mp->dhp ;
	        rs = entry_hasname(dep,dhp,name) ;
	    }
	    if (rs >= 0) break ;
	} /* end for */

	if (rs >= 0) {

	    rs = entry_start(&e,name,NULL,mp) ;
	    if (rs < 0)
	        goto ret0 ;

	    rs = entry_loadcalls(&e,dhp) ;
	    if (rs < 0)
	        entry_finish(&e) ;

	    goto ret1 ;
	}
#endif /* CF_SAMEMODULE */

/* create a new load module descriptor */

	size = sizeof(SYSDIALER_MOD) ;
	rs = uc_malloc(size,&mp) ;
	if (rs < 0)
	    goto bad0 ;

	f_alloc = TRUE ;
	memset(mp,0,size) ;

/* initialize a SYSDIALER entry */

	rs = entry_start(&e,name,NULL,mp) ;
	if (rs < 0)
	    goto bad1 ;

/* search for it in the filesystem */

	rs = sysdialer_sofind(op,op->pr,name,&e) ;
	if (rs < 0)
	    goto bad2 ;

	dhp = e.mp->dhp ;

/* got it */

	rs = entry_loadcalls(&e,dhp) ;
	if (rs < 0)
	    goto bad2 ;

/* save this entry */
ret1:
	rs = vecobj_add(&op->entries,&e) ;
	i = rs ;
	if (rs < 0)
	    goto bad2 ;

/* return point-to-entry to our caller */

	if (depp != NULL) {
	    vecobj_get(&op->entries,i,depp) ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("sysdialer_loadin: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	entry_finish(&e) ;

bad1:
	if (f_alloc)
	    uc_free(mp) ;

bad0:
	goto ret0 ;
}
/* end subroutine (sysdialer_loadin) */


int sysdialer_loadout(SYSDIALER *op,cchar *name)
{
	SYSDIALER_ENT	te, *dep ;
	int		rs ;
	int		ei ;

	if (op == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (op->magic != SYSDIALER_MAGIC) return SR_NOTOPEN ;

	if (name[0] == '\0') return SR_INVALID ;

	te.name = name ;
	if ((rs = vecobj_search(&op->entries,&te,vcmpname,&dep)) >= 0) {
	    ei = rs ;
	    if (dep->count <= 1) {
	        entry_finish(dep) ;
	        vecobj_del(&op->entries,ei) ;
	    } else {
	        dep->count -= 1 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (sysdialer_loadout) */


int sysdialer_check(SYSDIALER *op,time_t daytime)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSDIALER_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0) daytime = time(NULL) ;

	op->ti_lastcheck = daytime ;

	return SR_OK ;
}
/* end subroutine (sysdialer_check) */


/* private subroutines */


static int sysdialer_sofind(op,pr,soname,ep)
SYSDIALER	*op ;
const char	pr[] ;
const char	soname[] ;
SYSDIALER_ENT	*ep ;
{
	IDS		id ;
	int		rs ;

	if ((rs = ids_load(&id)) >= 0) {
	DIRSEEN		ds ;

	rs = SR_NOENT ;

#if	CF_LOOKSELF
	if ((rs < 0) && isOneOf(rsnotconnected,rs))
	    rs = sysdialer_sofindlocal(op,&id,soname,ep) ;
#endif

	if ((rs < 0) && isOneOf(rsnotconnected,rs)) {
	    if ((rs = dirseen_start(&ds)) >= 0) {

	        rs = sysdialer_sofindpr(op,&id,&ds,pr,soname,ep) ;

	        if ((rs < 0) && isOneOf(rsnotconnected,rs))
	            rs = sysdialer_sofindprs(op,&id,&ds,soname,ep) ;

	        if ((rs < 0) && isOneOf(rsnotconnected,rs))
	            rs = sysdialer_sofindvar(op,&id,&ds,soname,ep) ;

	        dirseen_finish(&ds) ;
	    } /* end if (dirseen) */
	} /* end if */

	ids_release(&id) ;
	} /* end if (ids) */

	return rs ;
}
/* end subroutine (sysdialer_sofind) */


#if	CF_LOOKSELF
static int sysdialer_sofindlocal(op,idp,soname,ep)
SYSDIALER	*op ;
IDS		*idp ;
const char	soname[] ;
SYSDIALER_ENT	*ep ;
{
	SYSDIALER_INFO	*dip ;
	void		*dhp = NULL ;
	int		rs = SR_OK ;

	dhp = RTLD_SELF ;
	dip = (SYSDIALER_INFO *) dlsym(dhp,soname) ;

	rs = SR_NOTFOUND ;
	if ((dip != NULL) && (strcmp(dip->name,soname) == 0)) {
	                ep->size = dip->size ;
	                ep->flags = dip->flags ;
	                ep->mp->dhp = dhp ;
	                rs = SR_OK ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("sysdialer_sofindlocal: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysdialer_sofindlocal) */
#endif /* CF_LOOKSELF */


static int sysdialer_sofindprs(op,idp,dsp,soname,ep)
SYSDIALER	*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	soname[] ;
SYSDIALER_ENT	*ep ;
{
	int		rs = SR_NOENT ;
	int		rs1 ;
	int		i ;
	const char	*prp ;

	for (i = 0 ; prnames[i] != NULL ; i += 1) {

	    rs1 = prcache_lookup(&op->pc,i,&prp) ;

	    if (rs1 >= 0)
	        rs = sysdialer_sofindpr(op,idp,dsp,prp,soname,ep) ;

	    if (rs >= 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (sysdialer_sofindprs) */


static int sysdialer_sofindpr(op,idp,dsp,pr,soname,ep)
SYSDIALER	*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	pr[] ;
const char	soname[] ;
SYSDIALER_ENT	*ep ;
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;
	int		pathlen ;
	char		libdname[MAXPATHLEN + 1] ;
	char		pathbuf[MAXPATHLEN + 1] ;

	rs = mkpath2(libdname,pr,LIBCNAME) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = dirseen_havename(dsp,libdname,-1) ;
	if (rs1 >= 0) {
	    rs = SR_NOENT ;
	    goto ret0 ;
	}

	rs = u_stat(libdname,&sb) ;
	if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = SR_NOTDIR ;

	if (rs >= 0) {
	    rs1 = dirseen_havedevino(dsp,&sb) ;
	    if (rs1 >= 0) {
		rs = SR_NOENT ;
		goto ret0 ;
	    }
	}

	if (rs >= 0)
	    rs = sysdialer_sochecklib(op,idp,dsp,libdname,soname,ep) ;

	if ((rs < 0) && (rs != SR_NOMEM)) {
	    pathlen = pathclean(pathbuf,libdname,-1) ;
	    if (pathlen >= 0)
	        dirseen_add(dsp,pathbuf,pathlen,&sb) ;
	}

ret0:
	return rs ;
}
/* end subroutine (sysdialer_sofindpr) */


static int sysdialer_sofindvar(op,idp,dsp,soname,ep)
SYSDIALER	*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	soname[] ;
SYSDIALER_ENT	*ep ;
{
	int		rs = SR_NOENT ;
	const char	*sp = getenv(VARLIBPATH) ;
	const char	*tp ;

	if (sp != NULL) {

	while ((tp = strpbrk(sp,":;")) != NULL) {

	    if ((tp - sp) > 0) {

	        rs = sysdialer_socheckvarc(op,idp,dsp,sp,(tp - sp),
	            soname,ep) ;

	        if ((rs >= 0) || (rs == SR_NOMEM))
		    break ;

	    } /* end if (non-zero length) */

	    sp = (tp + 1) ;

	} /* end for */

	if ((rs < 0) && (rs != SR_NOMEM) && (sp[0] != '\0'))
	    rs = sysdialer_socheckvarc(op,idp,dsp,sp,-1,soname,ep) ;

	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (sysdialer_sofindvar) */


static int sysdialer_socheckvarc(op,idp,dsp,ldnp,ldnl,soname,ep)
SYSDIALER	*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	ldnp[] ;
int		ldnl ;
const char	soname[] ;
SYSDIALER_ENT	*ep ;
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;
	int		pl ;
	const char	*pp ;
	char		pathbuf[MAXPATHLEN + 1] ;

	pp = (const char *) pathbuf ;
	rs = pathclean(pathbuf,ldnp,ldnl) ;
	pl = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = SR_NOENT ;
	rs1 = dirseen_havename(dsp,pp,pl) ;
	if (rs1 >= 0)
	    goto ret0 ;

	rs = u_stat(pp,&sb) ;
	if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = SR_NOTDIR ;

	if (rs >= 0) {
	    rs1 = dirseen_havedevino(dsp,&sb) ;
	    if (rs1 >= 0) {
		rs = SR_NOENT ;
		goto ret0 ;
	    }
	}

	if (rs >= 0)
	    rs = sysdialer_sochecklib(op,idp,dsp,pp,soname,ep) ;

	if ((rs < 0) && (rs != SR_NOMEM))
	     dirseen_add(dsp,pp,pl,&sb) ;

ret0:
	return rs ;
}
/* end subroutine (sysdialer_sofindvarc) */


static int sysdialer_sochecklib(op,idp,dsp,libdname,soname,ep)
SYSDIALER	*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	libdname[] ;
const char	soname[] ;
SYSDIALER_ENT	*ep ;
{
	struct ustat	sb ;
	int		rs = SR_NOENT ;
	int		rs1 ;
	int		i ;
	int		dsize ;
	const char	**dirs ;
	const char	*ldnp ;
	char		subdname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("sysdialer_sochecklib: libdname=%s\n",libdname) ;
	debugprintf("sysdialer_sochecklib: soname=%s\n",soname) ;
#endif

	dsize = sizeof(caddr_t) ;
	dirs = (dsize == 8) ? de64 : de32 ;

	for (i = 0 ; dirs[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	debugprintf("sysdialer_sochecklib: dir[%u]=%s\n",i,dirs[i]) ;
#endif

	    rs1 = SR_OK ;
	    ldnp = libdname ;
	    if (dirs[i][0] != '\0') {
		ldnp = subdname ;
	        rs1 = mkpath2(subdname,libdname,dirs[i]) ;
	    }

	    if (rs1 >= 0) {
		rs1 = u_stat(ldnp,&sb) ;
		if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode)))
		    rs1 = SR_NOTDIR ;
	    }

	    if ((rs1 < 0) && (rs1 != SR_NOMEM))
		dirseen_add(dsp,ldnp,-1,&sb) ;

	    if (rs1 >= 0) {
		rs = entry_checkdir(ep,ldnp,soname) ;

#if	CF_DEBUGS
	debugprintf("sysdialer_sochecklib: entry_checkdir() rs=%d\n",rs) ;
#endif

	    }

	    if ((rs >= 0) || (! isNotPresent(rs))) break ;
	} /* end for (dirs) */

#if	CF_DEBUGS
	debugprintf("sysdialer_sochecklib: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysdialer_sochecklib) */


#ifdef	COMMENT
static int sysdialer_sotest(op,soname)
SYSDIALER	*op ;
const char	soname[] ;
{
	SYSDIALER_INFO	*mip ;
	int		rs = SR_NOTFOUND ;

	mip = (SYSDIALER_INFO *) dlsym(op->sop,soname) ;

#if	CF_DEBUGS
	if (mip == NULL)
	debugprintf("sysdialer_sotest: dlsym() >%s<\n", dlerror()) ;
#endif

	if (mip != NULL) {
	    if (strcmp(mip->name,soname) == 0) {
	                ep->size = dip->size ;
	                ep->flags = dip->flags ;
	                ep->mp->dhp = dhp ;
		    rs = SR_OK ;
	    }
	}

	return rs ;
}
/* end subroutine (sysdialer_sotest) */
#endif /* COMMENT */


#if	CF_PRCACHE

static int prcache_start(pcp)
PRCACHE		*pcp ;
{
	int		rs ;
	int		size ;

	pcp->domainname = NULL ;
	size = (nelem(prnames) + 1) * sizeof(char *) ;
	if ((rs = uc_malloc(size,&pcp->prs)) >= 0) {
	    memset(pcp->prs,0,size) ;
	}

	return rs ;
}
/* end subroutine (prcache_start) */


static int prcache_finish(pcp)
PRCACHE		*pcp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (pcp->domainname != NULL) {
	    rs1 = uc_free(pcp->domainname) ;
	    if (rs >= 0) rs = rs1 ;
	    pcp->domainname = NULL ;
	}

	if (pcp->prs != NULL) {
	    for (i = 0 ; i < nelem(prnames) ; i += 1) {
		if (pcp->prs[i] != NULL) {
		    rs1 = uc_free(pcp->prs[i]) ;
	    	    if (rs >= 0) rs = rs1 ;
		}
	    } /* end for */
	    rs1 = uc_free(pcp->prs) ;
	    if (rs >= 0) rs = rs1 ;
	    pcp->prs = NULL ;
	}

	return rs ;
}
/* end subroutine (prcache_finish) */


static int prcache_lookup(pcp,i,rpp)
PRCACHE		*pcp ;
int		i ;
const char	**rpp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		size ;
	int		len = 0 ;
	char		pr[MAXPATHLEN + 1] ;

	*rpp = NULL ;
	if (i >= (nelem(prnames) - 1))
	    return 0 ;

	if (pcp->domainname == NULL) {
	    char	dn[MAXHOSTNAMELEN + 1] ;
	    if ((rs = getnodedomain(NULL,dn)) >= 0) {
		const char	*cp ;
		if ((rs = uc_mallocstrw(dn,-1,&cp)) >= 0) {
		    pcp->domainname = cp ;
		}
	    }
	} /* end if */

	if (rs < 0)
	    goto ret0 ;

	if (pcp->prs[i] == NULL) {

	    rs1 = mkpr(pr,MAXPATHLEN,prnames[i],pcp->domainname) ;
	    size = 0 ;
	    if (rs1 >= 0)
		size = rs1 ;

	    rs = uc_mallocstrw(pr,size,(pcp->prs + i)) ;

	} /* end if */

	if ((rs >= 0) && (pcp->prs[i][0] != '\0')) {
	    *rpp = pcp->prs[i] ;
	    if ((*rpp)[0] != '\0')
		len = strlen(*rpp) ;
	}

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (prcache_lookup) */

#endif /* CF_PRCACHE */


static int entry_start(ep,name,itype,mp)
SYSDIALER_ENT	*ep ;
const char	name[] ;
const char	itype[] ;
SYSDIALER_MOD	*mp ;
{
	int		rs = SR_OK ;

	memset(ep,0,sizeof(SYSDIALER_ENT)) ;

	rs = uc_mallocstrw(name,-1,&ep->name) ;
	if (rs < 0)
	    goto bad0 ;

	if ((itype != NULL) && (itype[0] != '\0')) {
	    rs = uc_mallocstrw(itype,-1,&ep->itype) ;
	    if (rs < 0)
		goto bad1 ;
	}

	if (mp != NULL) {
	    mp->count += 1 ;
	    ep->mp = mp ;
	}

	ep->count = 1 ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	uc_free(ep->name) ;
	ep->name = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (entry_start) */


static int entry_finish(ep)
SYSDIALER_ENT	*ep ;
{
	SYSDIALER_MOD	*mp ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (ep->name != NULL) {
	    rs1 = uc_free(ep->name) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->name = NULL ;
	}

	if (ep->itype != NULL) {
	    rs1 = uc_free(ep->itype) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->itype = NULL ;
	}

	if (ep->mp != NULL) {
	    mp = ep->mp ;
	    if (mp->count <= 1) {
	        if ((mp->dhp != NULL) && (mp->dhp != RTLD_DEFAULT)) {
	            dlclose(mp->dhp) ;
		}
	        rs1 = uc_free(mp) ;
		if (rs >= 0) rs = rs1 ;
		ep->mp = NULL ;
	    } else
	        mp->count -= 1 ;
	} /* end if */

	memset(ep,0,sizeof(SYSDIALER_ENT)) ;

	return rs ;
}
/* end subroutine (entry_finish) */


/* try to load a file with the given name */
static int entry_checkdir(ep,libdname,name)
SYSDIALER_ENT	*ep ;
const char	libdname[] ;
const char	name[] ;
{
	struct ustat	sb ;
	SYSDIALER_INFO	*dip ;
	vecstr		enames ;
	int		rs ;
	int		rs1 ;
	int		i ;
	int		dlmode ;
	int		namelen ;
	int		c ;
	int		fl = 0 ;
	void		*dhp = NULL ;
	const char	*fn ;
	const char	*sublibdname = SYSDIALERDNAME ;
	char		dname[MAXPATHLEN + 1] ;
	char		dlfname[MAXPATHLEN + 1] ;
	char		fname[MAXNAMELEN + 1] ;

#if	CF_DEBUGS
	debugprintf("sysdialer/entry_checkdir: libdname=%s\n",libdname) ;
#endif

	rs = mkpath2(dname,libdname,sublibdname) ;
	if (rs < 0)
	    goto ret0 ;

/* test for directory existing */

	rs = u_stat(dname,&sb) ;

#if	CF_DEBUGS
	debugprintf("sysdialer/entry_checkdir: dname=%s\n",dname) ;
	debugprintf("sysdialer/entry_checkdir: u_stat() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = SR_NOTDIR ;

	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_start(&enames,3,0) ;
	if (rs < 0)
	    goto ret0 ;

/* read the directory looking for the prefix name parts */

	namelen = strlen(name) ;

#if	CF_DEBUGS
	debugprintf("sysdialer/entry_checkdir: namelen=%u name=%s\n",
		namelen,name) ;
#endif

	rs = vecstr_loadexts(&enames,dname,name,namelen) ;
	c = rs ;
	if (rs < 0)
	    goto ret1 ;

/* load them in turn */

	rs = SR_NOTFOUND ;
	if (c == 0)
	    goto ret1 ;

	for (i = 0 ; exts[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("sysdialer/entry_checkdir: trying ext=%s\n",exts[i]) ;
#endif

	    if ((rs1 = vecstr_findn(&enames,exts[i],-1)) >= 0) {

	        rs = SR_INVALID ;
		fn = name ;
		if (exts[i][0] != '\0') {
		    fn = fname ;
	            mkfnamesuf1(fname,name,exts[i]) ;
		}

	        fl = mkpath2(dlfname,dname,fn) ;

#if	CF_DEBUGS
	        debugprintf("sysdialer/entry_checkdir: dfl=%u\n",
	            fl) ;
	        debugprintf("sysdialer/entry_checkdir: dlfname=%s\n",
	            dlfname) ;
#endif

	        dlmode = (RTLD_LAZY | RTLD_LOCAL) ;
	        if ((dhp = dlopen(dlfname,dlmode)) != NULL) {

	            dip = (SYSDIALER_INFO *) dlsym(dhp,name) ;

#if	CF_DEBUGS
	            debugprintf("sysdialer/entry_checkdir: dip{%p}\n",dip) ;
		    if (dip != NULL) {
	        	debugprintf("sysdialer/entry_checkdir: d_name=%s\n",
			    dip->name) ;
		    }
#endif

	            if ((dip != NULL) && (strcmp(dip->name,name) == 0)) {
	                ep->size = dip->size ;
	                ep->flags = dip->flags ;
	                ep->mp->dhp = dhp ;
	                rs = SR_OK ;
	                break ;
	            } else {
	                dlclose(dhp) ;
		    }

		} else {
#if	CF_DEBUGS
	            debugprintf("sysdialer/entry_checkdir: dlopen() >%s<\n",
	                dlerror()) ;
#endif
		    rs = SR_LIBACC ;
	        } /* end if */

	    } /* end if (tried one) */

	} /* end for (extensions) */

ret1:
	vecstr_finish(&enames) ;

ret0:

#if	CF_DEBUGS
	debugprintf("sysdialer/entry_checkdir: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fl : rs ;
}
/* end subroutine (entry_checkdir) */


/* load up the available subroutines from this module */
static int entry_loadcalls(ep,dhp)
SYSDIALER_ENT	*ep ;
void		*dhp ;
{
	int		rs = SR_OK ;
	int		nl, cl ;
	int		i ;
	int		(*fp)() ;
	char		symname[MAXNAMELEN + 1] ;
	char		*cp ;

	strwcpy(symname,ep->name,MAXNAMELEN) ;

	nl = strlen(ep->name) ;

	cp = (char *) (symname + nl) ;
	cl = MAXNAMELEN - nl ;

	for (i = 0 ; (rs >= 0) && (subs[i] != NULL) ; i += 1) {

	    rs = sncpy2(cp,cl,"_",subs[i]) ;

	    if (rs >= 0)
	        fp = (int (*)()) dlsym(dhp,symname) ;

#if	CF_DEBUGS
	    debugprintf("entry_loadcalls: sym=%s(%p)\n",symname,fp) ;
#endif

	    if ((rs >= 0) && (fp != NULL)) {
	        switch (i) {
	        case sub_open:
	            ep->c.open = fp ;
	            break ;
	        case sub_reade:
	            ep->c.reade = fp ;
	            break ;
	        case sub_recve:
	            ep->c.recve = fp ;
	            break ;
	        case sub_recvfrome:
	            ep->c.recvfrome = fp ;
	            break ;
	        case sub_recvmsge:
	            ep->c.recvmsge = fp ;
	            break ;
	        case sub_write:
	            ep->c.write= fp ;
	            break ;
	        case sub_send:
	            ep->c.send = fp ;
	            break ;
	        case sub_sendto:
	            ep->c.sendto = fp ;
	            break ;
	        case sub_sendmsg:
	            ep->c.sendmsg = fp ;
	            break ;
	        case sub_shutdown:
	            ep->c.shutdown = fp ;
	            break ;
	        case sub_close:
	            ep->c.close = fp ;
	            break ;
	        } /* end switch */
	    } /* end if (got a symbol) */

	} /* end for */

/* do a minimal check */

	if ((rs >= 0) && (ep->c.open == NULL))
	    rs = SR_LIBACC ;

#if	CF_DEBUGS
	debugprintf("entry_loadcalls: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (entry_loadcalls) */


static int entry_hasname(ep,dhp,name)
SYSDIALER_ENT	*ep ;
void		*dhp ;
const char	name[] ;
{
	SYSDIALER_INFO	*dip ;
	int		rs = SR_NOTFOUND ;

	dip = (SYSDIALER_INFO *) dlsym(dhp,name) ;

	if ((dip != NULL) && (strcmp(dip->name,name) == 0)) {
	    ep->size = dip->size ;
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (entry_hasname) */


static int vecstr_loadexts(lp,dname,name,namelen)
vecstr		*lp ;
const char	dname[] ;
const char	name[] ;
int		namelen ;
{
	struct fext	e ;
	fsdir		dir ;
	fsdir_ent	slot ;
	int		rs ;
	int		dnl ;
	int		nl ;
	int		c = 0 ;
	char		*dnp ;

#if	CF_DEBUGS
	debugprintf("sysdialer/vecstr_loadexts: dname=%s\n",dname) ;
	debugprintf("sysdialer/vecstr_loadexts: nl=%d name=%t\n",
		namelen,name,namelen) ;
#endif

	rs = fsdir_open(&dir,dname) ;
	if (rs < 0)
	    goto ret0 ;

	    while ((dnl = fsdir_read(&dir,&slot)) > 0) {

	        dnp = slot.name ;
	        if (dnl < namelen)
	            continue ;

#if	CF_DEBUGS
	debugprintf("sysdialer/vecstr_loadexts: dnp=%s\n",dnp) ;
#endif

		nl = getext(&e,dnp,dnl) ;

#if	CF_DEBUGS
	        debugprintf("sysdialer/vecstr_loadexts: getext() rs=%d\n",nl) ;
	        debugprintf("sysdialer/vecstr_loadexts: ext=%t\n",
			e.exp,e.exl) ;
#endif

	        if (nl != namelen)
	            continue ;

 		if (strncmp(dnp,name,namelen) != 0)
	            continue ;

#if	CF_DEBUGS
	        debugprintf("sysdialer/vecstr_loadexts: el=%u ext=%t\n",
			e.exl,e.exp,e.exl) ;
#endif

	        if ((e.exl == 0) || (matstr(exts,e.exp,e.exl) >= 0)) {

#if	CF_DEBUGS
	            debugprintf("sysdialer/vecstr_loadexts: matched\n") ;
#endif

	            c += 1 ;
	            rs = vecstr_add(lp,e.exp,e.exl) ;
		    if (rs < 0)
			break ;

	            if (c >= NEXTS)
	                break ;

	        } /* end if (got a match) */

	    } /* end while (directory entries) */

	    fsdir_close(&dir) ;

ret0:

#if	CF_DEBUGS
	debugprintf("sysdialer/vecstr_loadexts: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_loadexts) */


/* compare the whole entries (including the netgroup) */
static int vcmpname(e1pp,e2pp)
SYSDIALER_ENT	**e1pp, **e2pp ;
{

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	return strcmp((*e1pp)->name,(*e2pp)->name) ;
}
/* end subroutine (vcmpname) */


static int getext(ep,name,namelen)
struct fext	*ep ;
const char	name[] ;
int		namelen ;
{
	int		mnl ;
	const char	*tp ;

#if	CF_DEBUGS && 0
	debugprintf("sysdialer/getext: name=%t\n",
		name,namelen) ;
#endif

	mnl = strnlen(name,namelen) ;

#if	CF_DEBUGS && 0
	debugprintf("sysdialer/getext: mnl=%d\n",mnl) ;
#endif

	ep->exp = (name + mnl) ;
	ep->exl = 0 ;

	if ((tp = strnrchr(name,mnl,'.')) != NULL) {

#if	CF_DEBUGS && 0
	debugprintf("sysdialer/getext: el=%d\n",(name + mnl - (tp + 1))) ;
#endif

		ep->exp = (tp + 1) ;
		ep->exl = (name + mnl - (tp + 1)) ;
		mnl = (tp - name) ;
	}

#if	CF_DEBUGS && 0
	debugprintf("sysdialer/getext: ret=%d\n",mnl) ;
#endif

	return mnl ;
}
/* end subroutine (getext) */


