/* progfile */

/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* non-switchable debug print-outs */
#define	CF_TESTLINE	0		/* test long lines */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module will take as input a filename and it will perform specified
	cleanup activities on the contents of the file and write the results
	out to either STDOUT or back to the original file.

	= Forward verses backwards

	I've actually coded up two different ways to break long lines.  One is
	called "backward" since it searches for a line break starting from the
	end of the line and moving towards the beginning of the line.  This was
	the first algorithm implemented.  But tabs embeeded in the lines were
	not properly handled since there was no column count available (and
	tabs can take up more than one column) in order to know how long the
	leading part of a line was (column-wise).  So I coded up the "forward"
	algorithm.  This latter one searches for a line break starting from the
	beginning of the line and also keeps track of columns used by the
	leading part of the line.  This is the algorithm currently being used.

	= Forward algorithm

	We search ahead in the line for a 'token'.  The token is either a
	single whitespace character (so that it can be examined for its
	specific type (perhaps a tab as opposed to a regular space) or a
	sequence of non-whitespace characters.  All sequences of non-whitespace
	characters are always kept together -- as they should be.  Whitespace
	occuring after an output line is filled, is removed when the "rmwhite"
	option is turned ON (the default).  Further, whitespace characters that
	are not ignored are examined to see if they are tabs and if so, they
	are processed to determine how many characters columns they will take
	in the output line.  The created output line length is updated
	accordingly.

	= Note on "rmwhite" option

	This option generally gives a more pleasing output appearance for those
	certain (perhaps) rare circumstances where one or more whitespace
	characters might mess up the output by creating things like
	whitespace-only lines where none existed in the input.  Consequently,
	this option is ON by default.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#if	CF_TESTLINE && CF_DEBUG
#undef	LINEBUFLEN
#define	LINEBUFLEN	20
#else
#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(4096,LINE_MAX)
#else
#define	LINEBUFLEN	4096
#endif
#endif
#endif

#ifndef	NTABCOLS
#define	NTABCOLS	8
#endif

#undef	CHAR_TOKSEP
#define	CHAR_TOKSEP(c)	(CHAR_ISWHITE(c) || (! isprintlatin(c)))

#define	NBLANKS		40

#define	LINEDESC	struct linedesc


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	bwriteblanks(bfile *,int) ;
extern int	tabcols(int,int) ;
extern int	iceil(int,int) ;
extern int	isprintlatin(int) ;


/* external variables */


/* local structures */

struct linedesc {
	char		*lp ;
	int		ll ;
	int		lo ;
	int		li ;
	int		olenr ;
	int		nline ;
	int		f_bol:1 ;
	int		f_eol:1 ;
	int		f_eof:1 ;
} ;


/* forward references */

static int	procline(PROGINFO *,struct procoptions *,bfile *,LINEDESC *) ;
static int	nexttoken(const char *,int,const char **) ;


/* local variables */


/* exported subroutines */


int progfile(pip,opts,fname)
PROGINFO	*pip ;
struct procoptions	opts ;
const char	fname[] ;
{
	LINEDESC	ld ;
	bfile		infile, *ifp = &infile ;
	bfile		tmpfile, *ofp = &tmpfile ;
	const int	llen = LINEBUFLEN ;
	int		rs, rs1 ;
	int		len, lo ;
	int		rlen ;
	int		wlen = 0 ;
	const char	*oflags ;
	char		template[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("progfile: ent, fname=%s\n",fname) ;
	    debugprintf("progfile: opts inplace=%u rmwhite=%u\n",
	        opts.inplace,opts.rmwhite) ;
	}
#endif /* CF_DEBUG */

	tmpfname[0] = '\0' ;

/* open files */

	if ((fname == NULL) || (fname[0] == '\0') ||
	    (fname[0] == '-')) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: opening STDIN\n") ;
#endif

	    oflags = (opts.inplace) ? "drw" : "dr" ;
	    rs = bopen(ifp,BFILE_STDIN,oflags,0666) ;
	} else {
	    oflags = (opts.inplace) ? "rw" : "r" ;
	    rs = bopen(ifp,fname,oflags,0666) ;
	}

	if (rs < 0) {
	    if (! pip->f.quiet)
		bprintf(pip->efp,"%s: unavailable (%d) file=%s\n",
			pip->progname,rs,fname) ;
	    goto ret0 ;
	}

	if (opts.inplace) {
	    rs = mkpath2(template,pip->tmpdname,"tcXXXXXXXXXXXX") ;

	    if (rs >= 0)
	        rs = mktmpfile(tmpfname,0666,template) ;

	    if (rs >= 0) {
	        ofp = &tmpfile ;
	        rs = bopen(&tmpfile,tmpfname,"rwct",0666) ;
	    }

	    u_unlink(tmpfname) ;

	    if (rs < 0)
	        goto retclose ;

	} else
	    ofp = pip->ofp ;

	if ((! opts.inplace) && (opts.unbuf)) {
	    bcontrol(ifp,BC_LINEBUF,0) ;
	    bcontrol(ofp,BC_LINEBUF,0) ;
	}

/* go through the loops */

	memset(&ld,0,sizeof(LINEDESC)) ;
	ld.olenr = pip->linewidth ;
	ld.f_eof = FALSE ;
	ld.f_bol = TRUE ;

	rlen = 0 ;
	lo = 0 ;
	while ((! ld.f_eof) || (lo > 0)) {
	    int		ll ;
	    char	*lp = lbuf ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: f_eof=%u lo=%u\n",ld.f_eof,lo) ;
#endif

	    rs = 0 ;
	    if (! ld.f_eof) {

	        rs = breadline(ifp,(lbuf + lo),(llen - lo)) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("progfile: breadline() rs=%d\n",rs) ;
#endif

	        ld.f_eof = (rs == 0) ;
	        if (rs < 0) break ;

	    } /* end if (not EOF) */

	    len = lo + rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: len=%u\n",len) ;
#endif

	    if (len == 0)
		break ;

	    ld.f_eol = (lbuf[len - 1] == '\n') ? TRUE : FALSE ;

	    rlen += len ;
	    if (lbuf[len - 1] == '\n') len -= 1 ;

	    lbuf[len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: line=>%t<\n",lbuf,len) ;
#endif

	    lp = lbuf ;
	    ll = len ;

	    ld.lp = lp ;
	    ld.ll = ll ;

	    rs = procline(pip,&opts,ofp,&ld) ;
	    wlen += rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: procline() rs=%d\n",rs) ;
#endif

	    lo = ld.lo ;
	    if (rs < 0) break ;

	    ld.f_bol = ld.f_eol ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("progfile: loop wlen=%u lo=%u\n",wlen,lo) ;
	        debugprintf("progfile: loop f_eof=%u\n",ld.f_eof) ;
	}
#endif

	} /* end while (reading lines) */

/* write back if we were to process "in place" */

	if ((rs >= 0) && opts.inplace) {
	    int	tlen = 0 ;
	    int	f_ok = (rs >= 0) ;

	    if (f_ok) {

	        rs = bseek(ifp,0L,SEEK_SET) ;

	        if (rs >= 0)
	            rs = bseek(ofp,0L,SEEK_SET) ;

	        if (rs >= 0) {
	            rs = bcopyblock(ofp,ifp,wlen) ;
		    tlen = rs ;
		}

	        if (rs >= 0)
	            bcontrol(ifp,BC_TRUNCATE,tlen) ;

	    } /* end if (OK to do) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (inplace) */

retclose:
	bclose(ifp) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ret rs=%d wlen=%d\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


static int procline(pip,optp,ofp,ldp)
PROGINFO	*pip ;
struct procoptions	*optp ;
bfile		*ofp ;
LINEDESC	*ldp ;
{
	int		rs = SR_OK ;
	int		li, ll ;
	int		tl, cl ;
	int		stepcols, havecols ;
	int		wlen = 0 ;
	int		f_badchar ;
	int		f_copydown = FALSE ;
	int		f_fold = FALSE ;
	int		f_whitechar ;
	int		f_eol = FALSE ;
	int		f ;
	const char	*lp ;
	const char	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	        debugprintf("process/procline: ent\n") ;
#endif

	lp = ldp->lp ;
	ll = ldp->ll ;
	li = 0 ;
	while ((rs >= 0) && (ll > 0)) {

	    cl = nexttoken(lp,ll,&cp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("process/procline: li=%u olenr=%u\n",
	            ldp->li,ldp->olenr) ;
	        debugprintf("process/procline: tl=%u token=>%t<\n",
	            cl,cp,cl) ;
	    }
#endif /* CF_DEBUG */

	    if (cl <= 0)
	        break ;

	    f_eol = (cp[cl - 1] == '\n') ;

/* get out early if we don't have an EOL */

	    f_copydown = ((! ldp->f_eol) && (! ldp->f_eof) && 
		((cp + cl) == (lp + ll))) ;

	    if (f_copydown)
	        break ;

/* continue with normal processing */

	    li += cl ;
	    tl = cl ;

/* determine what sort of character we have (if only one) */

	    f_whitechar = FALSE ;
	    f_badchar = FALSE ;
	    if (*cp == '\t') {

	        f_whitechar = TRUE ;
	        stepcols = iceil(((ldp->li % NTABCOLS) + 1),NTABCOLS) ;

	    } else if (*cp == '\r') {

	        f_whitechar = TRUE ;
	        stepcols = 0 ;
	        cl = 0 ;

	    } else if ((cl == 1) && (! isprintlatin(*cp))) {

	        f_whitechar = TRUE ;
	        f_badchar = TRUE ;
	        stepcols = 1 ;
	        cl = 1 ;

	    } else {

	        f_whitechar = CHAR_ISWHITE(*cp) ;
	        stepcols = cl ;

	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("process/procline: stepcols=%u\n",stepcols) ;
	        debugprintf("process/procline: f_whitechar=%u\n",f_whitechar) ;
	    }
#endif /* CF_DEBUG */

/* over-line calculation */

	    if ((ldp->nline == 0) || (ldp->li > NTABCOLS)) {
	        havecols = ldp->olenr ;
	    } else
	        havecols = pip->linewidth - NTABCOLS ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process/procline: havecols=%u li=%u\n",
	            havecols,ldp->li) ;
#endif

	    if ((stepcols > havecols) && (ldp->li > 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process/procline: linefold\n") ;
#endif

	        f_fold = TRUE ;
	        if (optp->carriage) {
	            rs = bputc(ofp,'\r') ;
	            wlen += rs ;
	        }

	        if (rs >= 0) {
	            rs = bputc(ofp,'\n') ;
	            wlen += rs ;
	        }

	        ldp->li = 0 ;
	        ldp->olenr = pip->linewidth ;
	        ldp->nline += 1 ;

	    } /* end if (broke up a long line) */

/* do we have white-space at a fold boundary? */

	    if (optp->rmwhite && (cl > 0) && f_whitechar && f_fold) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process/procline: remove_char\n") ;
#endif

	        cl = 0 ;
	        stepcols = 0 ;

	    } /* end if ("remove-white" option) */

/* print out some stuff (if we have anything left) */

	    if (cl > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("process/procline: cl=%u cp=>%t<\n",
	                cl,cp,cl) ;
#endif

	        if (optp->carriage && (ldp->li == 0)) {
	            rs = bputc(ofp,'\r') ;
	            wlen += rs ;
	        }

	        if ((pip->lineindent > 0) && f_fold &&
	            (ldp->nline > 0) && (ldp->li == 0)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("process/procline: bwriteblanks() n=%u\n",
	                    pip->lineindent) ;
#endif

	            if (rs >= 0) {
	                rs = bwriteblanks(ofp,pip->lineindent) ;
	                wlen += rs ;
	                ldp->li = rs ;
		    }
	            ldp->olenr = pip->linewidth - ldp->li ;

	        } /* end if (outputting indent) */

	        if (rs >= 0) {

	            f_fold = FALSE ;
	            if (f_badchar) {
	                rs = bputc(ofp,' ') ;
	                wlen += rs ;
	            } else {
	                rs = bwrite(ofp,cp,cl) ;
	                wlen += rs ;
		    }

	            ldp->li += stepcols ;
	            ldp->olenr -= stepcols ;

	        } /* end if (ok) */

	    } /* end if (stuff to print) */

	    lp += tl ;
	    li += tl ;
	    ll -= tl ;

	} /* end while (processing tokens) */

/* perform any necessary copy-down */

	ldp->lo = 0 ;
	if ((rs >= 0) && f_copydown) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process/procline: copy-down cl=%u cp=>%t<\n",
	            cl,cp,cl) ;
#endif

	    ldp->lo = cl ;
	    memmove(ldp->lp,cp,cl) ;

	} /* end if (copy-down) */

/* should we add a final trailing EOL? */

	f = ldp->f_eol || ((! f_eol) && ldp->f_eof) ;
	if ((rs >= 0) && f && (! f_fold)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process/procline: adding EOL\n") ;
#endif

	    ldp->nline = 0 ;
	    ldp->li = 0 ;
	    ldp->olenr = pip->linewidth ;
	    if (optp->carriage) {
	        rs = bputc(ofp,'\r') ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	} /* end if (writing EOL) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process/procline: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


static int nexttoken(cchar sp[],int sl,cchar **rpp)
{
	int	rl = 0 ;
	*rpp = sp ;
	if (sl > 0) {
	    if (! CHAR_TOKSEP(*sp)) {
	        while ((sl > 0) && (! CHAR_TOKSEP(*sp))) {
	            sp += 1 ;
	            sl -= 1 ;
	        }
	    } else {
	        sp += 1 ;
	    }
	    rl = (sp - (*rpp)) ;
	}
	return rl ;
}
/* end subroutine (nexttoken) */


