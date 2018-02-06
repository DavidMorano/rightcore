/* mktagprint */

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

	int mktagprint(pip,aip,basedname,outfmt,outfname)
	struct proginfo	*pip ;
	struct arginfo	*aip ;
	const char	basedname[] ;
	const char	outfmt[] ;
	const char	outfname[] ;

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

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;

extern int	progtagprint(struct proginfo *,const char *,BIBLEBOOK *,
			int, bfile *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	proctagfile(struct proginfo *,const char *,BIBLEBOOK *,
			int, bfile *, const char *) ;


/* local variables */


/* exported subroutines */


int mktagprint(pip,aip,basedname,outfmt,outfname)
struct proginfo	*pip ;
struct arginfo	*aip ;
const char	basedname[] ;
const char	outfmt[] ;
const char	outfname[] ;
{
	BIBLEBOOK	bb ;
	bfile		outfile, *ofp = &outfile ;
	int		rs = SR_OK ;
	int		pan = 0 ;
	int		ofi ;
	int		ai ;
	int		c_tagref = 0 ;
	int		f_biblebook = FALSE ;
	int		f ;
	const char	*cp ;

	if (aip == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("mktagprint: basedname=%s\n",basedname) ;
	    debugprintf("mktagprint: outfmt=%s\n",outfmt) ;
	    debugprintf("mktagprint: outfname=%s\n",outfname) ;
	}
#endif /* CF_DEBUG */

	ofi = -1 ;
	if ((outfmt != NULL) && (outfmt[0] != '\0'))
	    ofi = matostr(outfmts,2,outfmt,-1) ;

	if (ofi == outfmt_bible) {
	    rs = biblebook_open(&bb,pip->pr,pip->ndbname) ;
	    f_biblebook = (rs >= 0) ;
	}

	if (rs < 0)
	    goto ret0 ;

/* initialize the data structures we need */

	if ((outfname != NULL) && (outfname[0] != '\0')) {
	    rs = bopen(ofp,outfname,"wct",0666) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto retoutopen ;

/* process the positional arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf( "mktagprint: positional arguments\n") ;
	    debugprintf( "mktagprint: argc=%u\n",aip->argc) ;
	    debugprintf( "mktagprint: ai_pos=%u\n",aip->ai_pos) ;
	    debugprintf( "mktagprint: ai_max=%u\n",aip->ai_max) ;
	}
#endif /* CF_DEBUG */

	for (ai = 1 ; ai < aip->argc ; ai += 1) {

	    f = (ai <= aip->ai_max) && (bits_test(&aip->pargs,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = aip->argv[ai] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("mktagprint: pa tagfname=%s\n",cp) ;
#endif

	    pan += 1 ;
	    rs = proctagfile(pip,basedname,&bb,ofi,ofp,cp) ;

	    if (rs > 0)
	        c_tagref += rs ;

	    if (rs < 0) {

	        if (*cp == '-')
	            cp = "*stdinput*" ;

	        bprintf(pip->efp,"%s: error processing tag file (%d)\n",
	            pip->progname,rs) ;

	        bprintf(pip->efp,"%s: errored file=%s\n",
	            pip->progname,cp) ;

	    } /* end if */

	    if (rs < 0)
	        break ;

	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mktagprint: argfile=%s\n",aip->afname) ;
#endif

/* process any tags in the argument list file */

	if ((rs >= 0) && (aip->afname != NULL) && 
	    (aip->afname[0] != '\0')) {
	    bfile	argfile ;

	    if (strcmp(aip->afname,"-") != 0) {
	        rs = bopen(&argfile,aip->afname,"r",0666) ;

	    } else
	        rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(&argfile,lbuf,LINEBUFLEN)) > 0) {
	            int len = rs ;

	            if (lbuf[len - 1] == '\n')
	                len -= 1 ;

	            lbuf[len] = '\0' ;
	            cp = lbuf ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("mktagprint: af tagfname=%s\n",cp) ;
#endif

	            pan += 1 ;
	            rs = proctagfile(pip,basedname,&bb,ofi,ofp,cp) ;

	            if (rs > 0)
	                c_tagref += rs ;

	            if (rs < 0) {

	                if (*cp == '-')
	                    cp = "*stdinput*" ;

	                bprintf(pip->efp,
	                    "%s: error processing tag file (%d)\n",
	                    pip->progname,rs) ;

	                bprintf(pip->efp,"%s: errored file=%s\n",
	                    pip->progname,cp) ;

	            } /* end if */

	            if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        bclose(&argfile) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("mktagprint: argfile done \n") ;
#endif

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
	        debugprintf("mktagprint: if tagfname=%s\n",cp) ;
#endif

	    pan += 1 ;
	    rs = proctagfile(pip,basedname,&bb,ofi,ofp,cp) ;

	    if (rs >= 0)
	        c_tagref += rs ;

	} /* end if (input file) */

	if (rs >= 0) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: tag references=%u\n",
	            pip->progname,c_tagref) ;

	} /* end if */

	bclose(ofp) ;

retoutopen:
	if (f_biblebook)
	    biblebook_close(&bb) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mktagprint: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (mktagprint) */


/* local subroutines */


static int proctagfile(pip,basedname,bbp,ofi,ofp,tagfname)
struct proginfo	*pip ;
const char	basedname[] ;
BIBLEBOOK	*bbp ;
int		ofi ;
bfile		*ofp ;
const char	tagfname[] ;
{
	bfile		tagfile, *tfp = &tagfile ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		len ;
	int		fnl ;
	int		c_tagref = 0 ;
	int		f_bol, f_eol ;
	char		lbuf[LINEBUFLEN + 1] ;
	char		*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("mktagprint/proctagfile: tagfname=%s\n",tagfname) ;
#endif

	if (tagfname == NULL)
	    return SR_FAULT ;

	if (strcmp(tagfname,"-") != 0) {
	    rs = bopen(tfp,tagfname,"r",0666) ;
	} else
	    rs = bopen(tfp,BFILE_STDIN,"dr",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("mktagprint/proctagfile: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

	f_bol = TRUE ;
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
	    debugprintf("mktagprint/proctagfile: tag=>%s<\n",cp) ;
#endif

	    if (f_bol) {

	        rs = progtagprint(pip,basedname,bbp,ofi,ofp,cp) ;

	        if (rs > 0)
	            c_tagref += rs ;

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

	    if (rs < 0)
	        break ;

	    f_bol = f_eol ;

	} /* end while (reading lines) */

	bclose(tfp) ;

ret0:
	return (rs >= 0) ? c_tagref : rs ;
}
/* end subroutine (proctagfile) */


