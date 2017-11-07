/* opensvc_logwelcome */

/* ADMIN-user open-service (logwelcome) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_PRNUSER	1		/* use 'prn' as user */
#define	CF_DISUID	0		/* print admin UID */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */
#define	CF_ORGLOC	0		/* include Organization-Location */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an user open-service module.  This little diddy supplies a fast
	little "issue" message for programs that perform logins onto the host.

	Synopsis:

	int opensvc_logwelcome(pr,prn,of,om,argv,envv,to)
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


	Notes: We generally prefer using 'sntmtime(3dam)' over the POSIX®
	'strftime(3c)' since it is somewhat cleaner and easier to use.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>		/* for 'strftime(3c)' */

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<sbuf.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"opensvc_logwelcome.h"
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

#define	VARKEYNAME	"ISSUE_KEYNAME"

#ifndef	ORGLOCLEN
#define	ORGLOCLEN	32
#endif

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	DEFORGUSER	"admin"

#define	ORGTRIM		44 /* calculated separately */
#define	KEYTRIM		8 /* maximum keyname length printed */

#define	NDEBFNAME	"opensvc_logwelcome.deb"

#define	DATETYPE	0		/* time-zone: 0=GMT, 1=<local> */


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	sntmtime(char *,int,TMTIME *,cchar *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matkeystr(cchar **,cchar *,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	getnodename(char *,int) ;
extern int	getdomainname(char *,int,cchar *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	getuserorg(char *,int,cchar *) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	localgetorg(cchar *,char *,int,cchar *) ;
extern int	localgetorgloc(cchar *,char *,int,cchar *) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	nprintf(cchar *,cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strdcpy2(char *,int,cchar *,cchar *) ;
extern char	*strdcpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;


/* local structures */


/* forward references */

static int mklinestr(char *,int,cchar *,cchar *,cchar *,cchar *) ;


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"sn",
	"long",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_long,
	argopt_overlast
} ;

static cchar	*days[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	NULL
} ;


/* exported subroutines */


int opensvc_logwelcome(pr,prn,of,om,argv,envv,to)
cchar		*pr ;
cchar		*prn ;
int		of ;
mode_t		om ;
cchar		**argv ;
cchar		**envv ;
int		to ;
{
	BITS		pargs ;
	KEYOPT		akopts ;
	uid_t		uid = 0 ;
	const int	ulen = USERNAMELEN ;
	const int	olen = ORGLEN ;
	const int	ollen = ORGLOCLEN ;
	const int	llen = LINEBUFLEN ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		ol = -1 ;
	int		ll = -1 ;
	int		cols = COLUMNS ;
	int		dtype = DATETYPE ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f_long = TRUE ;
	int		f ;
	cchar		*un = NULL ;
	cchar		*keyname = NULL ;
	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*zonespec = NULL ;
	cchar		*cp ;
	char		prlocal[MAXPATHLEN+1] ;
	char		ubuf[USERNAMELEN+1] ;
	char		obuf[ORGLEN+1] ;
	char		dbuf[TIMEBUFLEN+1] ;
	char		olbuf[ORGLOCLEN+1] = { 0 } ;
	char		lbuf[LINEBUFLEN+2] ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

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

	                case argopt_long:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					f_long = (rs > 0) ;
				    }
	                        }
	                        break ;

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

			    case 'l':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					f_long = (rs > 0) ;
				    }
	                        }
	                        break ;

	                    case 'o':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    KEYOPT	*kop = &akopts ;
	                            rs = keyopt_loads(kop,argp,argl) ;
				}
			    } else
	                        rs = SR_INVALID ;
	                        break ;

/* use local (or specified) time-zone */
	                    case 'z':
	                        dtype = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                zonespec = avp ;
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

/* initialization */

	lbuf[0] = '\0' ;

/* find the number of allowable columns (for line-length limiting) */

	if ((cp = getourenv(envv,VARCOLUMNS)) != NULL) {
	    int	v ;
	    rs1 = cfdeci(cp,-1,&v) ;
	    if (rs1 >= 0) cols = v ;
	}

/* find the username to act upon */

	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        keyname = argv[ai] ;
	        break ;
	    }
	} /* end for */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logwelcome: 1 keyname=%s\n",keyname) ;
#endif

/* optional keyname */

	if ((keyname == NULL) || (keyname[0] == '\0')) {
	    keyname = getourenv(envv,VARKEYNAME) ;
	}

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logwelcome: f keyname=%s\n",keyname) ;
#endif

/* default user as necessary (for use in getting the organization) */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_logwelcome: 0 un=%s\n",un) ;
#endif

#if	CF_PRNUSER /* more reliable in general */

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = prn ;

#else /* CF_PRNUSER */

/* this is a real cheapy way to try to get the sponsoring user-name */

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) {
	    int		pl, bl ;
	    cchar	*bp ;
	    pl = strlen(pr) ;
	    while (pl && (pr[pl-1] == '/')) pl -= 1 ;
	    if ((bl = sfbasename(pr,-1,&bp)) > 0) {
		rs1 = sncpy1w(ubuf,ulen,bp,bl) ;
		if (rs1 >= 0) un = ubuf ;
	    }
	}

#endif /* CF_PRNUSER */

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = DEFORGUSER ;

/* get the LOCAL program-root (needed later for 'localgetorg()' */

	prlocal[0] = '\0' ;
	if (rs >= 0) {
	    char	dn[MAXHOSTNAMELEN+1] ;
	    char	nn[NODENAMELEN+1] ;
	    if ((rs = getnodedomain(nn,dn)) >= 0) {
		rs = mkpr(prlocal,MAXPATHLEN,VARPRLOCAL,dn) ;
	    }
	}

