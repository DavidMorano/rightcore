/* biblecur */

/* bible-cursor */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2009-04-01, David A­D­ Morano

	This subroutine was written as an enhancement for adding
	back-matter (end pages) to the output document.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine does something! :-)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<field.h>
#include	<char.h>
#include	<localmisc.h>

#include	"biblecur.h"


/* local defines */


/* external subroutines */

extern int	field_word(FIELD *,const uchar *,cchar **) ;

extern int	siskipwhite(cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int biblecur_start(BIBLECUR *bcp)
{

	memset(bcp,0,sizeof(BIBLECUR)) ;
	bcp->book = -1 ;
	bcp->chapter = -1 ;
	bcp->verse = -1 ;
	return SR_OK ;
}
/* end subroutine (biblecur_start) */


int biblecur_finish(BIBLECUR *bcp)
{

	bcp->book = -1 ;
	bcp->chapter = -1 ;
	bcp->verse = -1 ;
	return SR_OK ;
}
/* end subroutine (biblecur_finish) */


int biblecur_check(BIBLECUR *bcp,cchar *buf,int buflen)
{
	int		rs = SR_OK ;
	int		sl ;
	int		si = 0 ;
	int		ch ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("biblecur_check: ent b=>%t<\n",
	    buf,strlinelen(buf,buflen,50)) ;
#endif

	sp = buf ;
	sl = buflen ;
	if (sl < 0) sl = strlen(sp) ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	if (sl > 0) {

/* start of new line */

#ifdef	COMMENT
	    bcp->f.newbook = FALSE ;
	    bcp->f.newchapter = FALSE ;
	    bcp->f.newverse = FALSE ;
#endif

/* go */

	    ch = MKCHAR(*sp) ;
	    if (isdigitlatin(ch)) {
	        FIELD	fsb ;
	        int	n_book, n_chapter, n_verse ;
	        uchar	nterms[32] ;

	        n_book = -1 ;
	        n_chapter = -1 ;
	        n_verse = -1 ;
	        fieldterms(nterms,FALSE,": \t") ;
	        if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	            int		new ;
	            int		i = 0 ;
	            int		fl ;
	            cchar	*fp ;

	            while ((fl = field_word(&fsb,nterms,&fp)) >= 0) {
	                if (fl > 0) {
	                    si = ((fp + fl) - buf) ;

#if	CF_DEBUGS
	                    debugprintf("biblecur_check: f=>%t<\n",fp,fl) ;
#endif

	                    if ((rs = cfdeci(fp,fl,&new)) >= 0) {
	                        switch (i) {
	                        case 0:
	                            n_book = new ;
	                            break ;
	                        case 1:
	                            n_chapter = new ;
	                            break ;
	                        case 2:
	                            n_verse = new ;
	                            break ;
	                        } /* end switch */
	                    } /* end if (ctdeci) */

	                } /* end if (non-zero) */
	                i += 1 ;
	                if (i == 3) break ;
	                if (rs < 0) break ;
	            } /* end while (field_word) */

	            field_finish(&fsb) ;
	        } /* end if (field) */

	        if (rs >= 0) {

	            if ((n_book >= 0) && (n_book != bcp->book)) {
	                bcp->f.newbook = TRUE ;
	                bcp->f.newchapter = TRUE ;
	                bcp->f.newverse = TRUE ;
	                bcp->book = n_book ;
	            }

	            if ((n_chapter >= 0) && (n_chapter != bcp->chapter)) {
	                bcp->f.newchapter = TRUE ;
	                bcp->f.newverse = TRUE ;
	                bcp->chapter = n_chapter ;
	            }

	            if ((n_verse >= 0) && (n_verse != bcp->verse)) {
	                bcp->f.newverse = TRUE ;
	                bcp->verse = n_verse ;
	            }

	            si += siskipwhite((sp + si),(sl - si)) ;

	        } /* end if (ok) */

	    } /* end if (proper character type) */

	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("biblecur_check: ret rs=%d si=%u\n",rs,si) ;
#endif

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (biblecur_check) */


int biblecur_newbook(BIBLECUR *bcp,BIBLEBOOK *bbp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = bcp->f.newbook ;

	if (f) {
	    const int	blen = BIBLEBOOK_LEN ;
	    rs1 = biblebook_get(bbp,bcp->book,bcp->bookname,blen) ;
	    if (rs1 < 0) {
	        rs = bufprintf(bcp->bookname,blen,"Book %u",
	            bcp->book) ;
	    }
	}

	bcp->f.newbook = FALSE ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (biblecur_newbook) */


int biblecur_newchapter(BIBLECUR *bcp)
{
	int		f = bcp->f.newchapter ;

	bcp->f.newchapter = FALSE ;
	return f ;
}
/* end subroutine (biblecur_newchapter) */


int biblecur_newverse(BIBLECUR *bcp,int sl)
{
	int		f = FALSE ;

	if (bcp->f.newverse && (sl > 0)) {
	    f = TRUE ;
	    bcp->f.newverse = FALSE ;
	}

	return f ;
}
/* end subroutine (biblecur_newverse) */


