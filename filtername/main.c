/* main (filtername) */

/* generic front-end */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ filtername [names(s) ...] [-af namefile]


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<hdbstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 3),2048)
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* forward references */

static int	usage(struct proginfo *) ;
static int	isfileok(const char *) ;


/* local structures */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"pm",
	"sn",
	"af",
	"if",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_if,
	argopt_of,
	argopt_overlast
} ;

static const char	*progmodes[] = {
	"filefilter",
	"fileuniq",
	"filtername",
	NULL
} ;

enum progmodes {
	progmode_filefilter,
	progmode_fileuniq,
	progmode_filtername,
	progmode_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct ustat	sb ;

	HDBSTR	mdb ;

	bfile	errfile ;
	bfile	infile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	len ;
	int	i, j, k ;
	int	cl, bnl ;
	int	ex = EX_INFO ;
	int	fd_debug = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*pmspec = NULL ;
	const char	*searchname = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*bnp ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
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
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            if ((argl - 1) > 0)
	                rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

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

/* do we have a keyword match or only key letters? */

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

	                case argopt_pm:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl > 0)
	                            pmspec = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pmspec = argp ;

	                    }

	                    break ;

	                case argopt_sn:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl > 0)
	                            searchname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            searchname = argp ;

	                    }

	                    break ;

/* argument file */
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

/* input file */
	                case argopt_if:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            ifname = argp ;

	                    }

	                    break ;

/* output file */
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
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
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
	                            "%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

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
	    debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel %u\n",
	        pip->progname,pip->debuglevel) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

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

	if (searchname == NULL)
	    searchname = getenv(VARSEARCHNAME) ;

#ifdef	COMMENT
	if ((searchname == NULL) && (pmspec != NULL)) {

	    searchname = pmspec ;

	}
#endif /* COMMENT */

	if (searchname == NULL) {

	    searchname = pip->progname ;
	    if ((cp = strchr(pip->progname,'.')) != NULL) {

	        searchname = tmpfname ;
	        strwcpy(tmpfname,pip->progname,(cp - pip->progname)) ;

	    }
	}

	proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

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

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->searchname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if (pip->progmode < 0)
	    pip->progmode = progmode_filtername ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0)
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif

/* help file */

	if (f_help)
	    goto help ;

/* initialize what we need */

	rs = hdbstr_start(&mdb,100) ;

	if (rs < 0)
	    goto done ;

/* gather the arguments to match against */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    pan += 1 ;
	    rs = hdbstr_add(&mdb,argv[ai],-1,NULL,0) ;

	    if (rs < 0) {

	        bprintf(pip->efp,
	            "%s: processing error (%d) in file=%s\n",
	            pip->progname,rs,argv[ai]) ;

	        break ;
	    }

	} /* end for */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	afile ;

	    char	linebuf[LINEBUFLEN + 1] ;


	    if (strcmp(afname,"-") != 0)
	        rs = bopen(&afile,afname,"r",0666) ;

	    else
	        rs = bopen(&afile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        while ((rs = breadline(&afile,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            cl = sfshrink(linebuf,len,&cp) ;

	            if ((cl <= 0) || (cp[0] == '#')) continue ;

	            cp[cl] = '\0' ;
	            pan += 1 ;
	            rs = hdbstr_add(&mdb,cp,cl,NULL,0) ;

	            if (rs < 0) {

	                bprintf(pip->efp,
	                    "%s: processing error (%d) in file=%t\n",
	                    pip->progname,rs,cp,cl) ;

	                break ;
	            }

	        } /* end while */

	        bclose(&afile) ;

	    } /* end if */

	} /* end if (argument file) */

	if (rs < 0)
	    goto ret2 ;

/* open output */

	rs = bopen(&outfile,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open the output file (%d)\n",
	        pip->progname,rs) ;
	    goto badopenout ;
	}

/* open input */

	rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	if (rs < 0) {
	    ex = EX_NOINPUT ;
	    bprintf(pip->efp,"%s: could not open input (%d)\n",
	        pip->progname,rs) ;
	    goto badopenin ;
	}

/* read an input line */

	while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

	    cl = sfshrink(linebuf,len,&cp) ;

	    if (cl > 0) {

	        cp[cl] = '\0' ;
	        bnl = sfbasename(cp,cl,&bnp) ;

	        rs1 = SR_NOENT ;
	        if (bnl > 0)
	            rs1 = hdbstr_fetch(&mdb,bnp,bnl,NULL,NULL) ;

	        if (rs1 == SR_NOENT) {

	            cp[cl++] = '\n' ;
	            rs = bwrite(&outfile,cp,cl) ;
	            if (rs < 0)
	                break ;

	        }

	    } /* end if (got a filename) */

	} /* end while (reading lines) */

	bclose(&infile) ;

badopenin:
	bclose(&outfile) ;

badopenout:
	hdbstr_finish(&mdb) ;

/* we are done */
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

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


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-V]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int isfileok(fname)
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	fd ;

	char	buf[10] ;


	rs = u_open(fname,O_RDONLY,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(fd,&sb) ;

	if ((rs >= 0) && S_ISSOCK(sb.st_mode))
	    rs = SR_NOTSUP ;

	if (rs >= 0) {

	    rs = u_read(fd,buf,4) ;

	    if (rs >= 4)
	        rs = (memcmp(buf,"\177ELF",4) != 0) ? SR_OK : SR_NOTSUP ;

	} /* end if (opened) */

	u_close(fd) ;

ret0:
	return (rs >= 0) ? TRUE : FALSE ;
}
/* end subroutine (isfileok) */



