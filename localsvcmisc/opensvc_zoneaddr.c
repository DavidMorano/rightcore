/* opensvc_zoneabbr */

/* LOCAL facility open-service (zoneabbr) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_GETUSERHOME	1		/* use 'getuserhome(3dam)' */
#define	CF_SETENTRY	0		/* need 'subinfo_setentry()' */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_zoneabbr(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<char.h>
#include	<vecstr.h>
#include	<getxusername.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<localmisc.h>

#include	"opensvc_zoneabbr.h"
#include	"defs.h"


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#define	NDEBFNAME	"opensvc_zoneabbr.deb"

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	statvfsdir(const char *,struct statvfs *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;


/* local structures */

struct subinfo_flags {
	uint		stores:1 ;
	uint		blocks:1 ;
} ;

struct subinfo {
	SUBINFO_FL	f ;
	SUBINFO_FL	open ;
	vecstr		stores ;
	int		to ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;
#if	CF_SETENTRY
static int	subinfo_setentry(SUBINFO *,cchar **,cchar *,int) ;
#endif

static int	subinfo_procuser(SUBINFO *,char *,int,const char *) ;
static int	mkfsline(char *,int,int,const char *,struct statvfs *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"sn",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_overlast
} ;


/* exported subroutines */


int opensvc_zoneabbr(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	SUBINFO		si, *sip = &si ;
	BITS		pargs ;
	KEYOPT		akopts ;
	const int	ulen = USERNAMELEN ;
	const int	llen = LINEBUFLEN ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		pan = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		ll = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f ;
	const char	*un = NULL ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char		ubuf[USERNAMELEN+1] ;
	char		lbuf[LINEBUFLEN+1] ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

	rs = subinfo_start(sip) ;
	if (rs < 0) goto badsubstart ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	f_akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		int ch = (argp[1] & 0xff) ;

	        if (isdigitlatin(ch)) {

	            argval = (argp+1) ;

	        } else if (ch == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr > 0) {
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        pr = argp ;
			    } else
	                        rs = SR_INVALID ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            prn = avp ;
	                    } else {
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            prn = argp ;
			    } else
	                        rs = SR_INVALID ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* program-root */
	                    case 'R':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
			    } else
	                        rs = SR_INVALID ;
	                        break ;

	                    case 'o':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
			    } else
	                        rs = SR_INVALID ;
	                        break ;

/* specify blocks as output rather than GigiBytes */
	                    case 'b':
	                        sip->f.blocks = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					sip->f.blocks = (rs > 0) ;
				    }
	                        }
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0) goto badarg ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logwelcome: ai_pos=%u ai_max=%u\n",
		ai_pos,ai_max) ;
#endif

/* find the username to act upon */

	if (argv != NULL) {
	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            switch (pan) {
	            case 0:
	                un = argv[ai] ;
		        break ;
	            } /* end switch */
	            pan += 1 ;
		}
    
	    } /* end for (handling positional arguments) */
	} /* end if (have arguments) */

/* default user as necessary */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_zoneabbr: 0 un=%s\n",un) ;
#endif

	if ((un == NULL) || (un[0] == '\0')) {
	    un = getourenv(envv,VARMOTDUSER) ;
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_zoneabbr: MOTD-un=%s\n",un) ;
#endif
	    if ((un == NULL) || (un[0] == '\0')) {
	        const char	*uidp = getourenv(envv,VARMOTDUID) ;
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_zoneabbr: MOTD-uid=%s\n",uidp) ;
#endif
	        if ((uidp != NULL) && (uidp[0] != '\0')) {
	            int		v ;
	            uid_t	uid ;
	            rs = cfdeci(uidp,-1,&v) ;
	            uid = v ;
	            if (rs >= 0) {
	                un = ubuf ;
	                rs = getusername(ubuf,ulen,uid) ;
	            }
	        }
	    }
	} /* end if (need a username) */
	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = "-" ;
	if (rs < 0) goto badret ;

/* process this user */

	rs = subinfo_procuser(sip,lbuf,llen,un) ;
	ll = rs ;
	if (rs < 0) goto badret ;

