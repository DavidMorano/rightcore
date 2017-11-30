/* progkeyer */

/* process the input files */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* run-time debug print-outs */
#define	CF_EXTRAWORDS	1		/* extra words? */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a single file.

	Synopsis:

	int progkeyer(pip,ofp,terms,delimiter,ignorechars,fname)
	PROGINFO	*pip ;
	bfile		*ofp ;
	uchar		terms[] ;
	char		delimiter[] ;
	char		ignorechars[] ;
	char		fname[] ;

	Arguments:

	- pip		program information pointer
	- fname		file to process

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<ascii.h>
#include	<char.h>
#include	<eigendb.h>
#include	<localmisc.h>

#include	"xwords.h"
#include	"config.h"
#include	"defs.h"
#include	"keys.h"


/* local defines */

#ifndef	LOWBUFLEN
#define	LOWBUFLEN	NATURALWORDLEN
#endif

#define	MAXEXTRAWORDS	3


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sfword(const char *,int,const char **) ;
extern int	field_word(FIELD *,const uchar *,const char **) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isalnumlatin(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	ignoreline(const char *,int,const char *) ;
static int	procword(PROGINFO *,HDB *,int,cchar *,int) ;


/* local variables */


/* exported subroutines */


int progkeyer(pip,ofp,omp,terms,dbuf,ignorechars,fname)
PROGINFO	*pip ;
bfile		*ofp ;
PTM		*omp ;
uchar		terms[] ;
cchar		*dbuf ;
cchar		*ignorechars ;
cchar		*fname ;
{
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		rs1 ;
	int		dlen = 0 ;
	int		sl, cl, ll ;
	int		c, nk ;
	int		n = 0 ;
	int		entries = 0 ;
	int		f_open = FALSE ;
	int		f_eol ;
	uchar		bterms[32] ;
	const char	*sp ;
	const char	*cp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    int	i ;
	    debugprintf("progkeyer: ent file=%s\n",fname) ;
	    debugprintf("progkeyer: delim=>%s<\n",dbuf) ;
	    debugprintf("progkeyer: terms(%p)\n",terms) ;
	    for (i = 0 ; i < 256 ; i += 1) {
	        if (BATST(terms,i)) {
	            debugprintf("progkeyer: t=%02x\n",i) ;
	        }
	    }
	}
#endif /* CF_DEBUG */

	if (pip->f.optbible) {
	    memcpy(bterms,terms,32) ;
	    BACLR(bterms,':') ;
	} /* end if (special bible processing) */

	if (dbuf != NULL) {
	    dlen = strlen(dbuf) ;
	}

/* open the file that we are supposed to process */

	if ((fname[0] == '-') || (fname[0] == '\0')) fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    FIELD	fsb ;
	    HDB		keydb ;
	    offset_t	offset = 0 ;
	    offset_t	recoff = 0 ;
	    uint	reclen = 0 ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		lo = 0 ;
	    int		hashsize = 20 ;
	    int		f_ent = FALSE ;
	    int		f_start = pip->f.wholefile ;
	    int		f_finish = FALSE ;
	    char	lbuf[LINEBUFLEN + 1], *lp ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progkeyer: file-processing\n") ;
#endif

/* figure a default hash DB size based on the input file length */

	    if (pip->f.wholefile) {
	        hashsize = 1000 ;
	        if ((rs = bsize(ifp)) > 0) {
	            hashsize = (rs / 6) ;
	        }
	    } /* end if (whole-file) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progkeyer: hashsize=%u\n",hashsize) ;
#endif

/* go to it, read the file line by line */

	    while (rs >= 0) {
	        rs = breadline(ifp,(lbuf + lo),(llen - lo)) ;
	        if (rs < 0) break ;
	        len = (lo + rs) ;
	        if (len == 0) break ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progkeyer: line> %t", lbuf,len) ;
#endif

/* we ignore lo...ng lines entirely, but we try to resynchronize up */

	        lp = lbuf ;
	        ll = len ;
	        f_eol = (lp[ll - 1] == '\n') ;
	        if (! f_eol) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkeyer: ignoring long line\n") ;
#endif

	            offset += len ;
	            while (((c = bgetc(ifp)) != SR_EOF) && (c != '\n')) {
	                offset += 1 ;
	                recoff += 1 ;
	            } /* end while */

	            lo = 0 ;
	            continue ;

	        } /* end if (discarding extra line input) */

	        lp[--ll] = '\0' ;

/* figure out where we start and/or end an entry */

	        if (! pip->f.wholefile) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("progkeyer: NWF f_ent=%u\n",f_ent) ;
	                debugprintf("progkeyer: delim=>%s<\n",dbuf) ;
	            }
