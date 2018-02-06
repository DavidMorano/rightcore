/* lookword */

/* word look-up object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We lookup a word in a dictionary.


*******************************************************************************/

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<naturalwords.h>
#include	<ascii.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"lookword.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	LOGICVAL
#define	LOGICVAL(v)	((v)?1:0)
#endif

/*
 * FOLD and DICT convert characters to a normal form for comparison,
 * according to the user specified flags.
 *
 * DICT expects integers because it uses a non-character value to
 * indicate a character which should not participate in comparisons.
 */
#define NO_COMPARE	(-2)

#if	CF_CHAR
#define DICT(c)		(isdict(c) ? (c) /* int */ : NO_COMPARE)
#define FOLD(c)		CHAR_TOFC(c)
#else /* CF_CHAR */
#define DICT(c) (isalnum(c) ? (c) & 0xFF /* int */ : NO_COMPARE)
#define FOLD(c) (isupper(c) ? tolower(c) : (char) (c))
#endif /* CF_CHAR */


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	strnndictcmp(const char *,int,const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	isdict(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	lookword_proc(LOOKWORD *,LOOKWORD_CUR *,
			cchar *,cchar *,cchar *) ;
static int	lookword_mksword(LOOKWORD *,char *,int,const char *) ;
static int	lookword_record(LOOKWORD *,LOOKWORD_CUR *,
			cchar *,cchar *,cchar *) ;
static int	lookword_recorder(LOOKWORD *,LOOKWORD_CUR *,VECOBJ *,int) ;

static int      compare(LOOKWORD *,cchar *,cchar *,cchar *,int *) ;

#if	CF_DEBUGS
static int	strtlen(const char *) ;
#endif

static cchar    *binary_search(LOOKWORD *,cchar *,cchar *,cchar *) ;
static cchar    *linear_search(LOOKWORD *,cchar *,cchar *,cchar *) ;


/* local variables */


/* exported subroutines */


int lookword_open(LOOKWORD *op,cchar *dfname,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (dfname == NULL) return SR_FAULT ;

	memset(op,0,sizeof(LOOKWORD)) ;
	op->f.dict = LOGICVAL(opts&LOOKWORD_ODICT) ;
	op->f.fold = LOGICVAL(opts&LOOKWORD_OFOLD) ;
	op->f.word = LOGICVAL(opts&LOOKWORD_OWORD) ;

	if ((rs = uc_open(dfname,O_RDONLY,0666)) >= 0) {
	    int	fd = rs ;
	    if ((rs = uc_fsize(fd)) >= 0) {
	        size_t		ms = rs ;
	        const int	mp = PROT_READ ;
	        const int	mf = MAP_SHARED ;
	        const char	*md ;
	        if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	            op->fd = fd ;
	            op->md = md ;
	            op->ms = ms ;
	            op->magic = LOOKWORD_MAGIC ;
	        } /* end if (mapfile) */
	    } /* end if (fsize) */
	    if (rs < 0)
	        u_close(fd) ;
	} /* end if (file-open) */

	return rs ;
}
/* end subroutine (lookword_open) */


int lookword_close(LOOKWORD *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->md != NULL) {
	    rs1 = u_munmap(op->md,op->ms) ;
	    if (rs >= 0) rs = rs1 ;
	    op->md = NULL ;
	    op->ms = 0 ;
	}

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (lookword_close) */


int lookword_curbegin(LOOKWORD *op,LOOKWORD_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	memset(curp,0,sizeof(LOOKWORD_CUR)) ;
	return SR_OK ;
}
/* end subroutine (lookword_curbegin) */


int lookword_curend(LOOKWORD *op,LOOKWORD_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (curp->ans != NULL) {
	    rs1 = uc_free(curp->ans) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->ans = NULL ;
	}
	curp->n = 0 ;
	return rs ;
}
/* end subroutine (lookword_curend) */


int lookword_lookup(LOOKWORD *op,LOOKWORD_CUR *curp,cchar *string)
{
	const int	slen = NATURALWORDLEN ;
	int		rs ;
	int		c = 0 ;
	char		sbuf[NATURALWORDLEN+1] ;

#if	CF_DEBUGS
	debugprintf("lookword_lookup: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (string == NULL) return SR_FAULT ;

	if ((rs = lookword_mksword(op,sbuf,slen,string)) > 0) {
	    cchar	*front = op->md ;
	    cchar	*back = (op->md+op->ms) ;
	    rs = lookword_proc(op,curp,front,back,sbuf) ;
	    c = rs ;
	}

#if	CF_DEBUGS
	debugprintf("lookword_lookup: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (lookword_lookup) */


int lookword_read(LOOKWORD *op,LOOKWORD_CUR *curp,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("lookword_read: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (curp->i < curp->n) {
	    LOOKWORD_WORD	*ap = (curp->ans + curp->i) ;
	    if ((rs = snwcpy(rbuf,rlen,ap->wp,ap->wl)) >= 0) {
	        rl = rs ;
	        curp->i += 1 ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("lookword_read: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (lookword_read) */


/* private subroutines */


static int lookword_proc(op,curp,front,back,string)
LOOKWORD	*op ;
LOOKWORD_CUR	*curp ;
cchar		*front ;
cchar		*back ;
cchar		*string ;
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	{
	    debugprintf("main/look: rewritten string=>%s<\n",string) ;
	    debugprintf("main/look: binary_search()\n") ;
	}
#endif

	front = binary_search(op,front,back,string) ;

#if	CF_DEBUGS
	debugprintf("main/look: linear_search\n") ;
#endif

	front = linear_search(op,front,back,string) ;

	if (front) {
	    rs = lookword_record(op,curp,front,back,string) ;
	    c = rs ;
	}

#if	CF_DEBUGS
	debugprintf("main/look: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (lookword_proc) */


static int lookword_mksword(LOOKWORD *op,char *rbuf,int rlen,cchar *s)
{
	int		rs = SR_OK ;
	int		i ;
	int		ch, dch, fch ;
	const char	*readp ;
	char		*writep ;

	readp = s ;
	writep = rbuf ;
	for (i = 0 ; (i < rlen) && (s[i] != '\0') ; i += 1) {
	    ch = MKCHAR(*readp++) ;
	    if (ch == 0) break ;

#if	CF_DEBUGS
	    debugprintf("main/mksword: och=%c\n",ch) ;
#endif

	    dch = (op->f.dict) ? DICT(ch) : ch ;

#if	CF_DEBUGS
	    debugprintf("main/mksword: ch=%c dch=%c(%02x)\n",ch,dch,dch) ;
#endif

	    if (dch != NO_COMPARE) {
	        fch = (op->f.fold) ? FOLD(dch) : dch ;
	        *(writep++) = fch ;
	    }

	} /* end for */
	*writep = '\0' ;

#if	CF_DEBUGS
	debugprintf("main/mksword: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (lookword_mksword) */


/* print as many lines as match string, starting at front */
static int lookword_record(op,curp,front,back,string)
LOOKWORD	*op ;
LOOKWORD_CUR	*curp ;
const char	*string, *front, *back ;
{
	VECOBJ		ans ;
	const int	esize = sizeof(LOOKWORD_WORD) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("main/lookword_record: s=>%s<\n",string) ;
#endif

	if ((rs = vecobj_start(&ans,esize,1,0)) >= 0) {
	    LOOKWORD_WORD	w ;
	    int			m ;
	    int			f_mat = TRUE ;
	    const char		*tp ;

	    while ((rs >= 0) && (front < back) && 
	        (compare(op,front,back,string,NULL) == 0)) {

	        if ((tp = strchr(front,'\n')) != NULL) {

	            if (op->f.word) {

	                f_mat = (compare(op,front,tp,string,&m) == 0) ;
	                f_mat = f_mat && 
	                    (string[m] == '\0') && (front[m] == '\n') ;
	                if (f_mat) {
	                    c += 1 ;
	                    w.wp = front ;
	                    w.wl = (tp-front) ;
	                    rs = vecobj_add(&ans,&w) ;
	                }

	            } else {

	                c += 1 ;
	                w.wp = front ;
	                w.wl = (tp-front) ;
	                rs = vecobj_add(&ans,&w) ;

	            } /* end if (whole or partial) */

	            front = (tp + 1) ;
	        } else {
	            front = back ;
		}

	    } /* end while */

	    if (rs >= 0) {
	        rs = lookword_recorder(op,curp,&ans,c) ;
	    }

	    rs1 = vecobj_finish(&ans) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecobj) */

#if	CF_DEBUGS
	debugprintf("lookword_record: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (lookword_record) */


static int lookword_recorder(LOOKWORD *op,LOOKWORD_CUR *curp,VECOBJ *alp,int c)
{
	const int	asize = ((c+1) * sizeof(LOOKWORD_WORD)) ;
	int		rs ;
	int		n = 0 ;
	void		*p ;

#if	CF_DEBUGS
	debugprintf("lookword_recorder: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (curp->ans != NULL) {
	    uc_free(curp->ans) ;
	    curp->ans = NULL ;
	}

	if ((rs = uc_malloc(asize,&p)) >= 0) {
	    LOOKWORD_WORD	*ans = (LOOKWORD_WORD *) p ;
	    LOOKWORD_WORD	*ep ;
	    int			i ;
	    curp->ans = (LOOKWORD_WORD *) p ;
	    for (i = 0 ; vecobj_get(alp,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            ans[n].wp = ep->wp ;
	            ans[n].wl = ep->wl ;
	            n += 1 ;
	        }
	    } /* end for */
	    ans[n].wp = NULL ;
	    ans[n].wl = 0 ;
	    curp->n = n ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("lookword_recorder: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (lookword_recorder) */


/*
 * Binary search for "string" in memory between "front" and "back".
 *
 * This routine is expected to return a pointer to the start of a line at
 * *or before* the first word matching "string".  Relaxing the constraint
 * this way simplifies the algorithm.
 *
 * Invariants:
 * 	'front' points to the beginning of a line at or before the first
 *	matching string.
 *
 * 	'back' points to the beginning of a line at or after the first
 *	matching line.
 *
 * Base of the Invariants.
 * 	front = NULL;
 *	back = EOF;
 *
 * Advancing the Invariants:
 *
 * 	p = first newline after halfway point from front to back.
 *
 * 	If the string at "p" is not greater than the string to match,
 *	p is the new front.  Otherwise it is the new back.
 *
 * Termination:
 *
 * 	The definition of the routine allows it [to] return at any point,
 *	since front is always at or before the line to print.
 *
 * 	In fact, it returns when the chosen "p" equals "back".  This
 *	implies that there exists a string [that] is [at] least half as long as
 *	(back - front), which in turn implies that a linear search will
 *	be no more expensive than the cost of simply printing a string or two.
 *
 * 	Trying to continue with binary search at this point would be
 *	more trouble than it's worth.
 */

#define	SKNL(p,back) \
	    while (((p) < (back)) && (*(p)++ != '\n')) ;

static const char *binary_search(op,front,back,string)
LOOKWORD	*op ;
const char	*string, *front, *back ;
{
	const char	*p = front + ((back - front) / 2) ;

#if	CF_DEBUGS
	debugprintf("main/binary_search: string=%s\n",string) ;
	debugprintf("main/binary_search: p(%p)\n",p) ;
#endif

	SKNL(p,back) ;

#if	CF_DEBUGS
	{
	    debugprintf("main/binary_search: after skip\n") ;
	    debugprintf("main/binary_search: len(p)=%u\n",strtlen(p)) ;
	    debugprintf("main/binary_search: starting p=%s\n",p) ;
	}
#endif

	while ((p < back) && (back > front)) {

#if	CF_DEBUGS
	    debugprintf("main/binary_search: p=%t\n",p,strtlen(p)) ;
#endif

	    if (compare(op,p,back,string,NULL) > 0) {
	        front = p ;
	    } else {
	        back = p ;
	    }

	    p = front + ((back - front) / 2) ;
	    SKNL(p, back) ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("main/binary_search: ret front(%p)\n",front) ;
#endif

	return (front) ;
}
/* end subroutine (binary_search) */


/****
  Find the first line that starts with string, linearly searching from front to
  back. Return NULL for no such line.

  This routine assumes: 
  + front points at the first character in a line 
  + front is before or at the first line to be printed

 ****/

static cchar *linear_search(op,front,back,string)
LOOKWORD	*op ;
const char	*string, *front, *back ;
{
	int		rc = 0 ;

	while (front < back) {

	    rc = compare(op,front,back,string,NULL) ;
	    if (rc <= 0) break ;

	    SKNL(front, back) ;

	} /* end while */

	return ((rc == 0) ? front : NULL) ;
}
/* end subroutine (linear_search) */


/*
 * Return LESS, GREATER, or EQUAL depending on how the string1 compares with
 * string2 (s1 ??? s2).
 *
 * 	o Matches up to len(s1) are EQUAL.
 *	o Matches up to len(s2) are GREATER.
 *
 * The string "s1" is NUL terminated.  The string s2 is '\n' terminated (or
 * "back" terminated).
 */

static int compare(LOOKWORD *op,cchar *s2,cchar *back,cchar *s1,int *np)
{
	int		ch1, ch2 ;
	int		fch1, fch2 ;
	int		i, j ;
	int		rc = 0 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	{
	    int	s2len = MIN(strtlen(s2),(back - s2)) ;
	    debugprintf("look/compare: s1=%s\n",s1) ;
	    debugprintf("look/compare: s2=%t\n",s2,s2len) ;
	}
#endif /* CF_DEBUGS */

	i = 0 ;
	j = 0 ;
	while (s1[i] && ((s2+j) < back) && (s2[j] != '\n')) {

	    ch1 = MKCHAR(s1[i]) ;
	    ch2 = MKCHAR(s2[j]) ;

#ifdef	OPTIONAL /* already performed */
	    if (op->f.dict && (! isdict(ch1))) {
	        i += 1 ;		/* ignore character in comparison */
	        continue ;
	    }
#endif /* OPTIONAL */

	    if (op->f.dict && (! isdict(ch2))) {
	        j += 1 ;		/* ignore character in comparison */
	        continue ;
	    }

	    if (op->f.fold) {
	        fch1 = FOLD(ch1) ;
	        fch2 = FOLD(ch2) ;
	        rc = (fch1 - fch2) ;

#if	CF_DEBUGS
	        {
	            debugprintf("look/compare: fch1=%c\n",fch1) ;
	            debugprintf("look/compare: fch2=%c\n",fch2) ;
	            debugprintf("look/compare: rc=%d\n",rc) ;
	        }
#endif

	    } else {
	        rc = (ch1 - ch2) ;
	    }

#if	CF_DEBUGS
	    debugprintf("look/compare: rc=%d\n",rc) ;
#endif

	    if (rc != 0) {
	        f = TRUE ;
	        break ;
	    }

	    i += 1 ;
	    j += 1 ;

	} /* end while */

#ifdef	COMMENT
	if ((! f) && (rc == 0)) {
	    f = ((s1[i] == '\0') && (s2[j] == '\n')) ;
	    if (! f)
	        rc = ((s1[i]) ? 1 : 0) ;
	}
#else /* COMMENT */
	if ((! f) && (rc == 0)) {
	    rc = ((s1[i]) ? 1 : 0) ;
	}
#endif /* COMMENT */

	if (np != NULL) *np = j ;

#if	CF_DEBUGS
	debugprintf("look/compare: ret i=%u rc=%d\n",i,rc) ;
#endif

	return rc ;
}
/* end subroutine (compare) */


#if	CF_DEBUGS
static int strtlen(cchar *s)
{
	int		i ;
	for (i = 0 ; *s && (*s != '\n') ; i += 1) {
	    s += 1 ;
	}
	return i ;
}
/* end subroutine (strtlen) */
#endif /* CF_DEBUGS */


