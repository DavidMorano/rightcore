/* main (termcarrier) */
/* lang=C99 */

/* part of 'termcarrier' -- perform carrier operations */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The program was copied very substantially from 'hangup'.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Do something with terminal carrier.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	process(PROGINFO *,const char *,const char *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */

static cchar	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"dn",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_dn,
	argopt_overlast
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	g ;
	USERINFO	u ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan, npa ;
	int	rs ;
	int	i ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	fd_debug ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*devbase = DEVDNAME ;
	const char	*cp ;
	char	argpresent[MAXARGGROUPS] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(&g,0,sizeof(PROGINFO)) ;

	g.progname = strbasename(argv[0]) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else {
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	}
	if (rs >= 0) {
	    g.efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	g.ofp = ofp ;
	g.verboselevel = 1 ;
	g.verboselevel = FALSE ;

	g.f.print = TRUE ;

/* process program arguments */

	rs = SR_OK ;
	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	ai_max = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    g.verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &g.verboselevel) ;

	                            }
	                        }

	                        break ;

	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) 
					g.tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) 
					goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) 
					g.tmpdname = argp ;

	                    }

	                    break ;

/* device base directory */
	                    case argopt_dn:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					devbase = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					devbase = argp ;

	                        }

	                        break ;

/* default action and user specified help */
	                default:
			rs = SR_INVALID ;
				ex = EX_USAGE ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
					if (avl)
	                            rs = cfdeci(avp,avl, &g.debuglevel) ;

	                        }

	                        break ;

/* device base directory */
	                    case 'd':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					devbase = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					devbase = argp ;

	                        }

	                        break ;

/* do not actually do it ! */
	                    case 'n':
	                        g.f.fakeit = TRUE ;
	                        break ;

/* print out something */
	                    case 'p':
	                        g.f.print = TRUE ;
	                        break ;

/* verbose output */
	                    case 'v':
	                            g.verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &g.verboselevel) ;

	                            }

	                            break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(g.efp,"%s: unknown option - %c\n",
	                            g.progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;
			if (rs < 0)
				break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                ai_max = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            ai_max = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(g.efp,"%s: extra arguments ignored\n",
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (g.debuglevel > 0) {
	    bprintf(g.efp, "%s: debuglevel=%u\n", g.progname,g.debuglevel) ;
	    bcontrol(g.efp,BC_LINEBUF,0) ;
	    bflush(g.efp) ;
	}

	if (f_version)
	    bprintf(g.efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto done ;

/* check a few more things */

	if (g.tmpdname == NULL)
	    g.tmpdname = getenv(VARTMPDNAME) ;

	if ((g.tmpdname == NULL) || (u_access(g.tmpdname,W_OK) < 0))
	    g.tmpdname = TMPDNAME ;

/* is the device base directory there? */

	if (u_access(devbase,R_OK) != 0) {

		ex = EX_OSFILE ;
	    bprintf(g.efp,
	        "%s: could not access device directory=%s\n",
	        g.progname,devbase) ;

	    goto done ;
	}

	if (g.verboselevel > 0)
	    bprintf(g.ofp,"%s: dev_directory=%s\n",
	        g.progname,devbase) ;

/* OK, we do it */

	if (npa > 0) {

	    if (g.f.print) {

	        if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0644)) < 0)
	            goto badoutopen ;

	    }

	    for (i = 0 ; i <= ai_max ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

		cp = argv[i] ;
	        rs = process(&g,devbase,cp) ;

		if (rs < 0)
		    break ;

	    } /* end for (looping through requested devices) */

	    if (g.f.print)
	        bclose(ofp) ;

	} else
	    goto badnoterms ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: ret \n") ;
#endif

/* good return from program */
goodret:
	bclose(g.ofp) ;

done:
	if (g.debuglevel > 0)
	    bprintf(g.efp,"%s: exiting ex=%u (%d)\n",
	        g.progname,ex,rs) ;

/* we are out of here */
ret2:
	bclose(g.efp) ;

ret1:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	bprintf(g.efp,
	    "%s: USAGE> %s device=state [device=state [...]] \n",
	    g.progname,g.progname) ;

	bprintf(g.efp, 
		"\t[-n] [-d devbase] [-V]\n") ;

	goto done ;

badargextra:
	bprintf(g.efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badarg ;

badargval:
	bprintf(g.efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badarg ;

badargnum:
	bprintf(g.efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badarg ;

badarg:
	goto done ;

badoutopen:
	bprintf(g.efp,"%s: could not open output (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badinopen:
	bprintf(g.efp,"%s: could not open standard input (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

baduser:
	bprintf(g.efp,"%s: could not user information (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badnoterms:
	bprintf(g.efp,"%s: no terminals were specified\n",
	    g.progname) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */


