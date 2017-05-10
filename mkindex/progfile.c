/* progfile */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* switchable debug print-outs */
#define	CF_LINE		1		/* use line number instead of offset */


/* revision history:

	= 1994-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a single file.

	Synopsis:

	int progfile(pip,hasha,nhash,mfp,nfp,fname)
	struct proginfo	*pip ;
	uint		hasha[] ;
	int		nhash ;
	MEMFILE		*mfp ;
	bfile		*nfp ;
	const char	fname[] ;

	Arguments:

	- pip		program information pointer
	- fname	file to process

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<field.h>
#include	<char.h>
#include	<localmisc.h>

#include	"memfile.h"
#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;

extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	ffbsi(uint) ;
extern int	iceil(int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylow(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	procdata(struct proginfo *,char *,int,uint *,int,
			MEMFILE *,bfile *, const char *) ;

static int	bwasteline(bfile *,char *,int) ;


/* local variables */


/* exported subroutines */


int progfile(pip,hasha,nhash,mfp,nfp,fname)
struct proginfo	*pip ;
uint		hasha[] ;
int		nhash ;
MEMFILE		*mfp ;		/* post MEMFILE */
bfile		*nfp ;		/* name file pointer */
const char	fname[] ;
{
	const int	size = (pip->pagesize * 4) ;

	int	rs ;
	int	llen = (size-1) ;

	char	*lbuf ;

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_valloc(size,&lbuf)) >= 0) {

	    rs = procdata(pip,lbuf,llen,hasha,nhash,mfp,nfp,fname) ;

	    uc_free(lbuf) ;
	} /* end if */

	return rs ;
}
/* end subroutine (progdata) */


/* local subroutines */


static int procdata(pip,lbuf,llen,hasha,nhash,mfp,nfp,fname)
struct proginfo	*pip ;
char		lbuf[] ;
int		llen ;
uint		hasha[] ;
int		nhash ;
MEMFILE		*mfp ;		/* post MEMFILE */
bfile		*nfp ;		/* name file pointer */
const char	fname[] ;
{
	struct postentry	*posta, e ;

	bfile		ifile, *ifp = &ifile ;

	offset_t	offset, noff, poff ;

	uint	hi, himask ;
	uint	pi ;

	int	rs, len, nlen ;
	int	line = 0 ;
	int	c = 0 ;
	int	sl, cl, el ;
	int	li ;
	int	shift ;

	const char	*sp, *cp, *ep ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("procdata: entered file=%s\n",fname) ;
	    debugprintf("procdata: nhash=%u\n",nhash) ;
	}
#endif

	len = iceil(sizeof(struct postentry),sizeof(int)) ;

	himask = (nhash - 1) ;
	shift = ffbsi(len) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procdata: himask=%08x\n",himask) ;
#endif

