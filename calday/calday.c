/* calday */

/* CALDAY object loader */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKSELF	0		/* try searching "SELF" for SO */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that
	allows access to the CALDAY datbase.


*******************************************************************************/


#define	CALDAY_MASTER		1


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
#include	<ids.h>
#include	<storebuf.h>
#include	<dirseen.h>
#include	<localmisc.h>

#include	"calday.h"
#include	"caldays.h"


/* local defines */

#undef	COMMENT

#define	CALDAY_DEFENTRIES	(44 * 1000)

#define	LIBDNAME		"lib"
#define	LIBCNAME		"lib"
#define	SONAME			"caldays"

#ifndef	VARLIBPATH
#define	VARLIBPATH		"LD_LIBRARY_PATH"
#endif

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	BVS_Q			CALDAYS_CITE
#define	BVS_C			CALDAYS_CUR


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mksofname(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	calday_objloadbegin(CALDAY *,const char *) ;
static int	calday_objloadend(CALDAY *) ;
static int	calday_sofind(CALDAY *,const char *,const char *,int) ;
static int	calday_sofindpr(CALDAY *,IDS *,DIRSEEN *,const char *,
			const char *,int) ;
static int	calday_sofindvar(CALDAY *,IDS *,DIRSEEN *,
			const char *,int) ;
static int	calday_socheckvarc(CALDAY *,IDS *,DIRSEEN *,
			const char *,int,
			const char *,int) ;
static int	calday_sochecklib(CALDAY *,IDS *,DIRSEEN *,const char *,
			const char *,int) ;
static int	calday_sotest(CALDAY *,const char *) ;
static int	calday_loadcalls(CALDAY *,const char *) ;

static int	isrequired(int) ;
static int	istermrs(int) ;


/* global variables */


/* local variables */

static const char	*exts[] = {
	"so",
	"o",
	"",
	NULL
} ;

static const char	*dirs64[] = {
	"sparcv9",
	"sparc",
	"",
	NULL
} ;

static const char	*dirs32[] = {
	"sparcv8",
	"sparcv7",
	"sparc",
	"",
	NULL
} ;

static const char	*subs[] = {
	"open",
	"count",
	"curbegin",
	"lookcite",
	"read",
	"curend",
	"check",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_curbegin,
	sub_lookcite,
	sub_read,
	sub_curend,
	sub_check,
	sub_audit,
	sub_close,
	sub_overlast
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


int calday_open(op,pr,dirnames,calnames)
CALDAY		*op ;
const char	pr[] ;
const char	*dirnames[] ;
const char	*calnames[] ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(CALDAY)) ;

	if ((rs = calday_objloadbegin(op,pr)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dirnames,calnames)) >= 0) {
		op->magic = CALDAY_MAGIC ;
	    }
	    if (rs < 0)
		calday_objloadend(op) ;
	} /* end if (objload-begin) */

#if	CF_DEBUGS
	debugprintf("calday_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calday_open) */


/* free up the entire vector string data structure object */
int calday_close(op)
CALDAY		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = calday_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (calday_close) */


int calday_count(op)
CALDAY		*op ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.count != NULL)
	    rs = (*op->call.count)(op->obj) ;

	return rs ;
}
/* end subroutine (calday_count) */


int calday_audit(op)
CALDAY		*op ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.audit != NULL)
	    rs = (*op->call.audit)(op->obj) ;

	return rs ;
}
/* end subroutine (calday_audit) */


int calday_check(op,daytime)
CALDAY		*op ;
time_t		daytime ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.check != NULL)
	    rs = (*op->call.check)(op->obj,daytime) ;

	return rs ;
}
/* end subroutine (calday_check) */


int calday_curbegin(op,curp)
CALDAY		*op ;
CALDAY_CUR	*curp ;
{
	int	rs = SR_OK ;


	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC) return SR_NOTOPEN ;

	if (op->call.curbegin != NULL)  {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
	        rs = (*op->call.curbegin)(op->obj,curp->scp) ;
		if (rs < 0) {
		    uc_free(curp->scp) ;
		    curp->scp = NULL ;
		    memset(curp,0,sizeof(CALDAY_CUR)) ;
		}
	    }
	} else
	    rs = SR_NOSYS ;

	return rs ;
}
/* end subroutine (calday_curbegin) */