/* get and format the current date */

	dbuf[0] = '\0' ;
	if (rs >= 0) {
	    const time_t	dt = time(NULL) ;
	    TMTIME		vals ;
	    if ((rs = tmtime_ztime(&vals,dtype,dt)) >= 0) {
	        int	dl = TIMEBUFLEN ;
		cchar	*fmt ;
	        char	*dp ;
		if (f_long) {
		    fmt = " %e %b %R %Z" ;
		    dp = strdcpy2(dbuf,9,days[vals.wday],"   ") ;
		    dl -= (dp-dbuf) ;
		} else {
		    fmt = "%a %e %b %R %Z" ;
		    dp = dbuf ;
		}
		rs = sntmtime(dp,dl,&vals,fmt) ;
	    } /* end if (tmtime_ztime) */
	} /* end if (ok) */

/* get our (machine) organization */

	obuf[0] = '\0' ;
	if ((rs >= 0) && (prlocal[0] != '\0')) {
	    rs = getuserorg(obuf,olen,un) ;
	    ol = rs ;
	    if (((rs >= 0) && (ol == 0)) || ((rs < 0) && isNotPresent(rs))) {
	        rs = localgetorg(prlocal,obuf,olen,un) ;
	        ol = rs ;
		if (ol > ORGTRIM) {
		    ol = ORGTRIM ;
		    obuf[ol] = '\0' ;
		}
	    }
	}

/* try to get our organization location */

#if	CF_ORGLOC
	if ((rs >= 0) && (prlocal[0] != '\0')) {
	    int		oll ;
	    rs = localgetorgloc(prlocal,olbuf,ollen,un) ;
	    oll = rs ;
#if	CF_DEBUGN
	    {
		cchar	*fn = NDEBFNAME ;
	        nprintf(fn,"opensvc_logwelcome: localgetorgloc() rs=%d ol=%t\n",
		    rs,olbuf,strlinelen(olbuf,oll,40)) ;
	    }
#endif
	    if (isNotAccess(rs)) { /* we don't care too much for these errors */
		olbuf[0] = '\0' ;
		if (isNotPresent(rs)) rs = SR_OK ;
		else if (rs == SR_OVERFLOW) rs = SR_OK ;
	    }
	    if ((rs >= 0) && (olbuf[0] != '\0')) {
		if (ol < 0) ol = strlen(obuf) ;
		if ((ol + oll + 3) > ORGTRIM) {
		    oll = 0 ;
		    olbuf[0] = '\0' ;
		}
	    }
	} /* end if (organization-location) */
#else /* CF_ORGLOC */
	olbuf[0] = '\0' ;
#endif /* CF_ORGLOC */

/* finally get the sponsoring user UID */

#if	CF_DISUID
	if ((rs >= 0) && (un != NULL)) {
	    if ((rs = getuid_user(un)) >= 0) {
	    	    uid = rs ;
	    } else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	    }
	}
#endif /* CF_DISUID */

/* put it all together */

#if	CF_DISUID
	if (rs >= 0) {
	    cchar	*fmt ;
	    if (olbuf[0] != '\0') {
	        fmt = "%s - %s (%s) ­%u­" ;
	    } else {
	        fmt = "%s - %s ­%u­" ;
	    }
	    rs = bufprintf(lbuf,llen,fmt,dbuf,obuf,olbuf,uid) ;
	    ll = rs ;
	}
#else /* CF_DISUID */
	if (rs >= 0) {
	    rs = mklinestr(lbuf,llen,dbuf,obuf,olbuf,keyname) ;
	    ll = rs ;
	}
#endif /* CF_DISUID */

	if (rs >= 0) {
	    if (ll < 0) ll = strlen(lbuf) ;
	    if (ll > cols) ll = cols ;
	    if (ll > 0) {
	        lbuf[ll++] = '\n' ;
	        lbuf[ll] = '\0' ;
	    }
	}

/* write it out */

	if (rs >= 0) {
	    if ((rs = u_pipe(pipes)) >= 0) {
	        const int	wfd = pipes[1] ; /* write end */
		{
	            fd = pipes[0] ; /* read end */
	            rs = u_write(wfd,lbuf,ll) ;
		}
	        u_close(wfd) ;
	        if (rs < 0) u_close(fd) ;
	    } /* end if (pipe) */
	} /* end if (ok) */

badarg:
	if (f_akopts) {
	    f_akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_logwelcome) */


/* local subroutines */


static int mklinestr(lbuf,llen,dbuf,obuf,olbuf,keyname)
char		lbuf[] ;
int		llen ;
cchar		*dbuf ;
cchar		*obuf ;
cchar		*olbuf ;
cchar		*keyname ;
{
	SBUF		b, *bp = &b ;
	int		rs ;
	int		ll = 0 ;

	if ((rs = sbuf_start(bp,lbuf,llen)) >= 0) {

	    sbuf_strw(bp,dbuf,-1) ;
	    sbuf_strw(bp," - ",3) ;
	    sbuf_strw(bp,obuf,-1) ;
	    if ((olbuf != NULL) && (olbuf[0] != '\0')) {
	        sbuf_strw(bp," (",2) ;
	        sbuf_strw(bp,olbuf,-1) ;
	        sbuf_strw(bp,")",1) ;
	    }
	    if ((keyname != NULL) && (keyname[0] != '\0')) {
	        sbuf_strw(bp," ­",2) ;
	        sbuf_strw(bp,keyname,KEYTRIM) ;
	        sbuf_strw(bp,"­",1) ;
	    }

	    ll = sbuf_finish(bp) ;
	    if (rs >= 0) rs = ll ;
	} /* end if (sbuf) */

	return (rs >= 0) ? ll : rs ;
}
/* end subroutine (mklinestr) */


