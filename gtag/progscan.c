/* progscan */

/* process a file by inserting bibliographical references */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_MULTICITE	1		/* allow multiple citations */


/* revision history:

	= 1992-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 1998-09-01, David A­D­ Morano
        This subroutine was modified to process the way MMCITE does citation. It
        used to use the old GNU 'lookbib' program in addition to the (old)
        standard UNIX version. But neither of these are used now. Straight out
        citeation keywrd lookup is done directly in a BIB database (files of
        which are suffixed 'rbd').

*/

/* Copyright © 1992,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes a file by looking up and inserting the
        bibliographical references into the text. All input is copied to the
        output with the addition of the bibliographical references.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"tagtrack.h"


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	sicite(const char *,int,const char *,int) ;
extern int	silbrace(const char *,int) ;

extern int	bprinter(bfile *,int,const char *,int) ;
extern int	findbibfile(struct proginfo *,PARAMOPT *,const char *,char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct procinfo {
	PARAMOPT	*app ;
	TAGTRACK	*ttp ;
	int		fi ;
} ;

struct mbdinfo {
	const char	*pp ;
	const char	*rp ;
	const char	*kp ;
	uint		loff ;
	int		pl ;
	int		rl ;
	int		kl ;
} ;


/* forward references */

#ifdef	COMMENT
static int mbdescape(struct proginfo *,struct mbdinfo *,uint,const char *,int) ;
#endif


/* local variables */


/* exported subroutines */


int progscan(pip,app,ttp,fi,fname)
struct proginfo	*pip ;
PARAMOPT	*app ;
TAGTRACK	*ttp ;
int		fi ;
const char	fname[] ;
{
	struct procinfo	pc ;

	bfile	infile, *ifp = &infile ;
	bfile	*tfp = &pip->tf.tfile ;

	int	rs ;
	int	tlen = 0 ;

	const char	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progscan: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	memset(&pc,0,sizeof(struct procinfo)) ;
	pc.app = app ;
	pc.ttp = ttp ;

/* enter the filename into storage for possible need later (on error) */

	cp = fname ;
	if (cp[0] == '-')
	    cp = "*STDIN*" ;

/* proceed to open the file */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progscan: open file\n") ;
#endif

	if (fname[0] == '-') fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    uint	foff = pip->tf.tlen ;
	    int		ll ;
	    int		f_bol, f_eol ;
	    const char	*lp ;
	    char	lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progscan: while-above\n") ;
#endif

	    f_bol = TRUE ;
	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        uint	loff = (foff+tlen) ;
	        int	len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("progscan: begin loff=%u\n",loff) ;
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progscan: line=>%t<\n",lbuf,
	                ((lbuf[len - 1] == '\n') ? (len - 1) : len)) ;
#endif

/* check for macros and escapes */

	        lp = lbuf ;
	        ll = (f_eol) ? (len - 1) : len ;
	        if (f_bol) {
		    if ((rs = tagtrack_scanline(ttp,fi,loff,lp,ll)) > 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progscan: got a macro li=%d\n",rs) ;
#endif

	                rs = bprintline(tfp,".\\\" TAG\n",-1) ;
		        tlen += rs ;
		    } else if (rs == SR_INVALID) {
			const char	*pn = pip->progname ;
			const char	*fmt ;
		        fmt = "%s: same label for multiple tags (%d)\n" ;
		        bprintf(pip->efp,fmt,pn,rs) ;
	                rs = bprinter(tfp,f_eol,lp,ll) ;
	                tlen += rs ;
		    } else if (rs >= 0) {
	                rs = bprinter(tfp,f_eol,lp,ll) ;
	                tlen += rs ;
		    } else if (rs < 0) {
			const char	*pn = pip->progname ;
			const char	*fmt ;
		        fmt = "%s: error in scaning file=%d (%d)\n" ;
		        bprintf(pip->efp,fmt,pn,fi,rs) ;
		    }
	        } else {
	            rs = bprinter(tfp,f_eol,lp,ll) ;
	            tlen += rs ;
	        } /* end if (specialized processing) */

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading input lines) */

	    bclose(ifp) ;
	} /* end if (file-open) */
	pip->tf.tlen += tlen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progscan: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (progscan) */


/* local subroutines */


#ifdef	COMMENT
static int mbdescape(pip,ip,loff,lp,ll)
struct proginfo	*pip ;
struct mbdinfo	*ip ;
uint		loff ;
const char	*lp ;
int		ll ;
{
	const int	el = strlen(BIBESCAPE) ;

	int	sl, cl ;
	int	si ;
	int	f = FALSE ;

	const char	*tp ;
	const char	*sp, *cp ;


	memset(ip,0,sizeof(struct mbdinfo)) ;
	ip->loff = loff ;
	ip->pp = lp ;
	ip->pl = ll ;
	ip->kp = NULL ;
	ip->kl = 0 ;
	ip->rp = lp ;
	ip->rl = ll ;
	if ((si = sicite(lp,ll,BIBESCAPE,el)) >= 0) {
	    ip->pl = si ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progscan/mbdescape: pl=%u\n",ip->pl) ;
#endif

	    sp = (lp + (si + el + 1)) ;
	    sl = ll - (si + el + 1) ;
	    if ((si = silbrace(sp,sl)) >= 0) {

	        cp = (sp + si + 1) ;
	        cl = sl - (cp - sp) ;
	        tp = strnchr(cp,cl,CH_RBRACE) ;

	        if (tp != NULL) {

	            f = TRUE ;

#if	CF_MULTICITE
	            ip->kp = cp ;
	            ip->kl = (tp - cp) ;
#else
	            ip->kl = nextfield(cp,(tp - cp),&ip->kp) ;
#endif /* CF_MULTICITE */

	            ip->rp = (tp + 1) ;
	            ip->rl = sl - ((tp + 1) - sp) ;

	        } else
	            f = FALSE ;

	    } /* end if (open brace) */

	} /* end if (possible escape) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progscan/mbdescape: ret f=%u\n",f) ;
#endif

	return f ;
}
/* end subroutine (mbdescape) */
#endif /* COMMENT */