/* open the file that we are supposed to process */

	if (fname[0] == '-') fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {

/* get the starting offset of the posting file */

	memfile_tell(mfp,&poff) ;

	memfile_buf(mfp,&posta) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("procdata: initial posta=%p\n",posta) ;
#endif

/* get the starting offset of the name-file */

	rs = btell(nfp,&noff) ;

/* write the first posting entry as NULL */

	if (rs >= 0) {
	e.noff = 0 ;
	e.next = 0 ;
	rs = memfile_write(mfp,&e,sizeof(struct postentry)) ;
	}

/* go to it, read the file line by line */

	ep = NULL ;
	el = 0 ;
	li = 0 ;
	if (rs >= 0) {
	    int	f_bol = TRUE ;
	    int	f_eol ;
	while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	    len = rs ;

	    f_eol = (lbuf[len - 1] == '\n') ;
	    if (f_eol) lbuf[--len] = '\0' ;

/* read the file name */

	    sp = lbuf ;
	    sl = len ;

	    if (f_bol && (sl > 0)) {

	    if (lbuf[0] == '-') continue ;
	    if (lbuf[0] == '#') continue ;

	        cp = strnchr(sp,sl,'\t') ;

	        if ((cp == NULL) && (! f_eol)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdata: wasting line\n") ;
#endif

	            bwasteline(ifp,lbuf,LINEBUFLEN) ;

	            f_bol = TRUE ;
	            continue ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("procdata: filespec=%t\n",
	                lbuf,(cp - lbuf)) ;
#endif

	        rs = bwrite(nfp,lbuf,(cp - lbuf)) ;

	        if (rs > 0) {
	            nlen = (rs + 1) ;
	            bputc(nfp,'\n') ;
	        }

#if	CF_LINE
	        e.noff = line ;
#else
	        e.noff = noff ;
#endif /* CF_LINE */

	        sp = cp + 1 ;
	        sl = lbuf + len - sp ;

	    } /* end if (BOL) */

	    if (rs < 0)
	        break ;

	    ep = NULL ;
	    el = 0 ;
	    if ((! f_eol) && (sl > 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("procdata: carryover check\n") ;
#endif

	        if (! CHAR_ISWHITE(sp[sl - 1])) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdata: end is not white >%c<\n",
				sp[sl - 1]) ;
#endif

	            cp = sp ;
	            cl = sl ;
	            while ((cl > 0) && (! CHAR_ISWHITE(cp[cl - 1])))
	                cl -= 1 ;

	            ep = cp + cl ;
	            el = sl - cl ;

	            sl -= el ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdata: carryover=>%t<\n",ep,el) ;
#endif

	        } /* end if (needed to carry stuff forward) */

	    } /* end if (not EOL) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("procdata: about to field loop for sl=%u\n",sl) ;
#endif

	    while ((cl = nextfield(sp,sl,&cp)) > 0) {
	        int	f_write ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("procdata: word=%t\n",cp,cl) ;
#endif

	        hi = hashelf(cp,cl) & himask ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("procdata: hi=%08x\n",hi) ;
#endif

	        pi = hasha[hi] ;

/* search if this posting pointer already points to our 'noff' value */

	        f_write = FALSE ;
	        if (pi != 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdata: hash collision pi=%u\n",pi) ;
#endif

	            while (posta[pi].noff != noff) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(5))
	                    debugprintf("procdata: looping on posta pi=%u\n",
				pi) ;
#endif

	                if (posta[pi].next == 0)
	                    break ;

	                pi = posta[pi].next ;

	            } /* end while */

	            if (posta[pi].noff != noff) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(5)) {
	                    uint	filelen, alloclen ;
	                    debugprintf("procdata: didn't find our noff=%lu \n",
	                        noff) ;
	                    debugprintf("procdata: posta=%p pi=%u\n",posta,pi) ;
	                    filelen = memfile_len(mfp) ;
	                    alloclen = memfile_allocation(mfp) ;
	                    debugprintf("procdata: filelen=%u allocation=%u\n",
	                        filelen,alloclen) ;
	                    debugprintf("procdata: ai=%u\n",(pi << shift)) ;
	                }
#endif /* CF_DEBUG */

	                f_write = TRUE ;
	                posta[pi].next = (poff >> shift) ;

	            } /* end if (didn't find) */

#if	CF_DEBUG
	            if (DEBUGLEVEL(5) && (! f_write)) {
	                debugprintf("procdata: looped and found our noff !\n") ;
	            }
#endif /* CF_DEBUGS */

	        } else {

	            f_write = TRUE ;
	            hasha[hi] = (poff >> shift) ;

	        } /* end if */

	        if (f_write) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdata: need a post write\n") ;
#endif

	            e.next = 0 ;
	            rs = memfile_write(mfp,&e,sizeof(struct postentry)) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdata: memfile_write() rs=%d\n",rs) ;
#endif

	            if (rs < 0)
	                break ;

	            c += 1 ;
	            poff += rs ;
	            memfile_buf(mfp,&posta) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("procdata: new posta=%p\n",posta) ;
#endif

	        } /* end if (needed to write) */

	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;

	    } /* end while (reading keys) */

	    li = 0 ;
	    if (ep != NULL) {

	        li = el ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("procdata: next carryover=>%t<\n",ep,el) ;
#endif

	        strncpy(lbuf,ep,el) ;

	    } /* end if (carrying forward) */

	    noff += nlen ;
	    line += 1 ;
	    f_bol = f_eol ;
	    if (rs < 0) break ;
	} /* end while (reading lines) */
	} /* end if */

	bclose(ifp) ;
	} /* end if (input-open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) 
	    debugprintf("procdata: ret rs=%d c=%u\n",rs, c) ;
#endif

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdata) */


static int bwasteline(fp,lbuf,llen)
bfile		*fp ;
char		lbuf[] ;
int		llen ;
{
	int	rs ;
	int	len ;

	while ((rs = breadline(fp,lbuf,llen)) > 0) {
	    len = rs ;
	    if (lbuf[len - 1] == '\n') break ;
	} /* end while */

	return rs ;
}
/* end subroutine (bwasteline) */



