/* main */

/* show the bits set representing terminating characters */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a program to show the field terminator characters from the C
	language syntax of the terminator block code.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfhexi(const char *,int,int *) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strshrink(char *) ;


/* external variables */


/* local structures */

struct adesc {
	uchar	array[32] ;
	int	i ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	adesc_start(struct adesc *) ;
static int	adesc_line(struct adesc *,cchar *,int) ;
static int	adesc_finish(struct adesc *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdname,
	argopt_help,
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


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	struct adesc	array ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		i, len, c ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_usage = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*pr = NULL ;
	const char	*cp ;
	char		argpresent[MAXARGGROUPS] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret0 ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->verboselevel = 1 ;

/* process program arguments */

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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

	                    break ;

/* temporary directory */
	                case argopt_tmpdname:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->tmpdname = argp ;
				} else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdec(avp,avl, &pip->debuglevel) ;
	                        }
	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
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
				break ;

	                    } /* end switch */
	                    akp += 1 ;

			    if (rs < 0) break ;
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

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	if (f_usage)
	    usage(pip) ;

/* set program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* get the input file name */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	        switch (pan) {

	        case 0:
	            ifname = cp ;
	            break ;

	        } /* end switch */

	        pan += 1 ;

	} /* end for */

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* OK, we do it */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: opening output\n") ;
#endif

	rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

	adesc_start(&array) ;

/* read in the array */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: opening input\n") ;
#endif

	if (ifname != NULL) {
	    rs = bopen(ifp,ifname,"r",0666) ;
	} else
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs >= 0) {
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {
		len = rs ;

	        lbuf[len] = '\0' ;
	        cp = strshrink(lbuf) ;

	        if (cp[0] == '\0') continue ;

	        adesc_line(&array,cp,-1) ;

	    } /* end while */

	    bclose(ifp) ;
	} /* end if (opened input file) */

/* print out the array */

	for (i = 0 ; i < 256 ; i += 1) {

	    if (BATST(array.array,i)) {

	        bprintf(ofp,"\t%3d\t",i) ;

	        if ((i & 0x7f) < 32) {

	            switch (i) {

		    case 0:
	                bprintf(ofp,"%\t(nul)") ;

			break ;

	            case CH_TAB:
	                bprintf(ofp,"\t(tab)") ;

	                break ;

	            case CH_FF:
	                bprintf(ofp,"\t(form feed)") ;

	                break ;

	            case CH_VT:
	                bprintf(ofp,"\t(vertical tab)") ;

	                break ;

	            case CH_CR:
	                bprintf(ofp,"\t(carriage return)") ;

	                break ;

	            case CH_LF:
	                bprintf(ofp,"\t(new line)") ;

	                break ;

	            case CH_BS:
	                bprintf(ofp,"\t(back space)") ;

	                break ;

		    case CH_ETX:
	                bprintf(ofp,"\t(ETX)") ;

	                break ;

	            default:
			if (i < 0x20)
	                bprintf(ofp,"%\t(control-%c)",('A' + i - 1)) ;

			else
	                bprintf(ofp,"%\t(*unknown*)") ;

	            } /* end switch */

	        } else {

	            switch (i) {

	            case CH_SPACE:
	                bprintf(ofp,"\t(space)") ;

	                break ;

	            default:
	                bprintf(ofp,"%c",i) ;

	            } /* end switch */

	        } /* end if */

	        bprintf(ofp,"\n") ;

	    } /* end if (we got one) */

	} /* end for */

	adesc_finish(&array) ;

	bclose(ofp) ;

	ex = EX_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

/* good return from program */
badoutopen:
goodret:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

/* we are out of here */
done:
retearly:
ret1:
	if (pip->efp != NULL)
	    bclose(pip->efp) ;

	proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

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
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-v]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int adesc_start(struct adesc *adp)
{
	int		i ;
	for (i = 0 ; i < 32 ; i += 1) {
	    adp->array[i] = 0 ;
	}
	adp->i = 0 ;
	return SR_OK ;
}
/* end subroutine (adesc_start) */


/* process a line of input */
static int adesc_line(struct adesc *adp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		ai = adp->i ;
	uchar		bterms[32] ;

#if	CF_DEBUGS
	debugprintf("adesc_line: index=%d\n",ai) ;
#endif

	fieldterms(bterms,0,",# \t") ;

	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	int		f_hex ;
	int		v ;
	int		fl ;
	cchar		*fp ;
	while ((fl = field_get(&fsb,bterms,&fp)) >= 0) {
	    if (fl > 0) {

	    f_hex = FALSE ;
	    f_hex = f_hex || (strncasecmp(fp,"0x",2) == 0) ;

#if	CF_DEBUGS
	    debugprintf("adesc_line: f_hex=%d\n",f_hex) ;
#endif

	    if (f_hex) {

	        rs = SR_EMPTY ;
	        if ((fl -= 2) > 0) {

	            fp += 2 ;
	            rs = cfhexi(fp,fl,&v) ;

#if	CF_DEBUGS
	            debugprintf("adesc_line: hex rs=%d v=\\x%02X\n",rs, v) ;
#endif

	            if (rs >= 0)
	                adp->array[ai++] = v & 0xFF ;

	        }

	    } else {
	        if ((rs = cfdeci(fp,fl,&v)) >= 0) {
	            adp->array[ai++] = v & 0xFF ;
		}
	    }

#if	CF_DEBUGS
	    debugprintf("adesc_line: rs=%d\n",rs) ;
#endif

	    }
	    if (rs < 0) break ;
	} /* end while */
	field_finish(&fsb) ;
	} /* end if (field) */

#if	CF_DEBUGS
	debugprintf("adesc_line: ret rs=%d index=%d\n",ai) ;
#endif

	adp->i = ai ;
	return (rs >= 0) ? ai : rs ;
}
/* end subroutine (adesc_line) */


static int adesc_finish(struct adesc *adp)
{

	return SR_OK ;
}
/* end subroutine (adesc_finish) */


