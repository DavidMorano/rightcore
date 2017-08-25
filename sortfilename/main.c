/* main */

/* program to sort file names based on a date string in the name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */
#define	CF_ISAEXEC	1		/* try to use 'isaexec(3c)' */


/* revision history:

	= 1989-03-01, David A­D­ Morano
	This subroutine was originally written.  This whole program, LOGDIR, is
	needed for use on the Sun CAD machines because Sun doesn't support
	LOGDIR or LOGNAME at this time.  There was a previous program but it is
	lost and not as good as this one anyway.  This one handles NIS+ also.
	(The previous one didn't.)

	= 1998-06-01, David A­D­ Morano
        I enhanced the program a little to print out some other user information
        besides the user's name and login home directory.

	= 1999-03-01, David A­D­ Morano
        I enhanced the program to also print out effective UID and effective
        GID.

*/

/* Copyright © 1989,1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ sortfilename < inputfilelist > outputlist


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<sbuf.h>
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

#ifndef	LINELEN
#define	LINELEN		2048
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	sfdigits(const char *,int,char **) ;
static int	sortfunc(const char **,const char **) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"ROOT",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_root,
	argopt_af,
	argopt_of,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	VECSTR		names ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan, npa ;
	int	rs ;
	int	len ;
	int	i, j, k ;
	int	sl, cl, ci ;
	int	c_files = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*sp, *cp, *cp2 ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;

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

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

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
	                        bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

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

/* quiet mode */
	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

				case 'q':
				    pip->verboselevel = 0 ;
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
	                            bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
					break ;

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

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;


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

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;

/* initialze a container to hold filenames */

	rs = vecstr_start(&names,30,VECSTR_PSORTED) ;

	if (rs < 0) {
		ex = EX_OSERR ;
		goto ret1 ;
	}

/* grab the names that are in the invocation arguments */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
		vecstr_add(&names,cp,-1) ;

	            c_files += 1 ;

	} /* end for */

/* grab the names that are in the argument file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	ifile ;

	    char	linebuf[LINELEN + 1] ;


	    if (strcmp(afname,"-") == 0) {
	        rs = bopen(&ifile,BFILE_STDIN,"dr",0666) ;
	    } else
	        rs = bopen(&ifile,afname,"r",0666) ;

	    if (rs >= 0) {

	        while ((rs = breadline(&ifile,linebuf,LINELEN)) > 0) {
		    len = rs ;

	            cl = sfshrink(linebuf,len,&cp) ;

	            if (cl <= 0) continue ;

	            if (cp[0] == '#') continue ;

	            cp[cl] = '\0' ;
	            vecstr_add(&names,cp,cl) ;

	            c_files += 1 ;

	        } /* end while */

	        bclose(&ifile) ;
	    } /* end if */

	} /* end if (argument file) */

/* read from standard input */

	if ((rs >= 0) && (c_files <= 0)) {

	    bfile	ifile ;

	    char	linebuf[LINELEN + 1] ;


	        rs = bopen(&ifile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        while ((rs = breadline(&ifile,linebuf,LINELEN)) > 0) {
		    len = rs ;

	            cl = sfshrink(linebuf,len,&cp) ;

	            if (cl <= 0) continue ;

	            if (cp[0] == '#') continue ;

	            cp[cl] = '\0' ;
	            vecstr_add(&names,cp,cl) ;

	            c_files += 1 ;

	        } /* end while */

	        bclose(&ifile) ;
	    } /* end if */

	} /* end if (standard input) */

/* sort all names */

	vecstr_sort(&names,sortfunc) ;

/* open the output file */

	if (ofname != NULL) {
	    rs = bopen(ofp,ofname,"wct",0666) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open the output file (%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: done w/ opening output \n") ;
#endif

	for (i = 0 ; vecstr_get(&names,i,&cp) >= 0 ; i += 1) {

		if (cp != NULL) 
		bprintf(ofp,"%s\n",cp) ;

	} /* end for */

	bclose(ofp) ;

/* we are done */
badoutopen:
done:
	if (rs < 0) rs = EX_DATAER ;

	vecstr_finish(&names) ;

/* early return thing */
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret2:
	bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

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
	ex = EX_INFO ;
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [filename(s) ...] [-af argfile]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int sortfunc(e1p,e2p)
const char	**e1p, **e2p ;
{
	int	c ;
	int	cmplen ;
	int	c1l, c2l ;
	int	f_low1, f_low2 ;

	char	*c1p, *c2p ;


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
/* end subroutine (sortfunc) */


static int sfdigits(s,slen,rpp)
const char	s[] ;
int		slen ;
char		**rpp ;
{
	int	rs ;
	int	cl ;

	char	*cp ;


#if	CF_DEBUGS
	debugprintf("main/sfdigits: s=>%s<\n",s) ;
#endif

	if (slen < 0)
	    slen = strlen(s) ;

	while ((slen > 0) && (*s != '_')) {
	    s += 1 ;
	    slen -= 1 ;
	}

	while ((slen > 0) && (! isdigit(*s))) {
	    s += 1 ;
	    slen -= 1 ;
	}

	if (rpp != NULL)
	    *rpp = (char *) s ;

	rs = slen ;

	cp = (char *) s ;
	cl = slen ;
	while ((cl > 0) && isdigit(*cp)) {
	    cp += 1 ;
	    cl -= 1 ;
	}

	return (cp - s) ;
}
/* end subroutine (sfdigits) */



