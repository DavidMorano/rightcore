/* opensvc_qotd */

/* LOCAL facility open-service (qotd) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_GETUSERHOME	1		/* use 'getuserhome(3dam)' */
#define	CF_LOCSETENT	0		/* need 'subinfo_setentry()' */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_qotd(pr,prn,of,om,argv,envv,to)
	cchar		*pr ;
	cchar		*prn ;
	int		of ;
	mode_t		om ;
	cchar		**argv ;
	cchar		**envv ;
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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<char.h>
#include	<vecstr.h>
#include	<getxusername.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<dayspec.h>
#include	<tmtime.h>
#include	<filebuf.h>
#include	<wordfill.h>
#include	<localmisc.h>

#include	"opensvc_qotd.h"
#include	"openqotd.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	COMBUFLEN	LINEBUFLEN

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	NDEBFNAME	"opensvc_qotd.deb"


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matkeystr(cchar **,cchar *,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	statvfsdir(cchar *,struct statvfs *) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getmjd(int,int,int) ;
extern int	getyrd(int,int,int) ;
extern int	ndigits(int,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	hasourmjd(cchar *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strdcpy1(char *,int,cchar *) ;


/* local structures */

struct subinfo_flags {
	uint		stores:1 ;
	uint		gmt:1 ;
	uint		mjd:1 ;
} ;

struct subinfo {
	SUBINFO_FL	f, final ;
	SUBINFO_FL	open ;
	vecstr		stores ;
	cchar		*pr ;
	int		linelen ;
	int		year ;
	int		mon ;
	int		mday ;
	int		yday ;
	int		to ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_mjd(SUBINFO *,cchar *) ;
static int	subinfo_tmtime(SUBINFO *) ;

#if	CF_LOCSETENT
static int	subinfo_setentry(SUBINFO *,cchar **,
			cchar *,int) ;
#endif


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"sn",
	"db",
	"to",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_db,
	argopt_to,
	argopt_overlast
} ;


/* exported subroutines */


int opensvc_qotd(cchar *pr,cchar *prn,int of,mode_t om,
		cchar **av,cchar **ev,int to)
{
	SUBINFO		si, *sip = &si ;
	BITS		pargs ;
	KEYOPT		akopts ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		pan = 0 ;
	int		v ;
	int		cl ;
	int		fd = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f ;
	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*dayspec = NULL ;
	cchar		*dbname = NULL ;
	cchar		*cp ;

	if (av != NULL) {
	    for (argc = 0 ; av[argc] != NULL ; argc += 1) ;
	}

	rs = subinfo_start(sip,pr) ;
	if (rs < 0) goto badsubstart ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	f_akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (av[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = av[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

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
	                    argp = av[++ai] ;
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
	                        argp = av[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            prn = argp ;
			    } else
	                        rs = SR_INVALID ;
	                    }
	                    break ;

/* database-name */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbname = avp ;
	                    } else {
	                    if (argr > 0) {
	                        argp = av[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            dbname = argp ;
			    } else
	                        rs = SR_INVALID ;
	                    }
	                    break ;

/* time-out */
	                case argopt_to:
			    cp = NULL ;
			    cl = -1 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
				    cl = avl ;
				}
	                    } else {
	                    if (argr > 0) {
	                        argp = av[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            cp = argp ;
				    cl = argl ;
				}
			    } else
	                        rs = SR_INVALID ;
	                    }
			    if (cp != NULL) {
				rs = cfdecti(cp,cl,&v) ;
				to = v ;
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
	                        argp = av[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
			    } else
	                        rs = SR_INVALID ;
	                        break ;

	                    case 'o':
	                    if (argr > 0) {
	                        argp = av[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    KEYOPT	*kop = &akopts ;
	                            rs = keyopt_loads(kop,argp,argl) ;
				}
			    } else
	                        rs = SR_INVALID ;
	                        break ;

/* time-out */
	                    case 't':
	                    if (argr > 0) {
	                        argp = av[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	     			    rs = cfdecti(argp,argl,&v) ;
	             		    to = v ;
	                        }
			    } else
	                        rs = SR_INVALID ;
	                        break ;

/* line width */
	                    case 'w':
	                    if (argr > 0) {
	                        argp = av[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	     			    rs = optvalue(argp,argl) ;
	             		    sip->linelen = rs ;
	                        }
			    } else
	                        rs = SR_INVALID ;
	                        break ;

/* use GMT */
	                    case 'z':
	                        sip->final.gmt = TRUE ;
	                        sip->f.gmt = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                sip->f.gmt = (rs > 0) ;
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
	nprintf(NDEBFNAME,"opensvc_qotd: ai_pos=%u ai_max=%u\n",
	    ai_pos,ai_max) ;
#endif

/* find the username to act upon */

	if (rs >= 0) {
	    cchar	*cp ;
	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (av[ai] != NULL)) ;
	        if (f) {
	            cp = av[ai] ;
		    if (cp[0] != '\0') {
	            	dayspec = cp ;
	                pan += 1 ;
	            	break ;
		    }
		} /* end if */

	    } /* end for (handling positional arguments) */
	} /* end if (ok) */

	if ((dayspec == NULL) || (dayspec[0] == '\0')) dayspec = "+" ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_qotd: dayspec=%s\n",dayspec) ;
#endif

/* default arguments */

	if (sip->linelen <= 0) {
	    cchar	*cp ;
	    if ((rs >= 0) && (argval != NULL)) {
	        rs = cfdeci(argval,-1,&v) ;
	        sip->linelen = v ;
	    }
	    if ((rs >= 0) && (sip->linelen <= 0)) {
	        if ((cp = getourenv(ev,VARCOLUMNS)) != NULL) {
	            rs = cfdeci(cp,-1,&v) ;
	            sip->linelen = v ;
	        }
	    }
	    if ((rs >= 0) && (sip->linelen <= 0)) {
	        sip->linelen = COLUMNS ;
	    }
	} /* end if (linelen) */

/* process */

	if (rs >= 0) {
	    if ((rs = subinfo_mjd(sip,dayspec)) >= 0) {
		const int	mjd = rs ;
		rs = openqotd(pr,mjd,of,to) ;
		fd = rs ;
	    } /* end if (subinfo_mjd) */
	} /* end if (ok) */

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

	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

badsubstart:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_qotd) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *pr)
{
	int		rs = SR_OK ;

	if (sip == NULL) return SR_FAULT ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
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


#if	CF_LOCSETENT
int subinfo_setentry(SUBINFO *sip,cchar **epp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (sip == NULL) return SR_FAULT ;

	if (epp == NULL) return SR_INVALID ;

	if (! sip->open.stores) {
	    rs = vecstr_start(&sip->stores,0,0) ;
	    sip->open.stores = (rs >= 0) ;
	}

/* find existing entry for later deletion */

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(&sip->stores,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        if ((rs = vecstr_add(&sip->stores,vp,len)) >= 0) {
	            rs = vecstr_get(&sip->stores,rs,epp) ;
		}
	    } else
		*epp = NULL ;
	} /* end if (ok) */

	if ((rs >= 0) && (oi >= 0)) {
	    vecstr_del(&sip->stores,oi) ;
	}

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (subinfo_setentry) */
#endif /* CF_LOCSETENT */


static int subinfo_mjd(SUBINFO *sip,cchar *dayspec)
{
	int		rs ;
	int		dl = strlen(dayspec) ;
	int		ch = (dayspec[0] & 0xff) ;
	cchar		*dp = dayspec ;

	if (ch == '+') {
	    if ((rs = subinfo_tmtime(sip)) >= 0) {
	        sip->f.mjd = TRUE ;
		rs = getmjd(sip->year,sip->mon,sip->mday) ;
	    }
	} else if ((rs = hasourmjd(dp,dl)) > 0) {
	    sip->f.mjd = TRUE ;
	} else {
	    DAYSPEC	ds ;
	    if ((rs = dayspec_load(&ds,dayspec,dl)) >= 0) {
		if ((ds.y < 0) || (ds.m < 0) || (ds.d < 0)) {
		    rs = subinfo_tmtime(sip) ;
		    if (ds.y < 0) ds.y = sip->year ;
		    if (ds.m < 0) ds.m = sip->mon ;
		    if (ds.d < 0) ds.d = sip->mday ;
		}
		if (rs >= 0) {
	     	    sip->f.mjd = TRUE ;
		    rs = getmjd(ds.y,ds.m,ds.d) ;
		}
	    } /* end if (dayspec_load) */
	} /* end if (dayspec handling) */

	return rs ;
}
/* end subroutine (subinfo_mjd) */


static int subinfo_tmtime(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		mday = sip->mday ;

	if (sip->mday <= 0) {
	    TMTIME	t ;
	    time_t	daytime = time(NULL) ;
	    rs = tmtime_localtime(&t,daytime) ;
	    sip->year = (t.year + TM_YEAR_BASE) ;
	    sip->mon = t.mon ;
	    sip->mday = t.mday ;
	    sip->yday = t.yday ;
	    mday = t.mday ;
	} else
	    mday = sip->mday ;

	return (rs >= 0) ? mday : rs ;
}
/* end subroutine (subinfo_tmtime) */