int calday_curend(op,curp)
CALDAY		*op ;
CALDAY_CUR	*curp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL)  {
	    if (op->call.curend != NULL) {
	        rs = (*op->call.curend)(op->obj,curp->scp) ;
	    } else
	        rs = SR_NOSYS ;
	    rs1 = uc_free(curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->scp = NULL ;
	} else
	    rs = SR_NOTSOCK ;

	return rs ;
}
/* end subroutine (calday_curend) */


int calday_lookcite(op,curp,qp)
CALDAY		*op ;
CALDAY_CUR	*curp ;
CALDAY_QUERY	*qp ;
{
	CALDAYS_CUR	*ocurp ;

	int	rs = SR_NOSYS ;


	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL) {
	    if (op->call.lookcite != NULL) {
	        CALDAYS_QUERY	sq ;
	        sq.m = qp->m ;
	        sq.d = qp->d ;
	        ocurp = (CALDAYS_CUR *) curp->scp ;
	        rs = (*op->call.lookcite)(op->obj,ocurp,&sq) ;
	    } else
	        rs = SR_NOSYS ;
	} else
	    rs = SR_NOTSOCK ;

	return rs ;
}
/* end subroutine (calday_lookcite) */


int calday_read(op,curp,qp,rbuf,rlen)
CALDAY		*op ;
CALDAY_CUR	*curp ;
CALDAY_CITE	*qp ;
char		*rbuf ;
int		rlen ;
{
	CALDAYS_CITE	sq ;
	CALDAYS_CUR	*ocurp ;

	int	rs = SR_NOSYS ;


	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != CALDAY_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL) {
	    if (op->call.read != NULL) {
	        qp->m = 0 ;
	        qp->d = 0 ;
	        ocurp = (CALDAYS_CUR *) curp->scp ;
	        if ((rs = (*op->call.read)(op->obj,ocurp,&sq,rbuf,rlen)) >= 0) {
	            qp->m = sq.m ;
	            qp->d = sq.d ;
	        }
	    } else
	        rs = SR_NOSYS ;
	} else
	    rs = SR_NOTSOCK ;

	return rs ;
}
/* end subroutine (calday_read) */


/* private subroutines */


