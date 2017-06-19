/* opensvc_cotd */

/* LOCAL facility open-service (cotd) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_GETUSERHOME	1		/* use 'getuserhome(3dam)' */
#define	CF_LOCSETENT	0		/* need 'subinfo_setentry()' */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by borrowing from some other open-service.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_cotd(pr,prn,of,om,argv,envv,to)
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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<char.h>
#include	<vecstr.h>
#include	<getxusername.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<commandment.h>
#include	<dayspec.h>
#include	<tmtime.h>
#include	<filebuf.h>
#include	<wordfill.h>
#include	<localmisc.h>

#include	"opensvc_cotd.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	VARCOLUMNS
#define	VARCOLUMNS	"COLUMNS"
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	COMBUFLEN	LINEBUFLEN

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	NDEBFNAME	"opensvc_cotd.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	statvfsdir(const char *,struct statvfs *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getmjd(int,int,int) ;
extern int	getyrd(int,int,int) ;
extern int	ndigits(int,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS || CF_DEBUGN
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
	const char	*pr ;
	int		linelen ;
	int		year ;
	int		mon ;
	int		mday ;
	int		yday ;
	int		to ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char *) ;
static int	subinfo_finish(SUBINFO *) ;
#if	CF_LOCSETENT
static int	subinfo_setentry(SUBINFO *,cchar **,cchar *,int) ;
#endif

static int	subinfo_cotd(SUBINFO *,int,const char *,const char *) ;
static int	subinfo_procout(SUBINFO *,int,int,int,cchar *,int) ;
static int	subinfo_procoutline(SUBINFO *,FILEBUF *,
			int,int,int,cchar *,int) ;
static int	subinfo_tmtime(SUBINFO *) ;

#ifdef	COMMENT
static int	subinfo_procuser(SUBINFO *,char *,int,const char *) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"sn",
	"db",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_db,
	argopt_overlast
} ;

static const char	blanks[] = "                    " ;


/* exported subroutines */


int opensvc_cotd(pr,prn,of,om,argv,envv,to)
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
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		pan = 0 ;
	int		pipes[2] ;
	int		v ;
	int		fd = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*dayspec = NULL ;
	const char	*dbname = NULL ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
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
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
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

/* database-name */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbname = avp ;
	                    } else {
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            dbname = argp ;
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

/* line width */
	                    case 'w':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	             		    rs = optvalue(argp,argl) ;
	          		    sip->linelen = rs ;
	                        }
			    } else
	                        rs = SR_INVALID ;
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
	nprintf(NDEBFNAME,"opensvc_cotd: ai_pos=%u ai_max=%u\n",
	    ai_pos,ai_max) ;
#endif

/* find a possible "dayspec" to act upon */

	if (argv != NULL) {
	    const char	*cp ;
	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
		    if (cp[0] != '\0') {
	            	dayspec = cp ;
	                pan += 1 ;
	            	break ;
		    }
		} /* end if */

	    } /* end for (handling positional arguments) */
	} /* end if (have arguments) */

	if ((dayspec == NULL) || (dayspec[0] == '\0')) dayspec = "+" ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_cotd: dayspec=%s\n",dayspec) ;
#endif

/* default arguments */

	if (sip->linelen <= 0) {
	    const char	*cp ;
	    if ((rs >= 0) && (argval != NULL)) {
	        rs = cfdeci(argval,-1,&v) ;
	        sip->linelen = v ;
	    }
	    if ((rs >= 0) && (sip->linelen <= 0)) {
	        if ((cp = getourenv(envv,VARCOLUMNS)) != NULL) {
	            rs = cfdeci(cp,-1,&v) ;
	            sip->linelen = v ;
	        }
	    }
	    if ((rs >= 0) && (sip->linelen <= 0)) {
	        sip->linelen = COLUMNS ;
	    }
	} /* end if (linelen) */

/* write it out */

	if (rs >= 0) {
	if ((rs = u_pipe(pipes)) >= 0) {
	    int	wfd = pipes[1] ;
	    fd = pipes[0] ;

	    rs = subinfo_cotd(sip,wfd,dbname,dayspec) ;

	    u_close(wfd) ;
	    if (rs < 0) {
		u_close(fd) ;
		fd = -1 ;
	    }
	} /* end if (u_pipe) */
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
/* end subroutine (opensvc_cotd) */


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
static int subinfo_setentry(SUBINFO *sip,cchar **epp,cchar *vp,int vl)
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
	        rs = vecstr_store(&sip->stores,vp,len,epp) ;
	    } /* end if (added new entry) */
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(&sip->stores,oi) ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_setentry) */
#endif /* CF_LOCSETENT */


