/* main */

/* test program (front-end) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	1		/* run-time debug print-outs */


/* revision history:

	= 1989-03-01, David A­D­ Morano
        This subroutine was originally written. This whole program, LOGDIR, is
        needed for use on the Sun CAD machines because Sun doesn't support
        LOGDIR or LOGNAME at this time. There was a previous program but it is
        lost and not as good as this one anyway. This one handles NIS+ also.
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

	$ testgecos.x [username]


*******************************************************************************/


#include	<evnstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<baops.h>
#include	<realname.h>
#include	<getax.h>
#include	<sysusernames.h>
#include	<dater.h>		/* for DATER_ZNAMESIZE */
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"gecos.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	initnow(struct timeb *,char *,int) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct keynum {
	int	i ;
	char	*name ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;
static int	handlename(PROGINFO *,bfile *,const char *) ;
static int	getkeyname(int,char **) ;


/* define command option words */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"af",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_af,
	argopt_overlast
} ;

static struct keynum	keynums[] = {
	{ gecosval_organization, "organization" },
	{ gecosval_realname, "realname" },
	{ gecosval_bin, "bin" },
	{ gecosval_account, "account" },
	{ gecosval_office, "office" },
	{ gecosval_hphone, "hphone" },
	{ gecosval_wphone, "wphone" },
	{ gecosval_printer, "printer" },
	{ 0, NULL },
} ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sb ;

	PROGINFO	pi, *pip = &pi ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	time_t	daytime = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	len, i, j, k ;
	int	sl, ci ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_self = FALSE ;
	int	f_entok = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*mbfname = NULL ;
	const char	*cp, *cp2 ;
	char		argpresent[MAXARGGROUPS] ;
	char		timebuf[TIMEBUFLEN + 1] ;

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

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + aol - avp ;
	                f_optequal = TRUE ;

	            } else {

	                akl = aol ;
	                avl = 0 ;

	            }

/* do we have a keyword match or only key letters? */

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
	                        if (avl > 0) {

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                        }
	                    }

	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
			    breka ;

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

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

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

/* get the current time-of-day */

	initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;

/* open the output file */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (f) {
	        cp = argv[ai] ;
		if (cp[0] != '\0') {
	            pan += 1 ;
	            rs = handlename(pip,ofp,cp) ;
		}
	    }

	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (pan == 0)) {
	    SYSUSERNAMES	u ;
	    if ((rs = sysusernames_open(&u,NULL)) >= 0) {
		const int	unlen = USERNAMELEN ;
		char		unbuf[USERNAMELEN+1] ;
	 	while ((rs = sysusernames_readent(&u,unbuf,unlen)) > 0) {
			pan += 1 ;
	    		if ((rs = handlename(pip,ofp,unbuf)) >= 0) {
			    rs = bputc(ofp,'\n') ;
			}
			if (rs < 0) break ;
		} /* end while */
		sysusernames_close(&u) ;
	    } /* end if (sysusernames) */
	} /* end if (default) */

	bclose(ofp) ;
	} /* end if (file-output) */

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

ret3:

/* we are done */
done:
ret2:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

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

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open the output file (%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> %s user(s) \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int handlename(pip,ofp,name)
PROGINFO	*pip ;
bfile		*ofp ;
const char	name[] ;
{
	struct passwd	pw ;
	GECOS		g ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs, rs1 ;
	int		i ;
	int		cl, kl ;
	const char	*cp, *kp ;
	char		*pwbuf ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	if ((rs = getpw_name(&pw,pwbuf,pwlen,name)) >= 0) {

	    bprintf(ofp,"user=%s\n",name) ;

	    if ((rs1 = gecos_start(&g,pw.pw_gecos,-1)) >= 0) {

	        for (i = 0 ; i < gecosval_overlast ; i += 1) {

	            cl = gecos_getval(&g,i,&cp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: gecos_getval() i=%u rs=%d\n",i,cl) ;
#endif

	            if (cl >= 0) {

	                kl = getkeyname(i,&kp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: getkeyname() i=%u kl=%d\n",i,kl) ;
		if (kp != NULL)
	        debugprintf("main: getkeyname() kp=%s\n",kp) ;
	}
#endif

	                if (kl < 0)
	                    kp = "unknown" ;

	                bprintf(ofp,"%t=%t\n",kp,kl,cp,cl) ;

	            } /* end if (got a value) */

	        } /* end for */

	        gecos_finish(&g) ;
	    } /* end if (initialized GECOS) */

	} /* end if (got one) */
	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (handlename) */


static int getkeyname(n,rpp)
int	n ;
char	**rpp ;
{
	int	i ;
	int	len = -1 ;

	for (i = 0 ; keynums[i].name != NULL ; i += 1) {
	    if (keynums[i].i == n) break ;
	} /* end for */

	if (keynums[i].name != NULL) {
	    *rpp = keynums[i].name ;
	    len = strlen(*rpp) ;
	}

	return len ;
}
/* end subroutine (getkeyname) */