/* find and load the DB-access object */
static int calday_objloadbegin(op,pr)
CALDAY		*op ;
const char	pr[] ;
{
	int	rs = SR_NOENT ;
	int	dlmode = (RTLD_LAZY | RTLD_LOCAL) ;

	const char	*soname = SONAME ;


	    op->sop = RTLD_DEFAULT ;
	    rs = calday_sotest(op,soname) ;

#if	CF_LOOKSELF
	if ((rs < 0) && (! istermrs(rs))) {
	    op->sop = RTLD_SELF ;
	    rs = calday_sotest(op,soname) ;
	}
#endif /* CF_LOOKSELF */

#if	CF_DEBUGS
	debugprintf("calday_objloadbegin: calday_sotest() rs=%d\n",rs) ;
#endif

	if ((rs < 0) && (! istermrs(rs)))
	    rs = calday_sofind(op,pr,soname,dlmode) ;

#if	CF_DEBUGS
	debugprintf("calday_objloadbegin: calday_sofind() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    void	*p ;
	    if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		op->obj = p ;
		rs = calday_loadcalls(op,soname) ;
		if (rs < 0) {
		    uc_free(op->obj) ;
		    op->obj = NULL ;
		}
	    } /* end if (memory-allocation) */
	    if ((rs < 0) && (op->sop != NULL)) {
	        if (op->sop != RTLD_DEFAULT) dlclose(op->sop) ;
	        op->sop = NULL ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (calday_objloadbegin) */


static int calday_objloadend(op)
CALDAY		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->obj != NULL) {
	    rs1 = uc_free(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    op->obj = NULL ;
	}

	if (op->sop != NULL) {
	    if (op->sop != RTLD_DEFAULT)
	        dlclose(op->sop) ;
	    op->sop = NULL ;
	}

	return rs ;
}
/* end subroutine (calday_objloadend) */


static int calday_sofind(op,pr,soname,dlmode)
CALDAY		*op ;
const char	pr[] ;
const char	soname[] ;
int		dlmode ;
{
	IDS	id ;

	DIRSEEN	ds ;

	int	rs ;


	if ((rs = ids_load(&id)) >= 0) {
	if ((rs = dirseen_start(&ds)) >= 0) {

	    rs = calday_sofindpr(op,&id,&ds,pr,soname,dlmode) ;

#if	CF_DEBUGS
	    debugprintf("calday_sofind: calday_sofindpr() rs=%d\n",rs) ;
#endif

	    if ((rs < 0) && (! istermrs(rs)))
	        rs = calday_sofindvar(op,&id,&ds,soname,dlmode) ;

	    dirseen_finish(&ds) ;
	} /* end if (dirseen) */
	ids_release(&id) ;
	} /* end if */

	return rs ;
}
/* end subroutine (calday_sofind) */


static int calday_sofindpr(op,idp,dsp,pr,soname,dlmode)
CALDAY		*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	pr[] ;
const char	soname[] ;
int		dlmode ;
{
	struct ustat	sb ;

	int	rs ;
	int	rs1 ;
	int	pathlen ;

	char	libdname[MAXPATHLEN + 1] ;
	char	pathbuf[MAXPATHLEN + 1] ;


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
	    rs = calday_sochecklib(op,idp,dsp,libdname,soname,dlmode) ;

	if ((rs < 0) && (! istermrs(rs))) {
	    pathlen = pathclean(pathbuf,libdname,-1) ;
	    if (pathlen >= 0)
	        dirseen_add(dsp,pathbuf,pathlen,&sb) ;
	}

ret0:
	return rs ;
}
/* end subroutine (calday_sofindpr) */


static int calday_sofindvar(op,idp,dsp,soname,dlmode)
CALDAY		*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	soname[] ;
int		dlmode ;
{
	int	rs = SR_NOENT ;

	const char	*tp, *sp ;


	sp = getenv(VARLIBPATH) ;
	if (sp == NULL)
	    goto ret0 ;

	while ((tp = strpbrk(sp,":;")) != NULL) {

	    if ((tp - sp) > 0) {

	        rs = calday_socheckvarc(op,idp,dsp,sp,(tp - sp),
	            soname,dlmode) ;

	        if ((rs >= 0) || istermrs(rs))
		    break ;

	    } /* end if (non-zero length) */

	    sp = (tp + 1) ;

	} /* end for */

	if ((rs < 0) && (! istermrs(rs))) {
	    if (sp[0] != '\0')
	        rs = calday_socheckvarc(op,idp,dsp,sp,-1,soname,dlmode) ;
	}

ret0:
	return rs ;
}
/* end subroutine (calday_sofindvar) */


static int calday_socheckvarc(op,idp,dsp,ldnp,ldnl,soname,dlmode)
CALDAY		*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	ldnp[] ;
int		ldnl ;
const char	soname[] ;
int		dlmode ;
{
	struct ustat	sb ;

	int	rs ;
	int	rs1 ;
	int	pl ;

	const char	*pp ;
	char	pathbuf[MAXPATHLEN + 1] ;


	pp = (const char *) pathbuf ;
	rs = pathclean(pathbuf,ldnp,ldnl) ;
	pl = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = dirseen_havename(dsp,pp,pl) ;
	if (rs1 >= 0) {
	    rs = SR_NOENT ;
	    goto ret0 ;
	}

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
	    rs = calday_sochecklib(op,idp,dsp,pp,soname,dlmode) ;

	if ((rs < 0) && (! istermrs(rs)))
	     dirseen_add(dsp,pp,pl,&sb) ;

ret0:
	return rs ;
}
/* end subroutine (calday_sofindvarc) */


static int calday_sochecklib(op,idp,dsp,libdname,soname,dlmode)
CALDAY		*op ;
IDS		*idp ;
DIRSEEN		*dsp ;
const char	libdname[] ;
const char	soname[] ;
int		dlmode ;
{
	struct ustat	sb ;

	const int	soperm = (R_OK | X_OK) ;

	int	rs = SR_NOENT ;
	int	rs1 ;
	int	i, j ;
	int	dsize ;

	const char	**dirs ;
	const char	*ldnp ;

	char	subdname[MAXPATHLEN + 1] ;
	char	sofname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("calday_sochecklib: libdname=%s\n",libdname) ;
#endif

	dsize = sizeof(caddr_t) ;
	dirs = (dsize == 8) ? dirs64 : dirs32 ;

	for (i = 0 ; dirs[i] != NULL ; i += 1) {

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

	    if ((rs1 < 0) && (! istermrs(rs)))
		dirseen_add(dsp,ldnp,-1,&sb) ;

	    for (j = 0 ; (rs1 >= 0) && (exts[j] != NULL) ; j += 1) {

	        rs1 = mksofname(sofname,ldnp,soname,exts[j]) ;

	        if (rs1 >= 0) {

	            if ((rs = u_stat(sofname,&sb)) >= 0)
	                rs = sperm(idp,&sb,soperm) ;

	            if (rs >= 0) {

#if	CF_DEBUGS
	debugprintf("calday_sochecklib: sofname=%s\n",sofname) ;
#endif

	                op->sop = dlopen(sofname,dlmode) ;

#if	CF_DEBUGS
	debugprintf("calday_sochecklib: dlopen() >%s<\n",
		((op->sop != NULL) ? "ok" : dlerror() )) ;
#endif

	                if (op->sop == NULL)
	                    rs = SR_NOENT ;

			if (rs >= 0) {

			    rs = calday_sotest(op,soname) ;

			    if ((rs < 0) && (op->sop != NULL) &&
			        (op->sop != RTLD_DEFAULT))
				dlclose(op->sop) ;

			}
	            }

	        } /* end if */
		rs1 = SR_OK ;

	        if (rs >= 0) break ;
	    } /* end for (exts) */

	    if (rs >= 0) break ;
	} /* end for (dirs) */

	if (rs < 0)
	    op->sop = NULL ;

	return rs ;
}
/* end subroutine (calday_sochecklib) */


static int calday_sotest(op,soname)
CALDAY		*op ;
const char	soname[] ;
{
	CALDAYS_OBJ	*mip ;

	int	rs = SR_NOTFOUND ;


	if (op->sop == NULL)
	    return SR_FAULT ;

	mip = (CALDAYS_OBJ *) dlsym(op->sop,soname) ;

#if	CF_DEBUGS
	if (mip == NULL)
	debugprintf("calday_sotest: dlsym() >%s<\n", dlerror()) ;
#endif

	if (mip != NULL) {

	    if (strcmp(mip->name,soname) == 0) {
		op->objsize = mip->objsize ;
		op->cursize = mip->cursize ;
		if ((op->objsize > 0) && (op->cursize > 0))
		    rs = SR_OK ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (calday_sotest) */


static int calday_loadcalls(op,soname)
CALDAY		*op ;
const char	*soname ;
{
	int	rs = SR_NOTFOUND ;
	int	i ;
	int	c = 0 ;

	char	symname[SYMNAMELEN + 1] ;

	void	*snp ;


	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    rs = sncpy3(symname,SYMNAMELEN,soname,"_",subs[i]) ;
	    if (rs < 0)
		break ;

	    snp = dlsym(op->sop,symname) ;

	    if ((snp == NULL) && isrequired(i)) {
	        rs = SR_NOTFOUND ;
		break ;
	    }

#if	CF_DEBUGS
	    debugprintf("calday_loadcalls: call=%s %c\n",
		subs[i],
		((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {

	        c += 1 ;
		switch (i) {

		case sub_open:
		    op->call.open = (int (*)(void *,const char *,
				const char **,const char **)) snp ;
		    break ;

		case sub_count:
		    op->call.count = (int (*)(void *)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,BVS_C *)) snp ;
		    break ;

		case sub_lookcite:
		    op->call.lookcite = (int (*)(void *,BVS_C *,BVS_Q *)) snp ;
		    break ;

		case sub_read:
		    op->call.read = 
			(int (*)(void *,BVS_C *,BVS_Q *,char *,int)) snp ;
		    break ;

		case sub_curend:
		    op->call.curend= 
			(int (*)(void *,BVS_C *)) snp ;
		    break ;

		case sub_check:
		    op->call.check = (int (*)(void *,time_t)) snp ;
		    break ;

		case sub_audit:
		    op->call.audit = (int (*)(void *)) snp ;
		    break ;

		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (calday_loadcalls) */


static int isrequired(i)
int	i ;
{
	int	f = FALSE ;


	switch (i) {

	case sub_open:
	case sub_curbegin:
	case sub_lookcite:
	case sub_read:
	case sub_curend:
	case sub_close:
	    f = TRUE ;
	    break ;

	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


static int istermrs(rs)
int	rs ;
{
	int	i ;
	int	f = FALSE ;


	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f)
		break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */



