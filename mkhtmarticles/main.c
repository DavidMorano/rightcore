/* main */

/* program to create HTM files from articles */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_FOOTER	1		/* use page footers */


/* revision history:

	= 1999-02-09, David A­D­ Morano

	This subroutine is being hacked again to make the MKHTMARTICLES
	program.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Symipsis:

	$ ls *.txt | mkhtmarticles > index.htm


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<tzfile.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<realname.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"artinfo.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	bprintlines(bfile *,int,const char *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	progfile(struct proginfo *,struct artinfo *,const char *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;

static int	sfdigits(const char *,int,const char **) ;
static int	cmpfwd(const char **,const char **) ;
static int	cmprev(const char **,const char **) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"ROOT",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_root,
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

static const char	*months[] = {
	"jan", "feb", "mar", "apr", "may", "jun", 
	"jul", "aug", "sep", "oct", "nov", "dec", 
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct ustat	sb ;

	VECSTR	names ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	tmpfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	len, i, j, k ;
	int	sl, cl, ci ;
	int	c_files = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_reverse = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*sp, *cp ;

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

	rs = 0 ;
	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

		    rs = cfdecti((argp + 1),(argl - 1),&argvalue) ;

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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

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
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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

/* argument file name */
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
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* reverse sort mode */
	                    case 'r':
	                        f_reverse = TRUE ;
	                        break ;

	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            strwcpy(pip->site_title,
	                                argp,MIN(argl,ITEMLEN)) ;

	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

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

	        } /* end if (digits as argument or not) */

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
	    usage(pip) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;
	ex = EX_OK ;

	if (pip->site_title[0] == '\0')
	    strwcpy(pip->site_title,DEFTITLE,ITEMLEN) ;

	pip->daytime = time(NULL) ;

	{
	    struct tm	cts ;
	    uc_localtime(&pip->daytime,&cts) ;
	    pip->cyear = cts.tm_year ;
	}

/* initialze a container to hold filenames */

	rs = vecstr_start(&names,30,VECSTR_PSORTED) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinitnames ;
	}

/* grab the names that are in the invocation arguments */

	for (ai = 0 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = vecstr_add(&names,cp,-1) ;

	    c_files += 1 ;
	    if (rs < 0) break ;
	} /* end for */

/* grab the names that are in the argument file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	ifile ;
	    char	lbuf[LINEBUFLEN + 1] ;


	    if (strcmp(afname,"-") != 0) {
	        rs = bopen(&ifile,afname,"r",0666) ;
	    } else
	        rs = bopen(&ifile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        while ((rs = breadline(&ifile,lbuf,LINEBUFLEN)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
		    lbuf[len] = '\0' ;

	            cl = sfshrink(lbuf,len,&cp) ;

	            if ((cl <= 0) || (cp[0] == '#')) continue ;

	            pan += 1 ;
	            rs = vecstr_add(&names,cp,cl) ;

	            c_files += 1 ;
	            if (rs < 0) break ;
	        } /* end while */

	        bclose(&ifile) ;
	    } /* end if */

	} /* end if (argument file) */

/* read from standard input */

	if ((rs >= 0) && (c_files <= 0)) {
	    bfile	ifile ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    if ((rs = bopen(&ifile,BFILE_STDIN,"dr",0666)) >= 0) {

	        while ((rs = breadline(&ifile,lbuf,LINEBUFLEN)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
		    lbuf[len] = '\0' ;

	            if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                 if (cp[0] != '#') {
	            	    pan += 1 ;
	            	    rs = vecstr_add(&names,cp,cl) ;
	            	    c_files += 1 ;
			}
		    }

	            if (rs < 0) break ;
	        } /* end while */

	        bclose(&ifile) ;
	    } /* end if */

	} /* end if (standard input) */

/* sort all names */

	if (rs >= 0) {
	    int		(*func)(const char **,const char **) ;

	    func = (f_reverse) ? cmprev : cmpfwd ;
	    vecstr_sort(&names,func) ;

	} /* end block */

/* open the output file */

	if (ofname != NULL) {
	    rs = bopen(ofp,ofname,"wct",0666) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open the output file (%d)\n",
	        pip->progname,rs) ;
	    goto badopenout ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: done w/ opening output \n") ;
