/* main (checkprog) */
/* language C89 */

/* program to check for matching C language delimiters */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* run-time debugging */


/* revision history:

	= 2004-09-10, David A­D­ Morano

	This was created quickly as a hack to replace the existing
	CHECKBRA program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This program will check the specified C language files
	for problems.
	

*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		10

#define	NUMNAMES	4
#define	NFUN		2

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	BUFLEN
#define	BUFLEN		200
#endif


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct fun_tab {
	char	*funcname ;
	char	c_open ;
	char	c_close ;
} ;

struct fun_count {
	int	c_open ;
	int	c_close ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;


/* local variables */

static struct fun_tab	cca[] = {
	{ "par", CH_LPAREN, CH_RPAREN },
	{ "bra", CH_LBRACE, CH_RBRACE },
	{ NULL, 0, 0 }
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO		pi, *pip = &pi ;

	struct fun_count	counts[NFUN + 1] ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	cfile, *cfp = &cfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argl, akl ;
	int	ai, ai_max, ai_pos ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	pan = 0 ;
	int	len ;
	int	i, j, k ;
	int	ufi = 0 ;
	int	f_v = FALSE ;
	int	ex = EX_INFO ;
	int	f_usage = FALSE ;

	const char	*argp, *akp ;
	const char	*argval = NULL ;
	const char	*fname[MAXPATHLEN + 1] ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	memset(pip,0,sizeof(PROGINFO)) ;
	pip->verboselevel = 1 ;

	sfbasename(argv[0],-1,&cp) ;

	pip->progname = cp ;

	pip->efp = &errfile ;
	bopen(pip->efp,BFILE_STDERR,"dwca",0666) ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0)
	    return EX_CANTCREAT ;

/* start parsing the arguments */

	i = 1 ;
	argc -= 1 ;

	while ((rs >= 0) && (argc > 0)) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            akp = argp ;
	            akl = argl ;

	            while (--akl) {
			int	kc = (*akp & 0xff) ;

	                akp += 1 ;
	                switch (kc) {

	                case 'D':
	                    pip->debuglevel = 1 ;
	                    break ;

	                case 'V':
	                    f_v = TRUE ;
	                    break ;

	                case 'v':
	                    pip->verboselevel = 2 ;
	                    break ;

	                case '?':
	                    f_usage = TRUE ;
	                    break ;

	                default:
			    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } /* end while */

	        } /* end if */

	    } else {

	        if (ufi < NPARG) {

	            if (argl > 0)
	                fname[ufi++] = argp ;

	        } else {

	            bprintf(pip->efp,
	                "%s: extra file arguments ignored\n",
	                pip->progname) ;

	        }

	    } /* end if */

	} /* end while */

	if (f_v) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    goto retearly ;
	}

/* initialization */

	for (i = 0 ; i < NFUN ; i += 1) {
	    counts[i].c_open = counts[i].c_close = 0 ;
	}

/* loop through the files */

	for (i = 0 ; i < ufi ; i += 1) {

static int procfile(PROGINFO *pip,ARGINFO *aip,BITS *bop,ofn

static int procfile(PROGINFO *pip,cchar *fn)
{
	bfile		cfile, *cfp = &cfile ;
	int		rs ;
	cchar		*pn = pip->progname ;
	ccha		*fmt ;
	if ((rs = bopen(cfp,fn,"r",0666)) < 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    while ((rs = breadline(cfp,lbuf,llen)) > 0) {
	        len = rs ;

	        for (j = 0 ; j < len ; j += 1) {
	            for (k = 0 ; k < NFUN ; k += 1) {
	                if (lbuf[j] == cca[k].c_open) {
	                    counts[k].c_open += 1 ;
	                } else if (lbuf[j] == cca[k].c_close) {
	                    counts[k].c_close += 1 ;
			}
	            } /* end for */
	        } /* end for */

	    } /* end while */

	    if (rs >= 0) {
	    for (k = 0 ; k < NFUN ; k += 1) {

	        if (counts[k].c_open != counts[k].c_close) {

	            bprintf(ofp,"file \"%s\" %s> open %d	close %d\n",
	                fname[i],cca[k].funcname,
	                counts[k].c_open, counts[k].c_close) ;

	        } else {

			if (pip->verboselevel > 0)
	            bprintf(ofp,"file \"%s\" %s> is clean\n",
	                fname[i],cca[k].funcname) ;

			if (pip->debuglevel > 0)
		    bprintf(pip->efp,"%s: file \"%s\" %s> is clean\n",
			pip->progname,
	                fname[i],cca[k].funcname) ;

	        } /* end if */

	    } /* end for */
	    } /* end if */

	    bclose(cfp) ;
	} else {
	    fmt = "%s: inaccessible file (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: file=%s\n",pn,fn) ;
	} /* end for */
	return rs ;
}
/* end subroutine (procfile) */

/* done */
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

ret2:
	bclose(ofp) ;

retearly:
ret1:
	bclose(pip->efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("b_la: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* error types of returns */
badret:
	ex = EX_DATAERR ;
	goto ret1 ;

usage:
	usage(pip) ;

	goto badret ;

badread:
	bprintf(pip->efp,"%s: bad read (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s <file(s)>\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


