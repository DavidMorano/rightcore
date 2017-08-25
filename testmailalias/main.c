/* main */

/* program to test the MAILALIAS object */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_USEPCS	0		/* use PCS alias-profile table */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This was originally written.

	= 1999-03-01, David A­D­ Morano
        I enhanced the program to also print out effective UID and effective
        GID.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testmailalias.x


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<realname.h>
#include	<mallocstuff.h>
#include	"svcfile.h"
#include	"kvsfile.h"
#include	"mailalias.h"
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifdef	ALIASNAMELEN
#define	ABUFLEN		ALIASNAMELEN
#else
#define	ABUFLEN		64
#endif

#ifdef	MAILADDRLEN
#define	VBUFLEN		MAILADDRLEN
#else
#define	VBUFLEN		2048
#endif

#ifndef	KEYBUFLEN
#ifdef	KVSFILE_KEYLEN
#define	KEYBUFLEN	KVSFILE_KEYLEN
#else
#define	KEYBUFLEN	MAXHOSTNAMELEN
#endif
#endif

#define	VALBUFLEN	MAXPATHLEN
#define	EBUFLEN		(2 * MAXPATHLEN)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;


/* local variables */

static const char *argopts[] = {
	    "ROOT",
	    "VERSION",
	    "VERBOSE",
	    "HELP",
	"af",
	    "of",
	    "pwfile",
	    "pwifile",
	    "ipwfile",
	    NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_af,
	argopt_of,
	argopt_pwfile,
	argopt_pwifile,
	argopt_ipwfile,
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
	{ 0, 0 }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sb ;

	struct proginfo	pi, *pip = &pi ;

	KVSFILE		aptab ;		/* alias-profile table */

	SVCFILE		st ;

	MAILALIAS	madb ;		/* selected MAILALIAS */

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	maxai ;
	int	rs, rs1, len, c ;
	int	i, j, k ;
	int	sl, cl, ci ;
	int	oflags ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*mafname = NULL ;
	const char	*sp, *cp, *cp2 ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	rs = SR_OK ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* keyword match or only key letters? */

	                if ((kwi = matpstr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

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

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                            }
	                        }

	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pr = argp ;

	                        }

	                        break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* argument list file */
	                    case argopt_af:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                afname = avp ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                afname = argp ;

	                        }

	                        break ;

/* output file name */
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

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                            "%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

	                                }
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* profile name */
	                        case 'p':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                mafname = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
	                                        &pip->verboselevel) ;

	                                }
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            rs = SR_INVALID ;
	                            bprintf(pip->efp,
	                                "%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                    aop += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or progopts) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* some initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* get the first positional argument as the username to key on */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    mafname = argv[ai] ;
	    BACLR(argpresent,ai) ;
	    break ;

	} /* end for */

	if ((mafname == NULL) || (mafname[0] == '-'))
	    mafname = "default" ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: mafname=%s\n",mafname) ;
#endif

/* open the output file */

	if (ofname != NULL)
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto retearly ;
	}

/* enumerate the domain table (a SVCFILE) */

	cp = "domain" ;

	bprintf(ofp,"domain table file=%s\n",cp) ;

	rs = svcfile_open(&st,cp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: domain=%s\n",cp) ;
	    debugprintf("main: svcfile_open() rs=%d\n",rs) ;
	}
#endif

	if (rs >= 0) {

	    SVCFILE_CUR	stcur ;

	    SVCFILE_ENT	ste ;

	    char	ebuf[EBUFLEN + 1] ;


/* do enumeration */

	    bprintf(ofp,"domain enumberation\n") ;

	    svcfile_curbegin(&st,&stcur) ;

	    while (TRUE) {

	        rs1 = svcfile_enum(&st,&stcur,&ste,ebuf,EBUFLEN) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main: svcfile_enum() rs=%d\n",rs1) ;
	            debugprintf("main: ste.nkeys=%u\n",ste.nkeys) ;
	            debugprintf("main: ste.size=%u\n",ste.size) ;
	        }
#endif

	        if (rs1 < 0)
	            break ;

	        bprintf(ofp,"nkeys=%u svc=%s\n",
	            ste.nkeys,ste.svc) ;

	        for (i = 0 ; ste.keyvals[i][0] != NULL ; i += 1) {

	            bprintf(ofp,"  k=%s v=>%s<\n",
	                ste.keyvals[i][0],ste.keyvals[i][1]) ;

	        }

	    } /* end while */

	    svcfile_curend(&st,&stcur) ;

/* do fetching */

	    bprintf(ofp,"domain fetching\n") ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;

	        bprintf(ofp,"domain query svc=%s\n",cp) ;

	        svcfile_curbegin(&st,&stcur) ;

	        while (TRUE) {

	            rs1 = svcfile_fetch(&st,cp,&stcur,
	                &ste,ebuf,EBUFLEN) ;

	            if (rs1 < 0)
	                break ;

	            for (i = 0 ; ste.keyvals[i][0] != NULL ; i += 1) {

	                bprintf(ofp,"  k=%s v=>%s<\n",
	                    ste.keyvals[i][0],ste.keyvals[i][1]) ;

	            }

	        } /* end while */

	        svcfile_curend(&st,&stcur) ;

	    } /* end for */

	    svcfile_close(&st) ;

	} /* end block */

