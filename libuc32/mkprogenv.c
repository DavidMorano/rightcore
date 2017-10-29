/* mkprogenv */

/* make program environment */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special */
#define	CF_DEBUGCN	0		/* compile-time child */
#define	CF_UINFO	1		/* try to use |uinfo(3uc)| */
#define	CF_DEFPATH	0		/* use our local default PATH */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little honey creates an environment list for launching new
	processes.  It uses both the existing environment (if any) and also
	creates the minimium variables felt needed to give a new processes a
	fair chance.

	Synopsis:

	int mkprogenv_start(op,envv)
	MKPROGENV	*op ;
	const char	**envv ;

	Arguments:

	op		object pointer
	pr		program-root
	envv		environment array

	Returns:

	>=0		OK
	<0		error


	Synopsis:

	int mkprogenv_finish(op)
	MKPROGENV	*op ;

	Arguments:

	op		object pointer

	Returns:

	>=0		OK
	<0		error


	Notes:

        1. We use |gethz()| rather than |uc_sysconf()| because it may be faster.
        The |gethz()| function caches its value internally without having to go
        to the kernel. In some implementations, |uc_sysconf()| also caches some
        of its values, but we can never be sure.

        2. The compile-time CF_DEFPATH allows for grabbing a default path from
        the system LOGIN configuration. This takes a little bit of time so we
        now feel that it is not a good idea. A better idea is to use some sort
        of system-call or pseudo system-call that stores that (LOGIN
        configuration) value. Something along these lines of using the system
        call |uc_sysconf()| to grab the CS_PATH value, which in some systems
        caches a default path (the CS_PATH) valiable.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/systeminfo.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<envlist.h>
#include	<uinfo.h>
#include	<userattr.h>
#include	<getxusername.h>
#include	<localmisc.h>

#include	"mkprogenv.h"


/* local defines */

#ifndef	DEFLOGFNAME
#define	DEFLOGFNAME	"/etc/default/login"
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(MAXHOSTNAMELEN + 40)
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#define	NENVS		150

#define	DEFPATH		"/usr/preroot/bin:/usr/xpg4/bin:/usr/bin:/usr/extra/bin"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy3w(char *,int,const char *,const char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	ctdecul(char *,int,ulong) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	nisdomainname(char *,int) ;
extern int	getpwd(char *,int) ;
extern int	gethz(int) ;
extern int	vecstr_envfile(vecstr *,const char *) ;
extern int	hasvarpathprefix(const char *,int) ;
extern int	mkvarpath(char *,const char *,int) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy3w(char *,int,cchar *,cchar *,cchar *,int) ;


/* external variables */

extern const char	**environ ; /* secretly it's 'char **' */


/* forward reference */

static int	mkprogenv_mkenv(MKPROGENV *,cchar **) ;
static int	mkprogenv_mkenvdef(MKPROGENV *,ENVLIST *,const char **) ;
static int	mkprogenv_mkenvsys(MKPROGENV *,ENVLIST *,const char **) ;
static int	mkprogenv_mkenvextras(MKPROGENV *,ENVLIST *,const char **) ;
static int	mkprogenv_envadd(MKPROGENV *,ENVLIST *,cchar *,cchar *,int) ;
static int	mkprogenv_userinfo(MKPROGENV *) ;

#if	CF_DEFPATH
static int	mkprogenv_mkenvpath(MKPROGENV *,ENVLIST *,cchar *,cchar *) ;
#else
static int	mkprogenv_cspath(MKPROGENV *,ENVLIST *) ;
#endif /* CF_DEFPATH */


/* local variables */

static cchar	*envbad[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"TERM",
	"TERMDEV",
	"TMOUT",
	"PWD",
	NULL
} ;

static cchar	*envsys[] = {
	"SYSNAME",
	"RELEASE",
	"VERSION",
	"MACHINE",
	"ARCHITECTURE",
	"HZ",
	"NODE",
	"TZ",
	"NISDOMAIN",
	NULL
} ;

static cchar	*envdef[] = {
	"LD_LIBRARY_PATH",
	"LD_RUN_PATH",
	"NISDOMAIN",
	"NODE",
	"DOMAIN",
	"USERNAME",
	"USER",
	"LOGNAME",
	"HOME",
	"TZ",
	"MAIL",
	"MAILDIR",
	"LANG",
	"LC_COLLATE",
	"LC_CTYPE",
	"LC_MESSAGES",
	"LC_MONETARY",
	"LC_NUMERIC",
	"LC_TIME",
	NULL
} ;