/* write it out */

	if ((rs = u_pipe(pipes)) >= 0) {
	    int	wfd = pipes[1] ;
	    fd = pipes[0] ;

	    rs = u_write(wfd,lbuf,ll) ;

	    u_close(wfd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if */

badret:
badarg:
	if (f_akopts) {
	    f_akopts = FALSE ;
	    rs1 = keyopt_finish(&akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = bits_finish(&pargs) ;
	if (rs >= 0) rs = rs1 ;

badpargs:
	rs1 = subinfo_finish(sip) ;
	if (rs >= 0) rs = rs1 ;

badsubstart:
ret0:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_zoneabbr) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (sip == NULL) return SR_FAULT ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->to = -1 ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip == NULL) return SR_FAULT ;

	if (sip->open.stores) {
	    sip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&sip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


#if	CF_SETENTRY
int subinfo_setentry(SUBINFO *sip,cchar **epp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		vnlen = 0 ;

	if (! sip->open.stores) {
	    rs = vecstr_start(&sip->stores,0,0) ;
	    sip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(&sip->stores,*epp) ;
	    }
	    if (vp != NULL) {
		int	i ;
	        vnlen = strnlen(vp,vl) ;
	        if ((rs = vecstr_add(&sip->stores,vp,vnlen)) >= 0) {
	            const char	*cp ;
	            i = rs ;
	            if ((rs = vecstr_get(&sip->stores,i,&cp)) >= 0) {
	                *epp = cp ;
		    }
	        } /* end if (added new entry) */
	    } /* end if (had a new entry) */
	} /* end if */
	if ((rs >= 0) && (oi >= 0)) {
	    vecstr_del(&sip->stores,oi) ;
	}
	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (subinfo_setentry) */
#endif /* CF_SETENTRY */


static int subinfo_procuser(SUBINFO *sip,char *lbuf,int llen,cchar *un)
{
	const int	hlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		f_blocks = sip->f.blocks ;
	int		ll = 0 ;
	char		hbuf[MAXPATHLEN+1] ;

#if	CF_GETUSERHOME /* not perfect enough for mixed environments */
	if ((un != NULL) && (un[0] != '\0') && (un[0] != '-')) {
	    rs = getuserhome(hbuf,hlen,un) ;
	} else {
	    struct passwd	pw ;
	    char	pwbuf[PWBUFLEN+1] ;
	    if ((rs = getpwusername(&pw,pwbuf,PWBUFLEN,-1)) >= 0)
	        rs = sncpy1(hbuf,hlen,pw.pw_dir) ;
	}
#else
	{
	    struct passwd	pw ;
	    char	pwbuf[PWBUFLEN+1] ;
	    if ((un != NULL) && (un[0] != '\0') && (un[0] != '-')) {
	        rs = GETPW_NAME(&pw,pwbuf,PWBUFLEN,un) ;
	    } else
	        rs = getpwusername(&pw,pwbuf,PWBUFLEN,-1) ;
	    if (rs >= 0)
	        rs = sncpy1(hbuf,hlen,pw.pw_dir) ;
	} /* end block */
#endif /* CF_GETUSERHOME */

	if (rs >= 0) {
	    struct statvfs	fss ;

	    if ((rs = statvfsdir(hbuf,&fss)) >= 0) {
	        rs = mkfsline(lbuf,llen,f_blocks,hbuf,&fss) ;
	        ll = rs ;
	    }

	} /* end if (have homedir) */

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (subinfo_procuser) */


static int mkfsline(lbuf,llen,f_blocks,fpath,fssp)
char		lbuf[] ;
int		llen ;
int		f_blocks ;
const char	*fpath ;
struct statvfs	*fssp ;
{
	LONG		vt ;
	LONG		vavail ;
	LONG		vtotal ;
	double		per = -1.0 ;
	int		rs = SR_OK ;
	int		ll = 0 ;

	vt = fssp->f_bavail * fssp->f_frsize ;
	vavail = vt / 1024 ;

	vt = fssp->f_blocks * fssp->f_frsize ;
	vtotal = vt / 1024 ;

	vt = fssp->f_blocks - fssp->f_bavail ;
	if (fssp->f_blocks != 0) {
	    double	fn = ((double) (vt * 100)) ;
	    double	fd = ((double) fssp->f_blocks) ;
	    per = fn/fd ;
	}

/* we go through some trouble here to get a snuggled-up floating number */

	{
	    const char	*dp ;
	    char	digbuf[DIGBUFLEN+1] ;
	    if (per < 0) per = 99.9 ;
	    rs = bufprintf(digbuf,DIGBUFLEN,"%4.1f",per) ;
	    if (rs >= 0) {
	        const char *fmt ;
		if (f_blocks) {
	            fmt = "%s avail=%llu total=%llu util=%s%%\n" ;
		} else {
	            fmt = "%s avail=%llu­GBi total=%llu­GBi util=%s%%\n" ;
		    vavail /= (1024*1024) ;
		    vtotal /= (1024*1024) ;
		}
	        dp = digbuf ;
	        while (dp[0] && CHAR_ISWHITE(*dp)) dp += 1 ;
	        rs = bufprintf(lbuf,llen,fmt,fpath,vavail,vtotal,dp) ;
	        ll = rs ;
	    }
	}

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mkfsline) */


