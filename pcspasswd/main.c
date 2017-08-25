/* main */

/* main module for the 'passwdv' program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is the front-end subroutine for the PASSWDV (password
	verification) program.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<paramopt.h>
#include	<getxusername.h>
#include	<pwfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	((MAXARGINDEX / 8) + 1)


/* external subroutines */

extern int	isinteractive() ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	process(struct proginfo *,PARAMOPT *,PWFILE *,bfile *,
			const char *) ;

extern char	*strshrink(char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"af",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_option,
	argopt_af,
	argopt_overlast
} ;

static const char	*procopts[] = {
	"seven",
	NULL
} ;

enum procopts {
	procopt_seven,
	procopt_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	struct proginfo	pi, *pip = &pi ;
	PARAMOPT	aparams ;
	PWFILE		pf, *pfp ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1, i ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f_passed = FALSE ;
	int	f ;

	cchar	*argp, *aop, *akp, *avp ;
	cchar	*pr = NULL ;
	cchar	*afname = NULL ;
	cchar	*pwfname = NULL ;
	cchar	*tp, *sp, *cp ;
	char	argpresent[MAXARGGROUPS] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS || CF_DEBUG */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->verboselevel = 1 ;
	pip->tmpdname = NULL ;

	pip->f.nooutput = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.sevenbit = FALSE ;
	pip->f.update = FALSE ;

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

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword or only key letters? */

	            if ((kwi = matstr(argopts,aop,aol)) >= 0) {

	                switch (kwi) {

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
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

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

/* the user specified some options */
	                case argopt_option:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = paramopt_loads(&param,PO_OPTION,
	                                avp,avl) ;

	                    } else {

	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = paramopt_loads(&param,PO_OPTION,
	                                argp,argl) ;

			    }

	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((uint) *aop) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                        }

	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* seven bit stripping option */
	                    case '7':
	                        pip->f.sevenbit = TRUE ;
	                        break ;

/* "no output" option (exit code only?) */
	                    case 'n':
	                        pip->f.nooutput = TRUE ;
	                        break ;

/* alternate PASSWD file */
	                    case 'p':
	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pwfname = argp ;

	                        break ;

/* update */
	                    case 'u':
	                        pip->f.update = TRUE ;
	                        break ;

/* verbose output */
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
	                        bprintf(pip->efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;
			    if (rs < 0)
				break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

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

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto done ;

	ex = EX_DATAERR ;
	if (isinteractive() < 0)
	    goto notinteractive ;

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

/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* options? */

	{
		PARAMOPT_CUR	cur ;


	paramopt_curbegin(&param,&cur) ;

	while (paramopt_enumvalues(&param,PO_OPTION,&cur,&cp) >= 0) {

	    if (cp == NULL) continue ;

	    if ((i = matstr(procopts,cp,-1)) < 0) 
		continue ;

	    switch (i) {

	    case procopt_seven:
	        pip->f.sevenbit = TRUE ;
	        break ;

	    } /* end switch */

	} /* end while (option processing) */

	paramopt_curend(&param,&cur) ;

	} /* end block */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: sevenbit=%d\n",pip->f.sevenbit) ;
#endif


	pfp = NULL ;
	if (pwfname != NULL) {

	    if ((rs = perm(pwfname,R_OK)) < 0)
	        goto badpwfile ;

	    if ((rs = pwfile_open(&pf,pwfname)) >= 0)
	        pfp = &pf ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        debugprintf("main: opened PWFILE \"%s\" (rs %d)\n",
	            pwfname,rs) ;

	        {
	            PWFILE_CUR	cur ;

		PWFILE_ENT	pe ;

		char	pebuf[PWFILE_RECLEN + 1] ;


	            pwfile_curbegin(pfp,&cur) ;

	        debugprintf("main: &pe %08lX pebuf %08lX\n",&pe,pebuf) ;

	            while (pwfile_enum(pfp,&cur,
			&pe,pebuf,PWFILE_RECLEN) >= 0) {


	                debugprintf("username=%s\n",pe.username) ;

	                debugprintf("password=%s\n",pe.password) ;

	                debugprintf("gecos=%s\n",pe.gecos) ;

	                debugprintf("realname=%s\n",pe.realname) ;

	                debugprintf("account=%s\n",pe.account) ;

	                debugprintf("office=%s\n",pe.office) ;

	                debugprintf("home phone=%s\n",pe.hphone) ;

	            } /* end while */

	            pwfile_curend(pfp,&cur) ;

	        } /* end block */

	    } /* end if */
#endif /* CF_DEBUG */

	} /* end if */


/* get ready */

