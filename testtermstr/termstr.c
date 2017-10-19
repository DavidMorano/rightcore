/* termstr */

/* object to manage terminal database strings */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-06-24, David A­D­ Morano
        I rewrote this from scratch. The previous version of this program was a
        hack.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int termstr_start(op,termtype)
	TERMSTR		*op ;
	const char	termtype[] ;

	Arguments:

	op		object pointer
	termtype	terminal type as a descriptive string

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<ansigr.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"termstr.h"


/* local defines */

#define	TERMSTR_MAGIC	0x88726325
#define	TERMSTR_START	60
#define	TERMSTR_COLS	132
/* mask for graphic renditions */
#define	TERMSTR_GRMASK		\
	(TERMSTR_GRBOLD| TERMSTR_GRUNDER| TERMSTR_GRBLINK| TERMSTR_GRREV)

#define	TCF_MDEFAULT	0x0000	/* default */
#define	TCF_MEC		0x0001	/* erase character */
#define	TCF_MVCV	0x0002	/* cursor visibility (VT) */
#define	TCF_MACV	0x0004	/* cursor visibility (ANSI) */
#define	TCF_MSCV	0x1000	/* cursor visibility (SCREEN) */
#define	TCF_MPSF	0x0008	/* has a preferred supplimental font */
#define	TCF_MSCS94	0x0010	/* supplemental character set 94 */
#define	TCF_MSCS96	0x0020	/* supplemental character set 96 */
#define	TCF_MSD		0x0040	/* has a status display (line) */
#define	TCF_MSCS94A	0x0080	/* supplemental character set 94a */
#define	TCF_MSR		0x0100	/* has setable line-scrolling regions */
#define	TCF_MSL		0x0400	/* setable number of lines */
#define	TCF_MVCSR	0x0200	/* cursor save-restore (VT) */
#define	TCF_MACSR	0x0800	/* cursor save-restore (ANSI) */
#define	TCF_MACSRS	0x2000	/* cursor save-restore (ANSI) is screwed */

#define	TCF_MBASIC	(TCF_MSR)
#define	TCF_MVT		(TCF_MSR | TCF_MVCSR)
#define	TCF_MVTE	(TCF_MSR | TCF_MVCSR | TCF_MEC)
#define	TCF_MVTADV	\
	(TCF_MVTE) | \
	(TCF_MPSF | TCF_MSCS94 | TCF_MSCS96 | TCF_MVCV | TCF_MSD) | \
	(TCF_MSL)

#define	TCF_MSCREEN	\
	(TCF_MVTE | TCF_MVCV | TCF_MACV | TCF_MSCV | TCF_MACSR)


#define	TLEN		30		/* stage buffer length */
#define	NBLANKS		8


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	termconseq(char *,int,int,int,int,int,int) ;
extern int	buffer_blanks(BUFFER *,int) ;
extern int	buffer_backs(BUFFER *,int) ;

#if	CF_DEBUGS
extern int	strnnlen(const char *,int,int) ;
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct termtype {
	const char	*name ;
	uint		flags ;
} ;


/* forward references */

static int	termstr_curm(TERMSTR *,int,int) ;
static int	termstr_findterm(TERMSTR *,const char *) ;
static int	termstr_conseq(TERMSTR *,int,int,int,int,int) ;


/* local variables */

static const struct termtype	terms[] = {
	{ "sun", 0 },
	{ "vt100", (TCF_MVT) },
	{ "ansi", (TCF_MSR | TCF_MEC | TCF_MACV | TCF_MACSR | TCF_MACSRS) },
	{ "vt101", (TCF_MVTE) },
	{ "vt102", (TCF_MVTE) },
	{ "vt220", (TCF_MVTE | TCF_MSCS94) },
	{ "xterm", (TCF_MVTE) },
	{ "xterm-color", (TCF_MVTE) },
	{ "screen", (TCF_MSCREEN | TCF_MSCS94A) },
	{ "screen94a", (TCF_MSCREEN | TCF_MSCS94 | TCF_MSCS94A) },
	{ "screen96", (TCF_MSCREEN | TCF_MSCS94 | TCF_MSCS96) },
	{ "vt320", (TCF_MVTADV) },
	{ "vt330", (TCF_MVTADV) },
	{ "vt340", (TCF_MVTADV) },
	{ "vt420", (TCF_MVTADV) },
	{ "vt430", (TCF_MVTADV) },
	{ "vt440", (TCF_MVTADV) },
	{ "vt520", (TCF_MVTADV) },
	{ "vt530", (TCF_MVTADV) },
	{ "vt540", (TCF_MVTADV) },
	{ NULL, 0 }
} ;

static const char	curtypes[] = "ABCD" ;

enum curtypes {
	curtype_u,
	curtype_d,
	curtype_r,
	curtype_l,
	curtype_overlast
} ;


/* exported subroutines */