/* find and print out the contents of the given (mail) alias-profile */

	bprintf(ofp,"mailalias profiles\n") ;

#if	CF_USEPCS
	cp = "/usr/add-on/pcs" ;
#else
	cp = pip->pr ;
#endif

	mkpath2(tmpfname,cp,"etc/mail/aptab") ;

	rs = kvsfile_open(&aptab,20,tmpfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: aptab=%s\n",tmpfname) ;
	    debugprintf("main: kvsfile_open() rs=%d\n",rs) ;
	}
#endif

	if (rs >= 0) {
	    kvsfile_cur	apcur ;

	    char	keybuf[KEYBUFLEN + 1] ;
	    char	valbuf[VALBUFLEN + 1] ;


/* do enumeration */

	bprintf(ofp,"aptab enumeration\n") ;

	    kvsfile_curbegin(&aptab,&apcur) ;

	    while (TRUE) {

	        rs1 = kvsfile_enum(&aptab,&apcur,keybuf,KEYBUFLEN,
	            valbuf,VALBUFLEN) ;

	        if (rs1 < 0)
	            break ;

	        bprintf(ofp,"  k=%s v=%s\n",
	            keybuf,valbuf) ;

	    } /* end while */

	    kvsfile_curend(&aptab,&apcur) ;

/* do fetching */

	bprintf(ofp,"aptab fetching\n") ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	        bprintf(ofp,"aptab key=%s\n",cp) ;

	        kvsfile_curbegin(&aptab,&apcur) ;

	        while (TRUE) {

	            rs1 = kvsfile_fetch(&aptab,cp,&apcur,
	                valbuf,VALBUFLEN) ;

	            if (rs1 < 0)
	                break ;

	            bprintf(ofp,"  val=%s\n", valbuf) ;

	        } /* end while */

	        kvsfile_curend(&aptab,&apcur) ;

	    } /* end for */

	    kvsfile_close(&aptab) ;

	} /* end block */

	bprintf(ofp,"mailalias profile=%s\n",mafname) ;

/* try to open the MAILALIAS database */

	oflags = (O_RDWR | O_CREAT) ;
	rs = mailalias_open(&madb,pip->pr,mafname,oflags,0666,0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: mailalias_open() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
		ex = EX_NOINPUT ;
	    goto done ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {

	    char	abuf[ABUFLEN + 1] ;
	    char	vbuf[VBUFLEN + 1] ;

	    MAILALIAS_CUR	cur ;

	    mailalias_curbegin(&madb,&cur) ;

	    while (TRUE) {

	        rs1 = mailalias_enum(&madb,&cur,abuf,ABUFLEN,vbuf,VBUFLEN) ;

	        debugprintf("main: mailalias_enum() rs=%d\n",rs1) ;

	        if (rs1 < 0)
	            break ;

	        debugprintf("main: a=%s v=%s\n",abuf,vbuf) ;

	    } /* end while */

	    mailalias_curend(&madb,&cur) ;

/* loop through the index */

	    {
	        uint	ri, hi, hilen ;


	        hilen = nextpowtwo(madb.rilen) ;

	        for (hi = 0 ; hi < hilen ; hi += 1) {

	            if ((ri = madb.indtab[hi][0]) != 0) {

	                debugprintf("main: hi=%u ri=%u key=%s v=%s next=%u\n",
	                    hi,ri,
	                    (madb.skey + madb.rectab[ri][0]),
	                    (madb.sval + madb.rectab[ri][1]),
	                    madb.indtab[hi][1]) ;

	            }

	        } /* end for */

	    } /* end block */

	} /* end if */
#endif /* CF_DEBUG */

/* perform the default actions */

	bprintf(ofp,"mailalias lookups\n") ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    char	abuf[ABUFLEN + 1] ;
	    char	vbuf[VBUFLEN + 1] ;

	    MAILALIAS_CUR	cur ;


	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: lookup alias=%s\n",cp) ;
#endif

	    mailalias_curbegin(&madb,&cur) ;

	    while (TRUE) {

	        rs1 = mailalias_fetch(&madb,0,cp,&cur,vbuf,VBUFLEN) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: mailalias_fetch() rs=%d\n",rs1) ;
#endif

	        if (rs1 < 0)
	            break ;

	        bprintf(ofp,"  k=%s v=%s\n",cp,vbuf) ;

	    } /* end while */

	    mailalias_curend(&madb,&cur) ;

	} /* end for (handling positional arguments) */

	mailalias_close(&madb) ;

/* we are done */
done:
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	bclose(ofp) ;

/* early return thing */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the information type thing */
usage:
	usage(pip) ;

	goto retearly ;

/* print out some help */
help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid arguments specified (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open the output file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* error types of returns */
badret:
	bclose(pip->efp) ;

	goto retearly ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s aliasname(s) \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	bprintf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



