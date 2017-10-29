/* progtagprint */

/* make (process) key tags */


#define	CF_DEBUG 	0		/* run-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a single file.

	Synopsis:

	int progtagprint(pip,aip,basedname,outfmt,ofname)
	PROGINFO	*pip ;
	ARGINFO		*aip ;
	const char	basedname[] ;
	const char	outfmt[] ;
	const char	ofname[] ;

	Arguments:

	- pip		program information pointer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<localmisc.h>

#include	"biblebook.h"
#include	"config.h"
#include	"defs.h"
#include	"outfmt.h"


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;

extern int	progtagprinter(PROGINFO *,cchar *,BIBLEBOOK *,
			int, bfile *,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	proctagfile(PROGINFO *,cchar *,BIBLEBOOK *,
			int, bfile *, cchar *) ;


/* local variables */


/* exported subroutines */


int progtagprint(pip,aip,basedname,outfmt,ofname)
PROGINFO	*pip ;
ARGINFO		*aip ;
const char	basedname[] ;
const char	outfmt[] ;
const char	ofname[] ;
{
	BIBLEBOOK	bb ;
	bfile		outfile, *ofp = &outfile ;
	int		rs = SR_OK ;
	int		pan = 0 ;
	int		ofi ;
	int		ai ;
	int		c = 0 ;
	int		f_biblebook = FALSE ;
	int		f ;
	const char	*cp ;

	if (aip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progtagprint: basedname=%s\n",basedname) ;
	    debugprintf("progtagprint: outfmt=%s\n",outfmt) ;
	    debugprintf("progtagprint: ofname=%s\n",ofname) ;
	}
#endif /* CF_DEBUG */

	ofi = -1 ;
	if ((outfmt != NULL) && (outfmt[0] != '\0')) {
	    ofi = matostr(outfmts,2,outfmt,-1) ;
	}

	if (ofi == outfmt_bible) {
	    rs = biblebook_open(&bb,pip->pr,pip->ndbname) ;
	    f_biblebook = (rs >= 0) ;
	}

	if (rs < 0)
	    goto ret0 ;

/* initialize the data structures we need */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = BFILE_STDOUT ;

	rs = bopen(ofp,ofname,"wct",0666) ;
	if (rs < 0)
	    goto retoutopen ;

/* process the positional arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf( "progtagprint: positional arguments\n") ;
	    debugprintf( "progtagprint: argc=%u\n",aip->argc) ;
	    debugprintf( "progtagprint: ai_pos=%u\n",aip->ai_pos) ;
	    debugprintf( "progtagprint: ai_max=%u\n",aip->ai_max) ;
	}
#endif /* CF_DEBUG */

	for (ai = 1 ; ai < aip->argc ; ai += 1) {

	    f = (ai <= aip->ai_max) && (bits_test(&aip->pargs,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = aip->argv[ai] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progtagprint: pa tagfname=%s\n",cp) ;
#endif

	    pan += 1 ;
	    rs = proctagfile(pip,basedname,&bb,ofi,ofp,cp) ;
	        c += rs ;

	    if (rs < 0) {

	        if (*cp == '-')
	            cp = "*stdinput*" ;

	        bprintf(pip->efp,"%s: error processing tag file (%d)\n",
	            pip->progname,rs) ;

	        bprintf(pip->efp,"%s: errored file=%s\n",
	            pip->progname,cp) ;

	    } /* end if */

	    if (rs < 0) break ;
	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progtagprint: argfile=%s\n",aip->afname) ;
#endif

/* process any tags in the argument list file */

	if ((rs >= 0) && (aip->afname != NULL) && (aip->afname[0] != '\0')) {
	    bfile	argfile ;
	    cchar	*afn = aip->afname ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(&argfile,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(&argfile,lbuf,llen)) > 0) {
	            int len = rs ;

	            if (lbuf[len - 1] == '\n')
	                len -= 1 ;

	            lbuf[len] = '\0' ;
	            cp = lbuf ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("progtagprint: af tagfname=%s\n",cp) ;
#endif

	            pan += 1 ;
	            rs = proctagfile(pip,basedname,&bb,ofi,ofp,cp) ;
	                c += rs ;

	            if (rs < 0) {

	                if (*cp == '-')
	                    cp = "*stdinput*" ;

	                bprintf(pip->efp,
	                    "%s: error processing tag file (%d)\n",
	                    pip->progname,rs) ;

	                bprintf(pip->efp,"%s: errored file=%s\n",
	                    pip->progname,cp) ;

	            } /* end if */

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(&argfile) ;
	    } else if (! pip->f.quiet) {
	        bprintf(pip->efp,
	            "%s: could not open the argument list file (%d)\n",
	            pip->progname,rs) ;
	        bprintf(pip->efp,"%s: argfile=%s\n",
	            pip->progname,aip->afname) ;
	    }

	} /* end if (argument file) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = pip->ifname ;
	    if (cp == NULL) cp = "-" ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progtagprint: if tagfname=%s\n",cp) ;
#endif

	    pan += 1 ;
	    rs = proctagfile(pip,basedname,&bb,ofi,ofp,cp) ;
	        c += rs ;

	} /* end if (input file) */

	if (rs >= 0) {
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: tag references=%u\n",
	            pip->progname,c) ;
	} /* end if */

	bclose(ofp) ;

retoutopen:
	if (f_biblebook)
	    biblebook_close(&bb) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progtagprint: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (progtagprint) */


/* local subroutines */


static int proctagfile(pip,basedname,bbp,ofi,ofp,tagfname)
PROGINFO	*pip ;
const char	basedname[] ;
BIBLEBOOK	*bbp ;
int		ofi ;
bfile		*ofp ;
const char	tagfname[] ;
{
	bfile		tagfile, *tfp = &tagfile ;
	int		rs  ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progtagprint/proctagfile: tagfname=%s\n",tagfname) ;
#endif

	if (tagfname == NULL) return SR_FAULT ;

	if (strcmp(tagfname,"-") == 0) tagfname = BFILE_STDIN ;

	if ((rs = bopen(tfp,tagfname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		fnl ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    cchar	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(tfp,lbuf,llen)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;
	        if (f_eol) len -= 1 ;
	        lbuf[len] = '\0' ;

	        cp = lbuf ;
	        if ((cp[0] == '\0') || (cp[0] == '#'))
	            continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("progtagprint/proctagfile: tag=>%s<\n",cp) ;
#endif

	        if (f_bol) {
	            rs = progtagprinter(pip,basedname,bbp,ofi,ofp,cp) ;
	            c += rs ;
	        } /* end if */

	        if (rs < 0) {

	            if (*cp == '-') {
	                fnl = -1 ;
	                cp = "*stdinput*" ;
	            } else
	                fnl = sibreak(cp,-1,":") ;

	            bprintf(pip->efp,
	                "%s: error processing tag reference file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: errored file=%t\n",
	                pip->progname,cp,fnl) ;

	        } /* end if */

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading lines) */
	    rs1 = bclose(tfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proctagfile) */