static int subinfo_cotd(SUBINFO *sip,int wfd,cchar *dbname,cchar *dayspec)
{
	COMMANDMENT	cmd, *cmp = &cmd ;
	int		rs ;

	if ((rs = commandment_open(cmp,sip->pr,dbname)) >= 0) {

#if	CF_DEBUGS
	    if (DEBUGLEVEL(3))
	        debugprintf("b_commandment/procspec: spec>%t<\n",
	            spec,strnlen(spec,speclen)) ;
#endif

	    if ((rs = commandment_max(cmp)) >= 0) {
	        int	max = rs ;
	        int	prec ;
	        int	n ;
	        int	ch ;
		int	dl = strlen(dayspec) ;

	        prec = ndigits(max,10) ;
		if (prec > sizeof(blanks)) prec = sizeof(blanks) ;

	        ch = (dayspec[0] & 0xff) ;
	        if (hasalldig(dayspec,dl) || (ch == '+')) {

	            if (ch == '+') {
	                rs = subinfo_tmtime(sip) ;
	                n = (max > 31) ? sip->yday : sip->mday ;
	            } else {
	                rs = cfdeci(dayspec,-1,&n) ;
	            }

	        } else {
	            DAYSPEC	ds ;

	            if ((rs = dayspec_load(&ds,dayspec,dl)) >= 0) {
			if ((ds.y < 0) || (ds.m < 0) || (ds.d < 0)) {
	                    rs = subinfo_tmtime(sip) ;
			    if (ds.y < 0) ds.y = sip->year ;
			    if (ds.m < 0) ds.m = sip->mon ;
			    if (ds.d < 0) ds.d = sip->mday ;
			}
	                if ((rs >= 0) && (max > 31)) {
			    rs = dayspec_yday(&ds) ;
			    n = rs ;
			} else
			    n = ds.d ;
		    } /* end if (dayspec_load) */

	        } /* end if */

	        if (rs >= 0) {
		    const int	clen = COMBUFLEN ;
		    char	cbuf[COMBUFLEN + 1] ;
	            int		cn = (n % max) ;
	            if (cn == 0) cn = max ;
	            if ((rs = commandment_get(cmp,cn,cbuf,clen)) >= 0) {
	                int	cl = rs ;
	                rs = subinfo_procout(sip,wfd,prec,cn,cbuf,cl) ;
	            }
	        } /* end if */

	    } /* end if (max) */

	    commandment_close(cmp) ;
	} /* end if (commandment) */

	return rs ;
}
/* end subroutine (subinfo_cotd) */


static int subinfo_procout(SUBINFO *sip,int wfd,int prec,int n,
		cchar *cbuf,int clen)
{
	FILEBUF		b ;
	int		rs ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	    debugprintf("b_commandment/procout: n=%u clen=%d\n",n,clen) ;
#endif

	if ((rs = filebuf_start(&b,wfd,0L,512,0)) >= 0) {
	    WORDFILL	w ;

	    if ((rs = wordfill_start(&w,cbuf,clen)) >= 0) {
		int		cols = sip->linelen ;
	        const int	llen = LINEBUFLEN ;
	        int		ll ;
		int		lw ;
	        int		ln = 0 ;
	        char		lbuf[LINEBUFLEN + 1] ;

		lw = MIN((cols - (prec+1)),llen) ;
	        while ((ll = wordfill_mklinefull(&w,lbuf,lw)) > 0) {

	            rs = subinfo_procoutline(sip,&b,prec,ln,n,lbuf,ll) ;
	            wlen += rs ;

	            ln += 1 ;
	            if (rs < 0) break ;
	        } /* end while (full lines) */

	        if (rs >= 0) {
	            if ((ll = wordfill_mklinepart(&w,lbuf,lw)) > 0) {

	                rs = subinfo_procoutline(sip,&b,prec,ln,n,lbuf,ll) ;
	                wlen += rs ;

	            }
	        } /* end if (partial lines) */

	        wordfill_finish(&w) ;
	    } /* end if (wordfill) */

	    filebuf_finish(&b) ;
	} /* end if (filebuf) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_procout) */


static int subinfo_procoutline(SUBINFO *sip,FILEBUF *fbp,int prec,int ln,int n,
		cchar *lp,int ll)
{
	int		rs ;
	int		wlen = 0 ;

	if (sip == NULL) return SR_FAULT ;

	if (ln == 0) {
	    rs = filebuf_printf(fbp,"%*u %t\n", prec,n, lp,ll) ;
	    wlen += rs ;
	} else {
	    rs = filebuf_printf(fbp,"%t %t\n", blanks,prec, lp,ll) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_procoutline) */


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