#endif

	bprintf(ofp,"<html>\n") ;

	bprintf(ofp,"<head>\n") ;

	bprintf(ofp,"<title>\n") ;

	bprintf(ofp,"Bush-Is-Bad\n") ;

	bprintf(ofp,"</title>\n") ;

	bprintf(ofp,"</head>\n") ;

	bprintf(ofp,"<body>\n") ;

/* copy over the "lead" piece */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: LEADFNAME bopen() fname=%s\n",LEADFNAME) ;
#endif

	rs = bopen(&tmpfile,LEADFNAME,"r",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: LEADFNAME bopen() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    rs1 = bcopyblock(&tmpfile,ofp,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: LEADFNAME bcopyblock() rs=%d\n",rs) ;
#endif

	    bclose(&tmpfile) ;

	} /* end if (leader file) */

/* create the article links */

	bprintf(ofp,"<ul>\n") ;

	for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1) {
	    struct artinfo	fi ;
	    if (cp == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: progfile() fname=%s\n",cp) ;
#endif

	    rs = progfile(pip,&fi,cp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
		char	timebuf[TIMEBUFLEN + 1] ;
	        debugprintf("main: progfile() rs=%d\n",rs) ;
	        if (rs >= 0) {
	            debugprintf("main: title=%s\n",fi.title) ;
	            debugprintf("main: author=%s\n",fi.author) ;
	            debugprintf("main: date=%s\n",
			timestr_log(fi.date,timebuf)) ;
	            debugprintf("main: publisher=%s\n",fi.publisher) ;
	        }
	    }
#endif /* CF_DEBUG */

	    if (rs >= 0) {

	        bprintf(ofp,"<li>\n") ;

	        bprintf(ofp,"<a href=\"%s\">\n",fi.htmfname) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: loop title=>%s<\n",fi.title) ;
#endif

	        rs1 = bprintlines(ofp,LINEFOLDLEN,fi.title,-1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: bprintlines() rs=%d\n",rs1) ;
#endif

	        bprintf(ofp,"</a>\n") ;

	        bprintf(ofp,"<br>\n") ;

		if (fi.author[0] != '\0') {
	            if (! fi.f.noauthor) {
	                bprintf(ofp,"by %s\n",fi.author) ;
	            } else
	                bprintf(ofp,"%s\n",fi.author) ;
		}

	        if (fi.date != 0) {
	            struct tm	ts ;

	            rs1 = uc_localtime(&fi.date,&ts) ;

	            if (rs1 >= 0) {

	                bprintf(ofp,", %s %u, %u\n",
	                    months[ts.tm_mon],
	                    ts.tm_mday,
	                    (TM_YEAR_BASE + ts.tm_year)) ;

	            }
	        }

	        bprintf(ofp,"</li>\n") ;

	    } /* end if (processed one) */

	} /* end for */

	bprintf(ofp,"</ul>\n") ;

/* copy over the "follow" piece */

	if ((rs = bopen(&tmpfile,TRAILFNAME,"r",0666)) >= 0) {

	    rs = bcopyblock(&tmpfile,ofp,-1) ;

	    bclose(&tmpfile) ;
	} /* end if (leader file) */

/* optional footer */

#if	CF_FOOTER
	if (pip->site_title[0] != '\0') {
	    bprintf(ofp,"<hr>\n") ;
	    bprintf(ofp,"<center>\n") ;
	    bprintf(ofp,"_ <a href=\"../index.htm\">home</a> _ \n") ;
	    bprintf(ofp,"</center>\n") ;
	    bprintf(ofp,"<br>\n") ;
	    bprintf(ofp,"<center>\n") ;
	    bprintf(ofp,"%s\n",pip->site_title) ;
	    bprintf(ofp,"</center>\n") ;
	}
#endif /* CF_FOOTER */

	bprintf(ofp,"</body>\n") ;

	bprintf(ofp,"</html>\n") ;

/* we are done */
done:
	bclose(ofp) ;

badopenout:
ret4:
	vecstr_finish(&names) ;

/* early return thing */
badinitnames:
retearly:
ret3:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret2:
	bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:
badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
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
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s [<filename(s)> ...] [-af <afile>] [-r]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int cmpfwd(e1p,e2p)
const char	**e1p, **e2p ;
{
	int	c ;
	int	cmplen ;
	int	c1l, c2l ;
	int	f_low1, f_low2 ;

	const char	*c1p, *c2p ;


#if	CF_DEBUGS
	debugprintf("main/sortfunc: e1=>%s<\n",*e1p) ;
	debugprintf("main/sortfunc: e2=>%s<\n",*e2p) ;
#endif

	if ((*e1p == NULL) && (*e2p == NULL))
	    return 0 ;

	if (*e1p == NULL)
	    return 1 ;

	if (*e2p == NULL)
	    return -1 ;

/* find the date-strings in both entries */

	c1l = sfdigits(*e1p,-1,&c1p) ;

	c2l = sfdigits(*e2p,-1,&c2p) ;

	if ((c1l == 0) && (c2l == 0))
	    return 0 ;

	if ((c1l > 0) && (c2l == 0))
	    return 1 ;

	if ((c1l == 0) && (c2l > 0))
	    return -1 ;

	f_low1 = (*c1p < '7') ? 1 : 0 ;
	f_low2 = (*c2p < '7') ? 1 : 0 ;
	if (! LEQUIV(f_low1,f_low2)) {

	    c = (f_low1 - f_low2) ;

#if	CF_DEBUGS
	    debugprintf("main/sortfunc: c=%d\n",c) ;
#endif

	    return c ;
	}

/* compare the strings */

	cmplen = MIN(c1l,c2l) ;

	c = strncmp(c1p,c2p,cmplen) ;

	if (c == 0)
	    c = c1l - c2l ;

	return c ;
}
/* end subroutine (cmpfwd) */


static int cmprev(e1p,e2p)
const char	**e1p, **e2p ;
{
	int	c ;
	int	cmplen ;
	int	c1l, c2l ;
	int	f_low1, f_low2 ;

	const char	*c1p, *c2p ;


#if	CF_DEBUGS
	debugprintf("main/sortfunc: e1=>%s<\n",*e1p) ;
	debugprintf("main/sortfunc: e2=>%s<\n",*e2p) ;
#endif

	if ((*e1p == NULL) && (*e2p == NULL))
	    return 0 ;

	if (*e1p == NULL)
	    return -1 ;

	if (*e2p == NULL)
	    return 1 ;

/* find the date-strings in both entries */

	c1l = sfdigits(*e1p,-1,&c1p) ;

	c2l = sfdigits(*e2p,-1,&c2p) ;

	if ((c1l == 0) && (c2l == 0))
	    return 0 ;

	if ((c1l > 0) && (c2l == 0))
	    return -1 ;

	if ((c1l == 0) && (c2l > 0))
	    return 1 ;

	f_low1 = (*c1p < '7') ? 1 : 0 ;
	f_low2 = (*c2p < '7') ? 1 : 0 ;
	if (! LEQUIV(f_low1,f_low2)) {

	    c = (f_low1 - f_low2) ;

#if	CF_DEBUGS
	    debugprintf("main/sortfunc: c=%d\n",c) ;
#endif

	    return c ;
	}

/* compare the strings */

	cmplen = MIN(c1l,c2l) ;

	c = strncmp(c1p,c2p,cmplen) ;

	if (c == 0)
	    c = c1l - c2l ;

	return (- c) ;
}
/* end subroutine (cmprev) */


static int sfdigits(s,sl,rpp)
const char	s[] ;
int		sl ;
const char	**rpp ;
{
	int	rs = SR_OK ;
	int	cl ;

	const char	*cp ;


#if	CF_DEBUGS
	debugprintf("main/sfdigits: s=>%s<\n",s) ;
#endif

	if (sl < 0)
	    sl = strlen(s) ;

	while ((sl > 0) && (*s != '_')) {
	    s += 1 ;
	    sl -= 1 ;
	}

	while ((sl > 0) && (! isdigit(*s))) {
	    s += 1 ;
	    sl -= 1 ;
	}

	if (rpp != NULL)
	    *rpp = (char *) s ;

	rs = sl ;

	cp = s ;
	cl = sl ;
	while ((cl > 0) && isdigit(*cp)) {
	    cp += 1 ;
	    cl -= 1 ;
	}

	return (cp - s) ;
}
/* end subroutine (sfdigits) */