	if ((! pip->f.nooutput) &&
	    ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0644)) < 0))
	    goto badoutopen  ;



/* OK, we do it */

	pan = 0 ;
	if (npa > 0) {

	    for (ai = 0 ; ai <= maxai ; ai += 1) {

	        if ((! BATST(argpresent,ai)) ||
	            (argv[ai][0] == '\0')) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: calling process name=%s\n",argv[ai]) ;
#endif

	        pan += 1 ;
	        rs1 = process(pip,&aparams,pfp,ofp,argv[ai]) ;

		f_passed = (rs1 > 0) ;

	        if (! pip->f.nooutput)
	            bprintf(ofp,"%svalid\n",(! f_passed) ? "in" : "") ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: f_passed=%u\n",f_passed) ;
#endif

	    } /* end for (looping through arguments) */

	} /* end if */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	afile ;


	    if (strcmp(afname,"-") != 0) 
	        rs = bopen(&afile,afname,"r",0666) ;

	    else
	        rs = bopen(&afile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&afile,linebuf,LINEBUFLEN)) > 0) {

		    len = rs ;
	            if (linebuf[len - 1] == '\n') 
			len -= 1 ;

	            linebuf[len] = '\0' ;
	            cp = strshrink(linebuf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs1 = process(pip,&aparams,pfp,ofp,cp) ;

		    f_passed = (rs1 > 0) ;
	            if (! pip->f.nooutput)
	                bprintf(ofp,"%svalid\n",(! f_passed) ? "in" : "") ;

	        } /* end while (reading lines) */

	        bclose(&afile) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"\trs=%d argfile=%s\n",rs,afname) ;

	        }

	    }

	} /* end if (processing file argument file list */

	if (pan <= 0) {

	    char	userbuf[USERNAMELEN + 1] ;


	    getusername(userbuf,USERNAMELEN,-1) ;

	    rs1 = process(pip,&aparams,pfp,ofp,userbuf) ;

		f_passed = (rs1 > 0) ;
	    if (! pip->f.nooutput)
	        bprintf(ofp,"%svalid\n",(! f_passed) ? "in" : "") ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: f_passed=%u\n",f_passed) ;
#endif

	} /* end if */


	if (! pip->f.nooutput)
	    bclose(ofp) ;


bad1:
	if (pwfname != NULL)
	    pwfile_close(&pf) ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting\n") ;
#endif


/* good return from program */
goodret:
	if (rs >= 0) {
		ex = (f_passed) ? EX_OK : EX_DATAERR ;
	} else
		ex = EX_DATAERR ;

/* we are out of here */
done:
ret3:

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: program exiting ex=%u\n",
	        pip->progname,ex) ;
#endif

retearly:
ret2:
ret1:
	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	usage(pip) ;

	goto retearly ;

/* print out some help */
help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* bad stuff comes here */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

notinteractive:
	bprintf(pip->efp,
		"%s: this program can only be executed interactively\n",
	    pip->progname,rs) ;

	goto badret ;

badpwfile:
	bprintf(pip->efp,"%s: could not open PASSWORD file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* bad stuff comes to here */
badret:
	ex = EX_DATAERR ;
	goto done ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (%d)\n",
	    pip->progname,rs) ;

	goto bad1 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;


	bprintf(pip->efp,
	    "%s: USAGE> %s [user ...] [-u] [-Vv]",
	    pip->progname,pip->progname) ;

	rs = bprintf(pip->efp,
	    " [-p passwdfile] \n") ;

	return rs ;
}
/* end subroutine (usage) */



