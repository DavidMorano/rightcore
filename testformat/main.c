/* main */

/* test 'format(3bfile)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	1		/* run-time debug print-outs */
#define	CF_STDERR	1		/* use 'stdio(3s)' */
#define	CF_BFILE	1		/* use 'bfile(3b)' */


/* revision history:

	= 2008-08-28, David A­D­ Morano

	This is a complete rewrite of the previous program by the
	same name.  We perform the same function but have rewritten it
	completely from scratch (that is the way life is sometimes!).


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ testformat.x


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<paramopt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strdcpy1(char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;
static int	process(PROGINFO *,bfile *) ;

static char	*remeol(char *,const char *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"TMPDIR",
	"af",
	"of",
	"set",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_tmpdir,
	argopt_af,
	argopt_of,
	argopt_set,
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
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;

	PARAMOPT	aparams ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs, rs1 ;
	int	len, v ;
	int	i, j, k ;
	int	sl, ci ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*searchname = NULL ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp, *cp2 ;

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

	proginfo_setbanner(pip,BANNER) ;

#if	CF_BFILE
	cp = getenv(VARERRFILE) ;
	if (cp != NULL) {
	    rs1 = bopen(&errfile,cp, "wca", 0666) ;
	} else
	    rs1 = bopen(&errfile, BFILE_STDERR, "dwca", 0666) ;
	if (rs1 >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}
#endif /* CF_BFILE */

