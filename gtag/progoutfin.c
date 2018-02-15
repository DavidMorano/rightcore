/* progout (Program Output) */
/* lang=C++11 */

/* part of the MMCITE program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	1		/* run-time debug print-outs */


/* revision history:

	= 1996-02-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine processes the temporary text file along with the
	accumulated citation and bibliographical information into the final
	text output.


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
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"citedb.h"
#include	"bdb.h"
#include	"keytracker.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif


/* declarations */

extern "C" int	progoutfin(PROGINFO *,BDB *,CITEDB *,cchar *) ;


/* external subroutines */

extern "C" int	matstr(const char **,const char *,int) ;
extern "C" int	bprintlns(bfile *,int,cchar *,int) ;

extern "C" int	progoutbib(PROGINFO *,bfile *,BDB_ENT *) ;
extern "C" int	isMacro(cchar *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugprintf(cchar *,...) ;
extern "C" int	debugprinthex(cchar *,int,cchar *,int) ;
extern "C" int	strlinelen(cchar *,int,int) ;
#endif

extern "C" char	*strwcpy(char *,cchar *,int) ;


/* forward references */

inline int	isWhite(int) ;


/* external variables */


/* local variables */


/* exported subroutines */


int progoutfin(PROGINFO *pip,BDB *bdbp,CITEDB *cdbp,cchar *ofname)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs, rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progout: ent\n") ;
#endif

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0644)) >= 0) {
	    CITEDB_ENT	ce ;
	    CITEDB_CUR	cur ;
	    BDB_ENT	be ;
	    bfile	*efp = (bfile *) pip->efp ;
	    bfile	*tfp = &pip->tf.tfile ;
	    uint	roff = 0 ;
	    const int	llen = LINEBUFLEN ;
	    int		clen ;
	    int		nblock = 0 ;
	    int		len, ll ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    cchar	*lp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    if ((rs = citedb_curbegin(cdbp,&cur)) >= 0) {
		const int	biblen = BIBBUFLEN ;
		char		bibbuf[BIBBUFLEN + 1] ;

	        while ((rs1 = citedb_enum(cdbp,&cur,&ce)) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("progout: citation off=%u\n",ce.coff) ;
	                debugprintf("progout: roff=%u\n",roff) ;
	                debugprintf("progout: nblock=%u\n",nblock) ;
	            }
#endif

	            if (ce.coff > roff) {

/* copy over the file chunk before the citation (if any) */

	                rs = breadline(tfp,lbuf,llen) ;
	                len = rs ;
	                roff += rs ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3)) {
	                    debugprintf("progout: breadline() rs=%d\n",rs) ;
	                    debugprintf("progout: nblock=%u\n",nblock) ;
			}
#endif
	                if (rs >= 0) {

	                    lp = lbuf ;
	                    ll = len ;
	                    while (ll && isWhite(*lp)) {
	                        lp += 1 ;
	                        ll -= 1 ;
	                    }

	                    if (ll > 0) {

	                        if ((nblock > 0) && isMacro(lp,ll)) {
	                            rs = bprintf(ofp,"\\&") ;
	                            wlen += rs ;
	                        }

	                        if (rs >= 0) {
	                            rs = bwrite(ofp,lp,ll) ;
	                            wlen += rs ;
	                        }

	                    } /* end if */

	                } /* end if (read a line) */

	                if ((rs >= 0) && (ce.coff > roff)) {
	                    clen = ce.coff - roff ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("progout: bcopyblock() clen=%u\n",
	                            clen) ;
#endif
	                    rs = bcopyblock(tfp,ofp,clen) ;
	                    wlen += rs ;
	                    roff += rs ;
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("progout: bcopyblock() rs=%d\n",
	                            rs) ;
#endif
	                } /* end if */

	                if (rs < 0) break ;
	            } /* end if (had some text to write) */

/* process the citation */

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progout: rs=%d ci=%u cn=%u cs=%s\n",
	                    rs,ce.ci,ce.n,ce.citestr) ;
