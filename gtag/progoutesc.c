/* progoutesc */

/* part of the GTAG program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1996-02-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine processes the temporary text file along with the
        accumulated citation and bibliographical information into the final text
        output.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"tagtrack.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#ifndef	LINEFOLDLEN
#define	LINEFOLDLEN	76
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	bprintlns(bfile *,int,cchar *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthex(cchar *,int,cchar *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* forward references */

inline int	isWhite(int) ;


/* external variables */


/* local variables */


/* exported subroutines */


int progoutesc(PROGINFO *pip,TAGTRACK *ttp,cchar *ofname)
{
	bfile		outfile, *ofp = &outfile ;
	bfile		*tfp = &pip->tf.tfile ;
	uint		roff = 0 ;
	int		rs, rs1 ;
	int		len ;
	int		clen ;
	int		nblock = 0 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutesc: ent\n") ;
#endif

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0644)) >= 0) {
	    TAGTRACK_ENT	te ;
	    TAGTRACK_CUR	cur ;
	    const int		llen = LINEBUFLEN ;
	    int			ll ;
	    cchar		*pn = pip->progname ;
	    cchar		*fmt ;
	    const char		*lp ;
	    char		lbuf[LINEBUFLEN + 1] ;

	    if ((rs = tagtrack_curbegin(ttp,&cur)) >= 0) {

	        while (rs >= 0) {
		    rs1 = tagtrack_enum(ttp,&cur,&te) ;
		    if (rs1 == SR_NOTFOUND) break ;
		    rs = rs1 ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("progoutesc: roff=%u\n",roff) ;
	                debugprintf("progoutesc: nblock=%u\n",nblock) ;
	                debugprintf("progoutesc: tag-fi=%u\n",te.fi) ;
	                debugprintf("progoutesc: tag-eoff=%u\n",te.eoff) ;
	                debugprintf("progoutesc: tag-elen=%d\n",te.elen) ;
	                debugprintf("progoutesc: tag-val=%u\n",te.v) ;
	            }
#endif /* CF_DEBUG */

	            if ((rs >= 0) && (te.eoff > roff)) {

/* copy over the file chunk before the citation (if any) */

	                if ((rs = breadline(tfp,lbuf,llen)) >= 0) {
	                    len = rs ;
	                    roff += rs ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("progoutesc: breadline() rs=%d\n",rs) ;
#endif

	                    lp = lbuf ;
	                    ll = len ;
	                    while (ll && isWhite(*lp)) {
	                        lp += 1 ;
	                        ll -= 1 ;
	                    }

	                    if (ll > 0) {

	                        if ((nblock > 0) && isMacro(lp,ll)) {
	                            rs = bprint(ofp,"\\&",-1) ;
	                            wlen += rs ;
	                        }

	                        if (rs >= 0) {
	                            rs = bwrite(ofp,lp,ll) ;
	                            wlen += rs ;
	                        }

	                    } /* end if */

	                } /* end if (read a line) */

	                if ((rs >= 0) && (te.eoff > roff)) {
	                    clen = (te.eoff - roff) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("progoutesc: "
				    "bcopyblock() clen=%u\n", clen) ;
#endif
	                    rs = bcopyblock(tfp,ofp,clen) ;
	                    wlen += rs ;
	                    roff += rs ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("progoutesc: "
				    "bcopyblock() rs=%d\n", rs) ;
#endif
	                } /* end if */

	            } /* end if (had some text to write) */

/* process the citation */

	            if (rs >= 0) {
			if (te.v <= 0) {
			    fmt = "%s: un-tagged reference\n" ;
			    bprintf(pip->efp,fmt,pn) ;
			}
			if ((rs = bseek(tfp,te.elen,SEEK_CUR)) >= 0) {
			    fmt = "%u" ;
	                    rs = bprintf(ofp,fmt,te.v) ;
	                    wlen += rs ;
			}
		    } /* end if */

	            nblock += 1 ;
	        } /* end while (looping on citations) */

	        tagtrack_curend(ttp,&cur) ;
	    } /* end if (cursor) */

/* handle the final text block (if any) */

	    if (rs >= 0) {
	        if ((rs = breadline(tfp,lbuf,llen)) > 0) {
	            len = rs ;
	            roff += rs ;

	            lp = lbuf ;
	            ll = len ;
	            while (ll && isWhite(*lp)) {
	                lp += 1 ;
	                ll -= 1 ;
	            }

	            if (ll > 0) {

	                if (isMacro(lp,ll)) {
	                    rs = bprint(ofp,"\\&",-1) ;
	                    wlen += rs ;
	                }

	                if (rs >= 0) {
	                    rs = bwrite(ofp,lp,ll) ;
	                    wlen += rs ;
	                }

	            } /* end if */

	        if (rs >= 0) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progoutesc: bcopyblock() all\n") ;
#endif
	            rs = bcopyblock(tfp,ofp,-1) ;
	            wlen += rs ;
	            roff += rs ;
	        }

	        } /* end if (breadline) */
	    } /* end if (following text block) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (output-file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progoutesc: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progoutesc) */


/* local subroutines */


inline int isWhite(int ch) {
	return CHAR_ISWHITE(ch) || (ch == '\n') ;
}
/* end subroutine (isWhite) */