/* early things to initialize */

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

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

	        if (isdigitlatin(argp[1]&0xff)) {

		    argval = (argp+1) ;

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

	            if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                switch (kwi) {

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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose */
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

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->tmpdname = argp ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some progopts */
	                case argopt_set:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        rs = paramopt_loadu(&aparams,argp,argl) ;
	                    break ;

/* argument files */
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

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                        break ;

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

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* verbose output */
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
	                        bprintf(pip->efp,"%s: unknown option - %c\n",
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
	debugprintf("main: d_ debuglevel=%u\n",pip->debuglevel) ;
#endif

#if	CF_DEBUGS
	debugprintf("main: ds debuglevel=%u\n",pip->debuglevel) ;
#endif

/* check arguments */

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

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
	}

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* start processing */

	if ((ofname != NULL) && (ofname[0] != '\0')) {
	    rs = bopen(ofp,ofname,"wct",0644) ;

	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

#if	CF_STDERR
	fprintf(stderr,"main: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
		ex = EX_CANTCREAT ;
		bprintf(pip->efp,"%s: output inaccessible (%d)\n",
	    	pip->progname,rs) ;
		goto badoutopen ;
	}

/* OK, we do it */

	rs = process(pip,ofp) ;

	bclose(ofp) ;

badoutopen:
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

/* we are out of here */

	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:
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
	int		rs = SR_OK ;
	int		wlen = 0 ;

	rs = bprintf(pip->efp,"%s: *no-arguments*\n",pip->progname) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(pip,ofp)
PROGINFO	*pip ;
bfile		*ofp ;
{
	int		rs = SR_OK ;
	int		i ;
	int		num ;
	const char	*fmt ;
	char		fbuf[BUFLEN + 1] ;
	char		lbuf[BUFLEN + 1] ;

	if (rs >= 0) {
	    const char	*s = "hello world!" ;
		i = 1 ;
	rs = bufprintf(fbuf,BUFLEN,"%s\n",s) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	}

	if (rs >= 0) {
	    const char	*s = "hello world!" ;
		i = 2 ;
	rs = bufprintf(fbuf,BUFLEN,"%t\n",s,-1) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	}

	if (rs >= 0) {
	i = 3 ;
	num = 17 ;
	rs = bufprintf(fbuf,BUFLEN,"num=%hx\n",num) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	}

	if (rs >= 0) {
		i = 4 ;
	num = -128 ;
	rs = bufprintf(fbuf,BUFLEN,"num=%08x\n",num) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	}

	if (rs >= 0) {
	num = 2 ;
	rs = ctdeci(fbuf,BUFLEN,num) ;
	fprintf(stderr,"main: 5 ctdeci() rs=%d b=%s\n",rs,fbuf) ;
	}

	if (rs >= 0) {
	num = 17 ;
	rs = bufprintf(fbuf,BUFLEN,"num=%o\n",num) ;
	fprintf(stderr,"main: 6 bufprintf() rs=%d\n",rs) ;
	}

	if (rs >= 0) {
	num = 17 ;
	rs = bufprintf(fbuf,BUFLEN,"num=%.7o\n",num) ;
	fprintf(stderr,"main: 7 bufprintf() rs=%d\n",rs) ;
	}

	if (rs >= 0) {
	num = 17 ;
	rs = bufprintf(fbuf,BUFLEN,"num=%11.7o\n",num) ;
	fprintf(stderr,"main: 8 stdio num=%11.7o\n",num) ;
	fprintf(stderr,"main: 8 bufprintf() rs=%d\n",rs) ;
	}

	if (rs >= 0) {
	num = 2 ;
	rs = bufprintf(fbuf,BUFLEN,"num=%u\n",num) ;
	fprintf(stderr,"main: 9 bufprintf() rs=%d\n",rs) ;
	}

	if (rs >= 0) {
	i = 10 ;
	num = -2 ;
	fmt = "num=%d\n" ;
	fprintf(stderr,"main: %u fmt=>%s<\n",i,remeol(lbuf,fmt)) ;
	rs = bufprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	snprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u stdio            b=>%s<\n",
		i,remeol(lbuf,fbuf));
	}

	if (rs >= 0) {
	i = 11 ;
	num = -2 ;
	fmt = "num=%.3d\n" ;
	fprintf(stderr,"main: %u fmt=>%s<\n",i,remeol(lbuf,fmt)) ;
	rs = bufprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	snprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u stdio            b=>%s<\n",
		i,remeol(lbuf,fbuf));
	}

	if (rs >= 0) {
	i = 12 ;
	num = -2 ;
	fmt = "num=%3d\n" ;
	fprintf(stderr,"main: %u fmt=>%s<\n",i,remeol(lbuf,fmt)) ;
	rs = bufprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	snprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u stdio            b=>%s<\n",
		i,remeol(lbuf,fbuf));
	}

	if (rs >= 0) {
	i = 13 ;
	num = -2 ;
	fmt = "num=%03d\n" ;
	fprintf(stderr,"main: %u fmt=>%s<\n",i,remeol(lbuf,fmt)) ;
	rs = bufprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	snprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u stdio            b=>%s<\n",
		i,remeol(lbuf,fbuf));
	}

	if (rs >= 0) {
	i = 14 ;
	num = 2 ;
	fmt = "num=%04x\n" ;
	fprintf(stderr,"main: %u fmt=>%s<\n",i,remeol(lbuf,fmt)) ;
	rs = bufprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		i,rs,remeol(lbuf,fbuf)) ;
	snprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u stdio            b=>%s<\n",
		i,remeol(lbuf,fbuf));
	}

	if (rs >= 0) {
	    i = 15 ;
	    num = 2 ;
	    {
	        fmt = "num=%04ß\n" ;
	        fprintf(stderr,"main: %u fmt=>%s<\n",i,remeol(lbuf,fmt)) ;
	        rs = bufprintf(fbuf,BUFLEN,fmt,num) ;
	        fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		        i,rs,remeol(lbuf,fbuf)) ;
	    }
	    {
		int	v = 0 ;
	        fmt = "num=%4ß\n" ;
	        fprintf(stderr,"main: %u fmt=>%s<\n",i,remeol(lbuf,fmt)) ;
	        rs = bufprintf(fbuf,BUFLEN,fmt,v) ;
	        fprintf(stderr,"main: %u bufprintf() rs=%d b=>%s<\n",
		        i,rs,remeol(lbuf,fbuf)) ;
	    }
#ifdef	COMMENT /* unknown format specifier to STDIO */
	snprintf(fbuf,BUFLEN,fmt,num) ;
	fprintf(stderr,"main: %u stdio            b=>%s<\n",
		i,remeol(lbuf,fbuf));
#endif /* COMMENT */
	}

	return rs ;
}
/* end subroutine (process) */


static char *remeol(char *lbuf,const char *s)
{
	char	*lbp ;
	lbp = strdcpy1(lbuf,BUFLEN,s) ;
	if ((lbuf[0] != '\0') && (lbp[-1] == '\n')) lbp[-1] = '\0' ;
	return lbuf ;
}
/* end subroutine (remeol) */