#endif

	            if (rs >= 0) {
	                if (ce.ci == 1) {

	                    rs = bprintf(ofp,"\\*(Rf\n") ;
	                    wlen += rs ;
	                    if (rs >= 0) {
	                        fmt = ".\\\"_ citation> %s\n" ;
	                        rs = bprintf(ofp,fmt,ce.citekey) ;
	                        wlen += rs ;
	                    }

	                    if (rs >= 0) {
	                        fmt = ".RS %s\n" ;
	                        if (ce.citestr[0] == '\0') fmt = ".RS\n" ;
	                        rs = bprintf(ofp,fmt,ce.citestr) ;
	                        wlen += rs ;
	                    }

	                    if (rs >= 0) {

#if	CF_DEBUG
	                        if (DEBUGLEVEL(3))
	                            debugprintf("progout: citekey=%s\n",
	                                ce.citekey) ;
#endif

	                        rs1 = bdb_query(bdbp,ce.citekey,&be,
	                            bibbuf,biblen) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(3))
	                            debugprintf("progout: bdb_query() rs=%d\n",
	                                rs1) ;
#endif

	                        if (rs1 >= 0) {

#if	CF_DEBUG
	                            if (DEBUGLEVEL(3)) {
	                                int	i = 0 ;
	                                debugprintf("progout: query=%s\n",
	                                    ce.citekey) ;
	                                while (be.keyvals[i][0] != NULL) {
	                                    debugprintf("progout: k=%s\n",
	                                        be.keyvals[i][0]) ;
	                                    debugprintf("progout: v=%t\n",
	                                        be.keyvals[i][1],
	                                        strnlen(be.keyvals[i][1],40)) ;
	                                    i += 1 ;
	                                } /* end while */
	                            }
#endif /* CF_DEBUG */

	                            rs = progoutbib(pip,ofp,&be) ;
	                            wlen += rs ;

#if	CF_DEBUG
	                            if (DEBUGLEVEL(3))
	                                debugprintf("progout: "
	                                    "progoutbib() rs=%d\n", rs) ;
#endif

	                        } else if (rs1 == SR_NOTFOUND) {

	                            fmt = "%s: not found citation=%s\n" ;
	                            bprintf(efp,fmt,pn,ce.citekey) ;

	                            fmt = "** citation not found **\n" ;
	                            rs = bprintf(ofp,fmt) ;
	                            wlen += rs ;

	                        } else if ((rs1 == SR_NOTUNIQ) && pip->f.uniq) {

	                            fmt = "%s: not unique citation=%s\n" ;
	                            rs = SR_NOTUNIQ ;
	                            bprintf(efp,fmt,pn,ce.citekey) ;

	                        } else {
	                            rs = rs1 ;
				}

	                    } /* end if */

	                    if (rs >= 0) {
	                        rs = bprintf(ofp,".RF\n") ;
	                        wlen += rs ;
	                    }

	                } else {

	                    rs = bprintf(ofp,"\\*(%s\n", ce.citestr) ;
	                    wlen += rs ;
	                    if (rs >= 0) {
	                        fmt = ".\\\"_ citation> %s\n" ;
	                        rs = bprintf(ofp,fmt,ce.citekey) ;
	                        wlen += rs ;
	                    }

	                } /* end if */
	            } /* end if (processing citation) */

	            nblock += 1 ;

	            if (rs < 0) break ;
	        } /* end while (looping on citations) */
		if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

	        rs1 = citedb_curend(cdbp,&cur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */

/* handle the final text block (if any) */

	    if (rs >= 0) {
	        rs = breadline(tfp,lbuf,llen) ;
	        len = rs ;
	        roff += rs ;

	        if (rs >= 0) {

	            lp = lbuf ;
	            ll = len ;
	            while (ll && isWhite(*lp)) {
	                lp += 1 ;
	                ll -= 1 ;
	            }

	            if (ll > 0) {

#ifdef	COMMENT
	                if (isMacro(lp,ll)) {
	                    rs = bprintf(ofp,"\\&") ;
	                    wlen += rs ;
	                }
#endif /* COMMENT */

	                if (rs >= 0) {
	                    rs = bwrite(ofp,lp,ll) ;
	                    wlen += rs ;
	                }

	            } /* end if */

	        } /* end if */

	        if ((rs >= 0) && (len > 0)) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progout: bcopyblock() all\n") ;
#endif
	            rs = bcopyblock(tfp,ofp,-1) ;
	            wlen += rs ;
	            roff += rs ;
	        }

	    } /* end if (following text block) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (output-file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progout: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progout) */


/* local subroutines */


inline int isWhite(int ch) {
	return CHAR_ISWHITE(ch) || (ch == '\n') ;
}
/* end subroutine (isWhite) */