#endif
	            if ((dlen > 0) && (dbuf != NULL)) {
	                if (dbuf[0] == '\0') {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: delimit every line\n") ;
#endif
	                    f_start = TRUE ;
	                    f_finish = TRUE ;
	                } else if (CHAR_ISWHITE(dbuf[0])) {
	                    if ((cl = sfshrink(lp,ll,&cp)) == 0) {
	                        if (f_ent) f_finish = TRUE ;
	                    } else {
	                        if (! f_ent) f_start = TRUE ;
	                    } /* end if */
	                } else {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: specified delimiter\n") ;
#endif
	                    if (strncmp(lp,dbuf,dlen) == 0) {
#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("progkeyer: got delimiter\n") ;
#endif
	                        if (f_ent)
	                            f_finish = TRUE ;
	                    } else {
#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("progkeyer: regular line\n") ;
#endif
	                        if (! f_ent)
	                            f_start = TRUE ;
	                    } /* end if */
	                } /* end if (delimiter cascade) */
	            } /* end if (non-null) */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("progkeyer: mid rs=%d\n",rs) ;
	                debugprintf("progkeyer: f_start=%u f_finish=%u\n",
	                    f_start,f_finish) ;
	            }
#endif

	            if (f_finish) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkeyer: finishing off entry\n") ;
#endif

	                f_ent = FALSE ;
	                f_finish = FALSE ;
	                if (n > 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: closing a DB\n") ;
#endif

	                    f_open = FALSE ;
	                    reclen = (uint) ((offset - recoff) & UINT_MAX) ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: i reclen=%u\n",
	                            reclen) ;
#endif

	                    rs = keys_end(pip,&keydb,ofp,omp,
	                        fname,recoff,reclen) ;
	                    nk = rs ;
	                    if (nk > 0)
	                        entries += 1 ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: keys_end() rs=%s\n",
				rs) ;
#endif

	                } /* end if */

	            } /* end if (finishing entry) */

	        } /* end if (not whole file -- determining entry boundary) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progkeyer: mid2 rs=%d f_start=%u\n",
			rs,f_start) ;
#endif

	        if ((rs >= 0) && f_start) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkeyer: starting offset=%ld\n",
	                    offset) ;
#endif

	            f_start = FALSE ;
	            f_ent = TRUE ;

	            n = 0 ;
	            reclen = 0 ;
	            recoff = offset ;
	            if (! f_open) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkeyer: opening a DB\n") ;
#endif

	                f_open = TRUE ;
	                rs = keys_begin(pip,&keydb,hashsize) ;

	            } /* end if (opening keys DB) */

	        } /* end if (starting entry) */

/* process this current input line if we are supposed to */

	        if ((rs >= 0) && (ll >= pip->minwordlen) && f_ent && 
	            (! ignoreline(lp,ll,ignorechars))) {

	            if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	                int	fl ;
			int	ch ;
	                int	f_first = FALSE ;
	                cchar	*fp ;

	                if (pip->f.optbible) {

	                    fl = field_get(&fsb,bterms,&fp) ;

	                    while ((fl > 0) && (fp[fl - 1] == ':')) {
	                        fl -= 1 ;
	                    }

	                    f_first = (fl > 0) ;

	                } /* end if (special bible processing) */

/* loop on parsing words (which become keys) from the input line */

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkeyer: f_first=%u\n",f_first) ;
#endif

	                while ((rs >= 0) &&
	                    (f_first || 
				((fl = field_word(&fsb,terms,&fp)) >= 0))) {

	                    char	lowbuf[LOWBUFLEN + 1] ;

	                    f_first = FALSE ;

/* remove apostrophes (single quotes) from the leading edge */

			    if (fl && (fp[0] == CH_SQUOTE)) {
				fp += 1 ;
				fl -= 1 ;
			    }

			    if (fl == 0) continue ;
			    ch = MKCHAR(fp[0]) ;
	                    if (! isalnumlatin(ch)) continue ;
	                    if (fl > NATURALWORDLEN) continue ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: word=%t\n",fp,fl) ;
#endif

	                    sp = fp ;
	                    sl = fl ;
	                    if (hasuc(fp,fl)) {
	                        int	ml = MIN(LOWBUFLEN,fl) ;
	                        sl = strwcpylc(lowbuf,fp,ml) - lowbuf ;
	                        sp = lowbuf ;
	                    }

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: lowercase word=%t\n",
	                            sp,sl) ;
#endif

/* remove possible trailing single quote */

	                    cl = sfword(sp,sl,&cp) ;

	                    if (cl <= 0) continue ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progkeyer: wl=%u wp=>%t<\n",
	                            cl,cp,cl) ;