int termstr_start(TERMSTR *op,cchar *termtype)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (termtype == NULL) return SR_FAULT ;

	if (termtype[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("termstr_start: termtype=%s\n",termtype) ;
#endif

	memset(op,0,sizeof(TERMSTR)) ;
	op->ti = -1 ;

/* find this terminal in the database */

	if ((rs = termstr_findterm(op,termtype)) >= 0) {
	    if ((rs = buffer_start(&op->b,TERMSTR_START)) >= 0) {
	        op->magic = TERMSTR_MAGIC ;
	    }
	} /* end if (termstr_findterm) */

	return rs ;
}
/* end subroutine (termstr_start) */


int termstr_finish(TERMSTR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	rs1 = buffer_finish(&op->b) ;
	if (rs >= 0) rs = rs1 ;

	op->ti = -1 ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutines (termstr_finish) */


/* clear buffer */
int termstr_clean(TERMSTR *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	rs = buffer_reset(&op->b) ;

	return rs ;
}
/* end subroutines (termstr_clean) */


/* write to the buffer */
int termstr_write(TERMSTR *op,cchar bp[],int bl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	if (rs >= 0) {
	    rs = buffer_buf(&op->b,bp,bl) ;
	    len += rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutines (termstr_write) */


/* write to the buffer w/ graphic-rendition */
int termstr_writegr(TERMSTR *op,int gr,cchar bp[],int bl)
{
	const int	grmask = TERMSTR_GRMASK ;
	int		rs = SR_OK ;
	int		len = 0 ;
	int		f_have = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	if ((rs >= 0) && (gr & grmask)) {
	    int	a1 = (gr & TERMSTR_GRBOLD) ? ANSIGR_BOLD : -1 ;
	    int	a2 = (gr & TERMSTR_GRUNDER) ? ANSIGR_UNDER : -1 ;
	    int	a3 = (gr & TERMSTR_GRBLINK) ? ANSIGR_BLINK : -1 ;
	    int	a4 = (gr & TERMSTR_GRREV) ? ANSIGR_REV : -1 ;
	    f_have = TRUE ;
	    rs = termstr_conseq(op,'m',a1,a2,a3,a4) ;
	    len += rs ;
	}

	if (rs >= 0) {
	    rs = buffer_buf(&op->b,bp,bl) ;
	    len += rs ;
	}

	if ((rs >= 0) && gr && f_have) {
	    const int	code = ANSIGR_OFFALL ;
	    rs = termstr_conseq(op,'m',code,-1,-1,-1) ;
	    len += rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutines (termstr_writegr) */


/* add a character to the buffer */
int termstr_char(TERMSTR *op,int ch)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	rs = buffer_char(&op->b,ch) ;

	return rs ;
}
/* end subroutines (termstr_char) */


/* get the buffer */
int termstr_get(TERMSTR *op,cchar **rpp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	rs = buffer_get(&op->b,rpp) ;

	return rs ;
}
/* end subroutines (termstr_get) */


/* erase-display */
int termstr_ed(TERMSTR *op,int type)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

/* type: 0=forward, 1=back, 2=whole */

	if (type >= 2)
	    type = 2 ;

	rs = termstr_conseq(op,'J',type,-1,-1,-1) ;

	return rs ;
}
/* end subroutine (termstr_ed) */


/* erase-line */
int termstr_el(TERMSTR *op,int type)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

/* 0=forward, 1=back, 2=whole */

	if (type >= 2)
	    type = 2 ;

	rs = termstr_conseq(op,'K',type,-1,-1,-1) ;

	return rs ;
}
/* end subroutine (termstr_el) */


/* erase-character */
int termstr_ec(TERMSTR *op,int n)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	if (n > 0) {
	    char	tbuf[TLEN + 1] ;
	    char	*bp ;
	    if (n >= TERMSTR_COLS) n = TERMSTR_COLS ;
	    if (n <= 1) {
	        bp = tbuf ;
	        *bp++ = ' ' ;
	        *bp++ = CH_BS ;
	        *bp = '\0' ;
	        rs = buffer_strw(&op->b,tbuf,(bp-tbuf)) ;
	    } else {
	        const int	ti = op->ti ;
	        uint		tf ;
	        tf = terms[ti].flags ;
	        if (tf & TCF_MEC) {
	            rs = termstr_conseq(op,'X',n,-1,-1,-1) ;
	        } else {
	            rs = buffer_blanks(&op->b,n) ;
	            if (rs >= 0)
	                rs = buffer_backs(&op->b,n) ;
	        } /* end if */
	    } /* end if */
	} else if (n < 0)
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (termstr_ec) */


/* cursor-up */
int termstr_curu(TERMSTR *op,int n)
{

	return termstr_curm(op,curtype_u,n) ;
}
/* end subroutine (termstr_curu) */


/* cursor-down */
int termstr_curd(TERMSTR *op,int n)
{

	return termstr_curm(op,curtype_d,n) ;
}
/* end subroutine (termstr_curd) */


/* cursor-right */
int termstr_curr(TERMSTR *op,int n)
{

	return termstr_curm(op,curtype_r,n) ;
}
/* end subroutine (termstr_curr) */


/* cursor-left */
int termstr_curl(TERMSTR *op,int n)
{

	return termstr_curm(op,curtype_l,n) ;
}
/* end subroutine (termstr_curl) */


/* cursor-home */
int termstr_curh(TERMSTR *op,int r,int c)
{
	int		rs = SR_OK ;
	int		sl = -1 ;
	const char	*sp = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	if ((r <= 0) && (c <= 0)) {
	    sp = "\033[H" ;
	    sl = 3 ;
	    rs = buffer_strw(&op->b,sp,sl) ;
	} else if (c <= 0) {
	    rs = termstr_conseq(op,'H',(r+1),-1,-1,-1) ;
	} else {
	    if (r < 0) r = 0 ;
	    rs = termstr_conseq(op,'H',(r+1),(c+1),-1,-1) ;
	}

	return rs ;
}
/* end subroutine (termstr_curh) */


/* set-scroll-region */
int termstr_ssr(TERMSTR *op,int r,int n)
{
	int		rs = SR_OK ;
	int		sl = -1 ;
	const char	*sp = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	if (r < 0) {
	    sp = "\033[r" ;
	    sl = 3 ;
	    rs = buffer_strw(&op->b,sp,sl) ;
	} else if (n > 0) {
	    if (r < 0) r = 0 ;
	    rs = termstr_conseq(op,'r',(r+1),(r+n),-1,-1) ;
	}

	return rs ;
}
/* end subroutine (termstr_ssr) */


/* crusor-save-restore */
int termstr_csr(TERMSTR *op,int f)
{
	uint		tf ;
	int		rs = SR_OK ;
	int		sl = -1 ;
	int		ti ;
	const char	*sp = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TERMSTR_MAGIC) return SR_NOTOPEN ;

	ti = op->ti ;
	tf = terms[ti].flags ;
	if (tf & TCF_MVCSR) {
	    sp = (f) ? TERMSTR_VCURS : TERMSTR_VCURR ;
	} else if (tf & TCF_MACSR) {
	    sp = (f) ? TERMSTR_ACURS : TERMSTR_ACURR ;
	} else
	    rs = SR_NOTSUP ;

	if ((rs >= 0) && (sp != NULL))
	    rs = buffer_strw(&op->b,sp,sl) ;

	return rs ;
}
/* end subroutine (termstr_csr) */


#ifdef	COMMENT

/* insert */
termstr_il(op,n) ;
termstr_ic(op,n) ;

/* delete */
termstr_dl(op,n) ;
termstr_dc(op,n) ;

/* insert-replacement-mode */
termstr_irm(op,f) ;

/* cursor-visibility */
termstr_cvis(op,f) ;

#endif /* COMMENT */


/* private subroutines */


static int termstr_curm(TERMSTR *op,int curtype,int n)
{
	int		rs = SR_OK ;
	int		name ;
	int		bl = -1 ;
	char		tbuf[TLEN + 1] ;
	char		*bp = NULL ;

	if (n != 0) {
	if ((curtype >= 0) && (curtype < curtype_overlast)) {
	    name = curtypes[curtype] ;
	    if (n <= 1) {
	        bp = tbuf ;
	        *bp++ = '\033' ;
	        *bp++ = CH_LBRACK ;
	        *bp++ = name ;
	        bl = 3 ;
	        rs = buffer_strw(&op->b,bp,bl) ;
	    } else {
	        rs = termstr_conseq(op,name,n,-1,-1,-1) ;
	    }
	} else
	    rs = SR_INVALID ;
	} /* end if (non-zero) */

	return rs ;
}
/* end subroutine (termstr_curm) */


static int termstr_findterm(TERMSTR *op,cchar *termtype)
{
	int		rs = SR_OK ;
	int		slen = strlen(termtype) ;
	int		n ;
	int		m ;
	int		m_max = 0 ;
	int		i, si ;
	const char	*np ;

	n = 2 ;
	m_max = -1 ;
	si = -1 ;
	for (i = 0 ; terms[i].name != NULL ; i += 1) {

	    np = terms[i].name ;
	    m = nleadstr(np,termtype,-1) ;

#if	CF_DEBUGS && 0
	    debugprintf("termstr_findterm: np=%s\n",np) ;
	    debugprintf("termstr_findterm: nleadstr() rs=%d\n",m) ;
#endif

	    if (((m >= n) && (m == slen)) || (np[m] == '\0')) {
	        if (m > m_max) {
	            m_max = m ;
	            si = i ;
	        }
	    } /* end if */

	} /* end for */

	rs = (si >= 0) ? si : SR_NOTFOUND ;
	if (rs >= 0)
	    op->ti = si ;

	return rs ;
}
/* end subroutine (termstr_findterm) */


static int termstr_conseq(TERMSTR *op,int name,int a1,int a2,int a3,int a4)
{
	const int	tlen = TLEN ;
	int		rs = SR_OK ;
	int		sl ;
	int		len = 0 ;
	char		tbuf[TLEN + 1] ;

	if (name != 0) {
	    rs = termconseq(tbuf,tlen,name,a1,a2,a3,a4) ;
	    sl = rs ;
	    if (rs >= 0) {
	        rs = buffer_strw(&op->b,tbuf,sl) ;
	        len += rs ;
	    }
	} else
	    rs = SR_INVALID ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (termstr_conseq) */