static cchar	*envextra[] = {
	"USERNAME",
	"HOME",
	NULL
} ;

enum envextra {
	extraenv_username,
	extraenv_home,
	extraenv_overlast
} ;


/* exported subroutines */


int mkprogenv_start(MKPROGENV *op,cchar **envv)
{
	int		rs ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("mkprogenv_start: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(MKPROGENV)) ;
	op->envv = (envv != NULL) ? envv : environ ;

	opts = VECHAND_OCOMPACT ;
	if ((rs = vechand_start(&op->env,NENVS,opts)) >= 0) {
	    const int	size = 256 ;
	    if ((rs = strpack_start(&op->stores,size)) >= 0) {
	        rs = mkprogenv_mkenv(op,envv) ;
	        if (rs < 0)
	            strpack_finish(&op->stores) ;
	    }
	    if (rs < 0)
	        vechand_finish(&op->env) ;
	} /* end if (vechand_start) */

#if	CF_DEBUGS
	debugprintf("mkprogenv_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkprogenv_start) */


int mkprogenv_finish(MKPROGENV *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->uh != NULL) {
	    rs1 = uc_free(op->uh) ;
	    if (rs >= 0) rs = rs1 ;
	    op->uh = NULL ;
	}

	op->un[0] = '\0' ;

	rs1 = strpack_finish(&op->stores) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->env) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("mkprogenv_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkprogenv_finish) */


int mkprogenv_envset(MKPROGENV *op,cchar *kp,cchar *vp,int vl)
{
	vechand		*elp = &op->env ;
	int		rs ;
	int		rs1 ;
	int		size = 1 ; /* terminating NUL */
	char		*p ;

	if (kp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkprogenv_envset: k=%s\n",kp) ;
	if (vp != NULL)
	    debugprintf("mkprogenv_envset: v=%s\n",vp,vl) ;
#endif

	size += strlen(kp) ;
	size += 1 ;			/* for the equals sign character */
	if (vp != NULL) size += strnlen(vp,vl) ;

	if ((rs = uc_malloc(size,&p)) >= 0) {
	    const char	*ep ;
	    char	*bp = p ;
	    bp = strwcpy(bp,kp,-1) ;
	    *bp++ = '=' ;
	    if (vp != NULL) bp = strwcpy(bp,vp,vl) ;
	    if ((rs = strpack_store(&op->stores,p,(bp-p),&ep)) >= 0) {
	        rs1 = vechand_search(elp,ep,vstrkeycmp,NULL) ;
	        if (rs1 >= 0) vechand_del(elp,rs1) ;
	        rs = vechand_add(elp,ep) ;
	    }
	    uc_free(p) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (mkprogenv_envset) */


int mkprogenv_getvec(MKPROGENV *op,cchar ***eppp)
{
	vechand		*elp = &op->env ;
	int		rs ;

	if (eppp != NULL) {
	    rs = vechand_getvec(elp,eppp) ;
	} else {
	    rs = vechand_count(elp) ;
	}

	return rs ;
}
/* end subroutine (mkprogenv_getvec) */


/* private subroutines */


static int mkprogenv_mkenv(MKPROGENV *op,cchar **envv)
{
	ENVLIST		envtrack, *etp = &envtrack ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if ((rs = envlist_start(etp,NENVS)) >= 0) {
	    vechand	*elp = &op->env ;
	    int		f_path = FALSE ;
	    const char	*varpath = VARPATH ;
	    const char	*varpwd = VARPWD ;
	    const char	*kp ;

	    if ((rs >= 0) && (envv != NULL)) {
	        int	i ;
	        for (i = 0 ; (rs >= 0) && (envv[i] != NULL) ; i += 1) {
	            kp = envv[i] ;
	            if (matkeystr(envbad,kp,-1) < 0) {
	                if ((! f_path) && (kp[0] == 'P')) {
	                    f_path = (strkeycmp(kp,varpath) == 0) ;
	                }
	                n += 1 ;
	                if ((rs = vechand_add(elp,kp)) >= 0) {
	                    rs = envlist_add(etp,kp,-1) ;
	                }
	            } /* end if (good ENV variable) */
	        } /* end for */
	    } /* end if (ENV was specified) */

	    if ((rs >= 0) && (! f_path)) {
#if	CF_DEFPATH
	        {
	            rs = mkprogenv_mkenvpath(op,etp,varpath,DEFPATH) ;
	            n += rs ;
	        }
#else /* CF_DEFPATH */
	        rs = mkprogenv_cspath(op,etp) ;
	        n += rs ;
#endif /* CF_DEFPATH */
	    } /* end if (PATH) */

/* default environment variables */

	    if ((rs >= 0) && (envv == NULL)) {
	        rs = mkprogenv_mkenvdef(op,etp,envdef) ;
	        n += rs ;
	    }

/* system environment variables */

	    if (rs >= 0) {
	        if ((rs = mkprogenv_mkenvdef(op,etp,envsys)) >= 0) {
	            n += rs ;
	            if (rs < (nelem(envsys)-1)) {
	                rs = mkprogenv_mkenvsys(op,etp,envsys) ;
	                n += rs ;
	            }
	        }
	    } /* end if (system environment variables) */

/* USERNAME and HOME */

	    if (rs >= 0) {
	        rs = mkprogenv_mkenvextras(op,etp,envextra) ;
	        n += rs ;
	    } /* end if (extra environment variables) */

/* PWD */

	    if (rs >= 0) {
	        const int	rsn = SR_NOTFOUND ;
	        if ((rs = envlist_present(etp,varpwd,-1,NULL)) == rsn) {
	            char	pwd[MAXPATHLEN + 1] ;
	            if ((rs = getpwd(pwd,MAXPATHLEN)) > 0) {
	                n += 1 ;
	                rs = mkprogenv_envadd(op,etp,varpwd,pwd,rs) ;
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            } /* end if */
	        }
	    } /* end if (PWD) */

/* done */

	    rs1 = envlist_finish(etp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (envlist) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mkprogenv_mkenv) */


static int mkprogenv_mkenvdef(MKPROGENV *op,ENVLIST *etp,cchar **envs)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;
	const char	*kp ;
	const char	*cp ;

	for (i = 0 ; (rs >= 0) && (envs[i] != NULL) ; i += 1) {
	    kp = envs[i] ;
	    if ((rs = envlist_present(etp,kp,-1,NULL)) == rsn) {
	        rs = SR_OK ;
	        if ((cp = getourenv(op->envv,kp)) != NULL) {
	            n += 1 ;
	            rs = mkprogenv_envadd(op,etp,kp,cp,-1) ;
	        } /* end if */
	    } /* end if (adding a default ENV) */
	} /* end for (defualt ENVs) */

#if	CF_DEBUGS
	debugprintf("mkprogenv_mkenvdef: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mkprogenv_mkenvdef) */


static int mkprogenv_mkenvsys(MKPROGENV *op,ENVLIST *etp,cchar **envs)
{
	const int	vlen = VBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		vl ;
	int		n = 0 ;
#if	CF_UINFO
	UINFO_NAME	uname ;
#else
	struct utsname	un ;
#endif
	const char	*tp ;
	const char	*kp ;
	const char	*vp ;
	char		vbuf[VBUFLEN+1] = { 0 } ;

#if	CF_UINFO
	rs = uinfo_name(&uname) ;
#else
	rs = u_uname(&un) ;
#endif

	for (i = 0 ; (rs >= 0) && (envs[i] != NULL) ; i += 1) {
	    kp = envs[i] ;

#if	CF_DEBUGS
	    debugprintf("mkprogenv_mkenvsys: k=%s\n",kp) ;
#endif
	    if ((rs = envlist_present(etp,kp,-1,NULL)) == SR_NOTFOUND) {
	        const int	sc = MKCHAR(kp[0]) ;

	        rs = SR_OK ;
	        vp = NULL ;
	        vl = -1 ;
	        switch (sc) {
#if	CF_UINFO
	        case 'S':
	        case 'R':
	        case 'V':
	        case 'M':
	        case 'N':
	            switch (sc) {
	            case 'S':
	                vp = uname.sysname ;
	                break ;
	            case 'R':
	                vp = uname.release ;
	                break ;
	            case 'V':
	                vp = uname.version ;
	                break ;
	            case 'M':
	                vp = uname.machine ;
	                break ;
	            case 'N':
	                if (kp[1] == 'I') {
	                    rs = nisdomainname(vbuf,vlen) ;
	                    vl = rs ;
	                    vp = vbuf ;
	                } else {
	                    vp = uname.nodename ;
	                    if ((tp = strchr(vp,'.')) != NULL) {
	                        rs = snwcpy(vbuf,vlen,vp,(tp-vp)) ;
	                        vl = rs ;
	                        vp = vbuf ;
	                    }
	                } /* end if */
	                break ;
	            } /* end switch */
	            break ;
#else /* CF_UINFO */
	        case 'S':
	            vp = un.sysname ;
	            break ;
	        case 'R':
	            vp = un.release ;
	            break ;
	        case 'V':
	            vp = un.version ;
	            break ;
	        case 'M':
	            vp = un.machine ;
	            break ;
	        case 'N':
	            vp = un.nodename ;
	            if ((tp = strchr(vp,'.')) != NULL) {
	                rs = snwcpy(vbuf,vlen,vp,(tp-vp)) ;
	                vl = rs ;
	                vp = vbuf ;
	            }
	            break ;
#endif /* CF_INFO */
	        case 'A':
#ifdef	SI_ARCHITECTURE
	            rs1 = u_sysinfo(SI_ARCHITECTURE,vbuf,vlen) ;
	            vl = rs1 ;
	            if (rs1 >= 0) vp = vbuf ;
#endif /* SI_ARCHITECTURE */
	            break ;
	        case 'H':
	            if ((rs = gethz(0)) >= 0) {
	                vp = vbuf ;
	                rs = ctdecl(vbuf,vlen,rs) ;
	                vl = rs ;
	            }
	            break ;
	        case 'T':
	            if (op->un[0] == '\0') {
	                rs = mkprogenv_userinfo(op) ;
	            }
	            if (rs >= 0) {
	                USERATTR	a ;
	                if ((rs = userattr_open(&a,op->un)) >= 0) {
	                    {
	                        rs1 = userattr_lookup(&a,vbuf,vlen,"tz") ;
	                        if (rs1 >= 0) vp = vbuf ;
	                    }
	                    rs1 = userattr_close(&a) ;
	                    if (rs1 >= 0) rs = rs1 ;
	                } /* end if (userattr) */
	                if (isNotPresent(rs) || (rs == SR_NOSYS)) {
	                    rs = SR_OK ;
	                }
	            }
	            break ;
	        } /* end switch */

	        if ((rs >= 0) && (vp != NULL)) {
#if	CF_DEBUGS
	            debugprintf("mkprogenv_mkenvsys: a=>%t<\n",
	                vp,strlinelen(vp,vl,40)) ;
#endif
	            n += 1 ;
	            rs = mkprogenv_envadd(op,etp,kp,vp,vl) ;
	        } /* end if */

	    } /* end if (environment variable was not already present) */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("mkprogenv_mkenvsys: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mkprogenv_mkenvsys) */


static int mkprogenv_mkenvextras(MKPROGENV *op,ENVLIST *etp,cchar **envs)
{
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;
	int		f_home = FALSE ;
	int		f_username = FALSE ;
	const char	*var ;
	const char	*kp ;

	for (i = 0 ; (rs >= 0) && (envs[i] != NULL) ; i += 1) {
	    kp = envs[i] ;
	    if ((rs = envlist_present(etp,kp,-1,NULL)) == nrs) {
	        rs = SR_OK ;
#if	CF_DEBUGCN
	        nprintf(DEBFNAME,"uc_openprog: need i=%u var=%s\n",i,kp) ;
#endif
	        switch (i) {
	        case extraenv_username:
	            f_username = TRUE ;
	            break ;
	        case extraenv_home:
	            f_home = TRUE ;
	            break ;
	        } /* end switch */
	    } /* end if (not found) */
	    if (f_username && f_home) break ;
	} /* end for (extra ENVs) */

#if	CF_DEBUGCN
	nprintf(DEBFNAME,"uc_openprog: f_username=%u f_home=%u\n",
	    f_username,f_home) ;
#endif

	if ((rs >= 0) && (f_username || f_home)) {
	    if ((rs = mkprogenv_userinfo(op)) >= 0) {
	        if ((rs >= 0) && f_username) {
	            var = envs[extraenv_username] ;
	            rs = mkprogenv_envadd(op,etp,var,op->un,-1) ;
	            n += rs ;
	        } /* end if */
	        if ((rs >= 0) && f_home) {
	            var = envs[extraenv_home] ;
	            rs = mkprogenv_envadd(op,etp,var,op->uh,-1) ;
	            n += rs ;
	        } /* end if */
	    } /* end if (user-info) */
	} /* end if (needed them) */

#if	CF_DEBUGS
	debugprintf("mkprogenv_mkenvextras: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mkprogenv_mkenvextras) */


static int mkprogenv_envadd(MKPROGENV *op,ENVLIST *etp,cchar *kp,
		cchar *vp,int vl)
{
	vechand		*elp = &op->env ;
	int		rs ;
	int		kl = strlen(kp) ;
	int		bl = 0 ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("mkprogenv_envadd: k=%s\n",kp) ;
#endif


	bl += (kl+1) ;
	if (vp != NULL) bl += ((vl >= 0) ? vl : strlen(vp)) ;

	if ((rs = uc_malloc((bl+1),&bp)) >= 0) {
	    const char	*ep ;

	    strdcpy3w(bp,bl,kp,"=",vp,vl) ;

	    if ((rs = strpack_store(&op->stores,bp,bl,&ep)) >= 0) {
	        if ((rs = vechand_add(elp,ep)) >= 0) {
	            rs = envlist_add(etp,ep,kl) ;
	        }
	    } /* end if (store) */

	    uc_free(bp) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("mkprogenv_envadd: ret rs=%d bl=%u\n",rs,bl) ;
#endif

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (mkprogenv_envadd) */


#if	CF_DEFPATH
static int mkprogenv_mkenvpath(MKPROGENV *op,ENVLIST *etp,
cchar *varpath,cchar *defpath)
{
	vecstr		deflogin ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	const char	*tp = DEFLOGFNAME ;
	const char	*cp ;

	if ((rs = vecstr_start(&deflogin,10,0)) >= 0) {

	    cp = NULL ;
	    if ((rs = vecstr_envfile(&deflogin,tp)) >= 0) {

	        rs1 = vecstr_finder(&deflogin,varpath,vstrkeycmp,&cp) ;

	        if ((rs1 >= 0) && ((tp = strchr(cp,'=')) != NULL)) {
	            cp = (tp + 1) ;
	        }

	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    } /* end if (vecstr_envfile) */

	    if (cp == NULL) cp = defpath ;

	    if (rs >= 0) {
	        n += 1 ;
	        rs = mkprogenv_envadd(op,etp,varpath,cp,-1) ;
	    } /* end if */

	    rs1 = vecstr_finish(&deflogin) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUGS
	debugprintf("mkprogenv_mkenvpath: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mkprogenv_mkenvpath) */
#else /* CF_DEFPATH */
static int mkprogenv_cspath(MKPROGENV *op,ENVLIST *etp)
{
	const int	plen = (2*MAXPATHLEN) ;
	int		rs ;
	int		n = 0 ;
	cchar		*varpath = VARPATH ;
	char		*pbuf ;
	if ((rs = uc_malloc((plen+1),&pbuf)) >= 0) {
	    if ((rs = uc_confstr(_CS_PATH,pbuf,plen)) >= 0) {
	        rs = mkprogenv_envadd(op,etp,varpath,pbuf,rs) ;
	        n += rs ;
	    } /* end if */
	    uc_free(pbuf) ;
	} /* end if (m-a-f) */
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mkprogenv_cspath) */
#endif /* CF_DEFPATH */


static int mkprogenv_userinfo(MKPROGENV *op)
{
	int		rs = SR_OK ;
	if (op->un[0] == '\0') {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
	            cchar	*cp ;
	            strwcpy(op->un,pw.pw_name,USERNAMELEN) ;
	            if ((rs = uc_mallocstrw(pw.pw_dir,-1,&cp)) >= 0) {
	                op->uh = cp ;
	            }
	        } /* end if (getpwusername) */
	        uc_free(pwbuf) ;
	    } /* end if (m-a) */
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (mkprogenv_userinfo) */