#endif

#if	CF_EXTRAWORDS
	                    {
	                        XWORDS	xw ;
	                        if ((rs = xwords_start(&xw,cp,cl)) >= 0) {
	                            int	i = 0 ;
	                            while ((rs >= 0) &&
	                                ((sl = xwords_get(&xw,i++,&sp)) > 0)) {
	                                rs = procword(pip,&keydb,n,sp,sl) ;
	                                if (rs > 0) n += 1 ;
	                            } /* end while */
	                            xwords_finish(&xw) ;
	                        } /* end if */
	                    } /* end block */
#else /* CF_EXTRAWORDS */

	                    rs = procword(pip,&keydb,n,cp,cl) ;
	                    if (rs > 0)
	                        n += 1 ;

#endif /* CF_EXTRAWORDS */

	                    if (rs < 0) break ;
	                } /* end while (getting word fields from the line) */

	                field_finish(&fsb) ;
	            } /* end if (field) */

	        } /* end if (we were in an entry) */

	        offset += len ;
	        reclen += len ;
	        if (rs < 0) break ;
	    } /* end while (looping reading lines) */

/* write out (finish off) the last (or only) entry */

	    if (f_open) {

	        f_open = FALSE ;
	        reclen = (uint) ((offset - recoff) & UINT_MAX) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progkeyer: f keys_end() reclen=%u\n",
	                reclen) ;
#endif

	        rs1 = keys_end(pip,&keydb,ofp,omp,fname,recoff,reclen) ;
	        nk = rs1 ;
	        if (rs >= 0) rs = rs1 ;
	        if (nk > 0)
	            entries += 1 ;

	    } /* end if */

	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-input) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkeyer: ret rs=%d entries=%u\n",rs,entries) ;
#endif

	return (rs >= 0) ? entries : rs ;
}
/* end subroutine (progkeyer) */


/* local subroutines */


/* which input lines are supposed to be ignored? */
static int ignoreline(cchar *lbuf,int ll,cchar *ignorechars)
{
	int	f = FALSE ;
	if ((ignorechars != NULL) && (lbuf[0] == '%')) {
	    if (ll < 2) f = TRUE ;
	    if ((! f) && (strchr(ignorechars,lbuf[1]) != NULL)) f = TRUE ;
	} /* end if */
	return f ;
}
/* end subroutine (ignoreline) */


/* process a word */
static int procword(PROGINFO *pip,HDB *keydbp,int n,cchar *wp,int wl)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if ((pip->maxkeys == 0) || (n < pip->maxkeys)) {
	    if ((wl > 0) && (wl <= NATURALWORDLEN)) {
	        if (wl >= pip->minwordlen) {
	            EIGENDB	*edbp = &pip->eigendb ;
	            const int	rsn = SR_NOTFOUND ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                if (wl > 0) {
	                    debugprintf("progkeyer/procword: key=%t\n",wp,wl) ;
	                } else {
	                    debugprintf("progkeyer/procword: zero length\n") ;
	                }
	            }
#endif /* CF_DEBUG */

/* check if this word is in the eigenword database */

	            if ((rs = eigendb_exists(edbp,wp,wl)) >= 0) {
	                f = FALSE ;
	            } else if (rs == rsn) {
	                if ((pip->maxwordlen > 0) && (wl > pip->maxwordlen)) {
	                    wl = pip->maxwordlen ;
	                }
	                rs = keys_add(pip,keydbp,wp,wl) ;
	                f = (rs > 0) ;
	            }

	        } /* end if (minimum length) */
	    } /* end if (word length within range) */
	} /* end if (did not reach maximum key limit) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkeyer/procword: ret=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procword) */


