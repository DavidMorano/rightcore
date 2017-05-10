/* main */

/* front-end for the TESTMAILBOX program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	1		/* run-time debug print-outs */
#define	CF_CLEN		1		/* use 'content-length' */
#define	CF_MSGINFO	1		/* call '_msginfo()' */
#define	CF_MBRW		1		/* open mailbox read-write */
#define	CF_TESTDEL	0		/* test msg-delete */
#define	CF_TESTHDRADD	1		/* test msg-hdr-add */
#define	CF_MBCACHE	1		/* test MBCACHE object */


/* revision history:

	= 2008-01-16, David A­D­ Morano

	Of course this was taken from previous programs.  I deleted
	the list that was here and replaced it with this simple
	admission. :-)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testmailbox.x <mailbox>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/timeb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<dater.h>
#include	<realname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"mailbox.h"
#include	"mbcache.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	initnow(struct timeb *,char *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;
static int	procmbcache(PROGINFO *,bfile *,
			const char *,int,MAILBOX *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;

	MAILBOX		mb ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	time_t	daytime = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	rs ;
	int	rs1 ;
	int	len ;
	int	sl ;
	int	v ;
	int	mbopts = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f_usage = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*mbfname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

/* get the current time-of-day */

	rs = initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinitnow ;
	}

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
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

	        if (isdigit(argp[1])) {

		    argval = (argp + 1) ;

	        } else if (argp[1] == '-') {

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

/* do we have a keyword match or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        pr = argp ;
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
				}
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* output filename */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
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

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                    rs = optvalue(avp,avl) ;
	                                    pip->debuglevel = rs ;
				    }
	                        }
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* output file name */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ofname = argp ;
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
				    }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or options) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* get the first positional argument as the mailbox name */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    mbfname = argv[ai] ;
	    BACLR(argpresent,ai) ;
	    break ;

	} /* end for */

/* open the output file */

	if (ofname != NULL) {
	    rs = bopen(ofp,ofname,"wct",0666) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: done w/ opening output \n") ;
#endif

/* open the mailbox */

	mbopts = 0 ;

#if	CF_CLEN
#else
	mbopts |= MAILBOX_ONOCLEN ;
#endif

#if	CF_MBRW
	mbopts |= MAILBOX_ORDWR ;
#else
	mbopts |= MAILBOX_ORDONLY ;
#endif

	rs = mailbox_open(&mb,mbfname,mbopts) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: mailbox_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    MAILBOX_INFO	mbinfo ;
	    MAILBOX_MSGINFO	msginfo, *mp = &msginfo ;

	    int	mi, n ;


	    rs = mailbox_info(&mb,&mbinfo) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: mailbox_info() rs=%d\n",rs) ;
#endif

	    bprintf(ofp,"number msgs=%u\n",mbinfo.nmsgs) ;

#if	CF_MSGINFO
	    for (mi = 0 ; (rs >= 0) && (mi < mbinfo.nmsgs) ; mi += 1) {

	        rs = mailbox_msginfo(&mb,mi,&msginfo) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main: msg=%u mailbox_msginfo() rs=%d\n",
			mi,rs) ;
#endif /* CF_DEBUGS */

	        if ((rs >= 0) && (mp != NULL)) {

	            bprintf(ofp,"msg=%u\n",mi) ;

	            bprintf(ofp,"  moff=%u mlen=%u\n",
			mp->moff,mp->mlen) ;

	            bprintf(ofp,"  hoff=%u hlen=%u\n",
			mp->hoff,mp->hlen) ;

	            bprintf(ofp,"  boff=%u blen=%u\n",
			mp->boff,mp->blen) ;

	            bprintf(ofp,"  clen=%d \n",
	                mp->clen) ;

	            bprintf(ofp,"  clines=%d\n",
	                mp->clines) ;

	        } /* end if */

#if	CF_TESTDEL
		n = 969 ;
		    if (mi == n) {
			rs = mailbox_msgdel(&mb,mi,TRUE) ;
			if (rs >= 0)
	            bprintf(ofp,"  deleted=%u\n",mi) ;
			}
#endif /* CF_TESTDEL */

#if	CF_TESTHDRADD
		n = 2 ;
		    if (mi == n) {
			rs = mailbox_msghdradd(&mb,mi,"x-here","there",-1) ;
			if (rs >= 0)
	            bprintf(ofp,"  hdradd=%u\n",mi) ;
			}
#endif /* CF_TESTHDRADD */

		if (rs < 0)
			break ;

	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: final rs=%d\n",rs) ;
#endif

#endif /* CF_MSGINFO */

#if	CF_MBCACHE
	    if (rs >= 0)
		rs = procmbcache(pip,ofp,mbfname,mbopts,&mb) ;
#endif

	    mailbox_close(&mb) ;
	} /* end if (mailbox) */

/* close the mailbox */

	bclose(ofp) ;

done:
badoutopen:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* we are done */
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

badinitnow:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [mailbox] \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,"%s: <mailbox> [-?V] [-Dv]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procmbcache(pip,ofp,mbfname,mbopts,mbp)
PROGINFO	*pip ;
bfile		*ofp ;
const char	mbfname[] ;
int		mbopts ;
MAILBOX		*mbp ;
{
	MBCACHE		mc ;
	MBCACHE_INFO	minfo ;
	MBCACHE_SCAN	*msp ;

	int	rs = SR_OK ;
	int	nmsgs ;
	int	mi ;
	int	j ;

	const char	*sp ;
	const char	*fp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmbcache: mbfname=%s\n",mbfname) ;
#endif /* CF_DEBUGS */

	rs = mbcache_start(&mc,mbfname,mbopts,mbp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmbcache: mbcache_init() rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	if (rs < 0)
	    goto ret0 ;

	rs = mbcache_mbinfo(&mc,&minfo) ;
	nmsgs = minfo.nmsgs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("main/procmbcache: mbcache_mbinfo() rs=%d\n",rs) ;
	debugprintf("main/procmbcache: nmsgs=%u\n",nmsgs) ;
	}
#endif /* CF_DEBUGS */

	if (rs < 0)
	    goto ret1 ;

	for (mi = 0 ; mi < nmsgs ; mi += 1) {
		rs = mbcache_msgscan(&mc,mi,&msp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmbcache: mi=%u mbcache_msgscan() rs=%d\n",
		mi,rs) ;
#endif /* CF_DEBUGS */

		if (rs < 0)
			break ;
		for (j = 0 ; j < mbcachemf_overlast ; j += 1) {
		switch (j) {
		case mbcachemf_scanfrom:
		    sp = "from" ;
		    break ;
		case mbcachemf_scansubject:
		    sp = "subject" ;
		    break ;
		case mbcachemf_scandate:
		    sp = "date" ;
		    break ;
		case mbcachemf_scanline:
		    sp = "scanline" ;
		    break ;
		case mbcachemf_hdrstatus:
		    sp = "status" ;
		    break ;
		default:
		    sp = "unknown" ;
		    break ;
		} /* end switch */
		fp = msp->vs[j] ;
		if (fp == NULL)
			fp = "-" ;
		rs = bprintf(ofp,"MSG %s=>%t<\n",
			sp,fp,strnlen(fp,40)) ;
		} /* end for (scan fields) */
	} /* end for (messages) */

ret1:
	mbcache_finish(&mc) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmbcache: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutines (procmbcache) */



