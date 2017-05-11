/* gecos */

/* parse a GECOS field located in a buffer */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine (object) module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module parses out the various GOCOS information as it is given in
        its encoded form in a specified buffer.

	Extra Notes:

        The GECOS field of the 'passwd' database should be formatted in one of
        the following ways:

	    name,office,workphone,homephone
	    organization-name(account,bin)office,workphone,homephone,printer
	    name(account)office,workphone,homephone,printer
	    name

        Note also that an ampersand character ('&') that appears anywhere in the
        GCOS field is to be logically replaced by the corresponding username of
        the entry.

	The original AT&T GECOS field contained:

	    department-name(account,bin)

	and was further put into a 'struct comment' with fields:

	    c_dept
	    c_name
	    c_acct
	    c_bin

        If a real-name 'name' contains a hyphen character naturally (it is part
        of the actual real-name) then it should be entered into the GECOS field
        with an underscore substituted for where original hyphen charaters
        appear. These are converted back to hyphen characters when read out to
        callers by various "read-out" subroutine interfaces. This object does
        not do this "hyphen" conversion itself and so a higher level interface
        must perform that function.

	Some suggestions for the GECOS field are:

	    orgdept-name(account,bin)office,workphone,homephone
	    orgdept-name(account,bin)office,workphone,homephone,printer

	Actual examples:

	    XNR64430-d.a.morano(126483,BIN8221)
	    rockwell-d.a.morano(126283,BIN8221)4B-411,5336,6175679484,hp0


*******************************************************************************/


#define	GECOS_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"gecos.h"


/* local defines */

#undef	N
#define	N		NULL

#ifndef	CH_LPAREN
#define	CH_LPAREN	0x28
#define	CH_RPAREN	0x29
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	gecos_storeit(GECOS *,SBUF *,int) ;
static int	gecos_storename(GECOS *,SBUF *,const char *) ;


/* local variables */

static const char	brkleft[] = {
	CH_COMMA, CH_LPAREN, '\0',
} ;

static const char	brkright[] = {
	CH_COMMA, CH_RPAREN, '\0',
} ;


/* exported subroutines */


int gecos_start(GECOS *op,cchar *sbuf,int slen)
{
	int		n ;
	int		bl = slen ;
	int		f_paren = FALSE ;
	const char	*tp ;
	const char	*bp ;

	if (op == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	memset(op,0,sizeof(GECOS)) ;

	if (slen < 0) slen = strlen(sbuf) ;

	n = 0 ;
	bp = sbuf ;
	bl = slen ;

#if	CF_DEBUGS
	debugprintf("gecos_start: start bl=%u\n",bl) ;
#endif

/* do we have the AT&T standard leading department/organization? */

	if ((tp = strnchr(bp,bl,'-')) != N) {

	    if ((strnpbrk(tp,(bl - (tp - bp)),brkleft) != NULL) ||
	        (strnpbrk(bp,(tp - bp),brkleft) == NULL)) {

	        n += 1 ;
	        op->vals[gecosval_organization].vp = bp ;
	        op->vals[gecosval_organization].vl = (tp - bp) ;

	        bp = (tp + 1) ;
	        bl = ((sbuf + slen) - (tp + 1)) ;

	    } /* end if */

	} /* end if (organization) */

/* OK, everybody has the real user name! */

#if	CF_DEBUGS
	debugprintf("gecos_start: realname bl=%u\n",bl) ;
#endif

	if (bl > 0) {

	    if ((tp = strnpbrk(bp,bl,brkleft)) != N) {

	        if (tp[0] == CH_LPAREN)
	            f_paren = TRUE ;

	        n += 1 ;
	        op->vals[gecosval_realname].vp = bp ;
	        op->vals[gecosval_realname].vl = (tp - bp) ;

	        bp = (tp + 1) ;
	        bl = (sbuf + slen) - (tp + 1) ;

	    } else {

	        n += 1 ;
	        op->vals[gecosval_realname].vp = bp ;
	        op->vals[gecosval_realname].vl = bl ;
	        bl = 0 ;

	    } /* end if */

	} /* end if */

/* do we have the standard AT&T account-bin information? */

#if	CF_DEBUGS
	debugprintf("gecos_start: account-bin bl=%u\n",bl) ;
#endif

	if (bl > 0) {

	    if (f_paren &&
	        ((tp = strnpbrk(bp,bl,brkright)) != N)) {

	        if (tp[0] == CH_RPAREN)
	            f_paren = FALSE ;

	        if (tp - bp) {
	            n += 1 ;
	            op->vals[gecosval_account].vp = bp ;
	            op->vals[gecosval_account].vl = (tp - bp) ;
	        }

	        bp = (tp + 1) ;
	        bl = (sbuf + slen) - (tp + 1) ;

	    } else {

	        n += 1 ;
	        op->vals[gecosval_bin].vp = bp ;
	        op->vals[gecosval_bin].vl = bl ;
	        bl = 0 ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("gecos_start: bin bl=%u\n",bl) ;
#endif

	if ((bl > 0) && f_paren) {

	    if ((tp = strnpbrk(bp,bl,brkright)) != N) {

	        if (tp[0] == CH_RPAREN)
	            f_paren = FALSE ;

	        if ((tp - bp) && (op->vals[gecosval_bin].vp == NULL)) {
	            n += 1 ;
	            op->vals[gecosval_bin].vp = bp ;
	            op->vals[gecosval_bin].vl = (tp - bp) ;
	        }

	        bp = (tp + 1) ;
	        bl = (sbuf + slen) - (tp + 1) ;

	    } else if (op->vals[gecosval_bin].vp == NULL) {

	        n += 1 ;
	        op->vals[gecosval_bin].vp = bp ;
	        op->vals[gecosval_bin].vl = bl ;
	        bl = 0 ;

	    } /* end if */

	} /* end if (possible printer-bin item) */

/* what about the finger information stuff? */

#if	CF_DEBUGS
	debugprintf("gecos_start: office bl=%u\n",bl) ;
#endif

	if (bl > 0) {

	    if ((tp = strnpbrk(bp,bl,brkright)) != N) {

	        if (tp - bp) {
	            n += 1 ;
	            op->vals[gecosval_office].vp = bp ;
	            op->vals[gecosval_office].vl = (tp - bp) ;
	        }

	        bp = (tp + 1) ;
	        bl = (sbuf + slen) - (tp + 1) ;

	    } else {

	        n += 1 ;
	        op->vals[gecosval_office].vp = bp ;
	        op->vals[gecosval_office].vl = bl ;
	        bl = 0 ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("gecos_start: wphone bl=%u\n",bl) ;
#endif

	if (bl > 0) {

	    if ((tp = strnpbrk(bp,bl,brkright)) != N) {

	        if (tp - bp) {
	            n += 1 ;
	            op->vals[gecosval_wphone].vp = bp ;
	            op->vals[gecosval_wphone].vl = (tp - bp) ;
	        }

	        bp = (tp + 1) ;
	        bl = (sbuf + slen) - (tp + 1) ;

	    } else if (op->vals[gecosval_office].vp == NULL) {

	        n += 1 ;
	        op->vals[gecosval_office].vp = bp ;
	        op->vals[gecosval_office].vl = bl ;
	        bl = 0 ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("gecos_start: hphone bl=%u\n",bl) ;
#endif

	if (bl > 0) {

	    if ((tp = strnpbrk(bp,bl,brkright)) != N) {

	        if (tp - bp) {
	            n += 1 ;
	            op->vals[gecosval_hphone].vp = bp ;
	            op->vals[gecosval_hphone].vl = (tp - bp) ;
	        }

	        bp = (tp + 1) ;
	        bl = (sbuf + slen) - (tp + 1) ;

	    } else {

	        n += 1 ;
	        op->vals[gecosval_hphone].vp = bp ;
	        op->vals[gecosval_hphone].vl = bl ;
	        bl = 0 ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("gecos_start: printer bl=%u\n",bl) ;
#endif

	if (bl > 0) {

	    if ((tp = strnpbrk(bp,bl,brkright)) != N) {

	        if (tp - bp) {
	            n += 1 ;
	            op->vals[gecosval_printer].vp = bp ;
	            op->vals[gecosval_printer].vl = (tp - bp) ;
	        }

	        bp = (tp + 1) ;
	        bl = (sbuf + slen) - (tp + 1) ;

	    } else {

	        n += 1 ;
	        op->vals[gecosval_printer].vp = bp ;
	        op->vals[gecosval_printer].vl = bl ;
	        bl = 0 ;

	    } /* end if */

	} /* end if */

	return n ;
}
/* end subroutine (gecos_start) */


int gecos_finish(GECOS *op)
{
	int		i ;

	if (op == NULL) return SR_FAULT ;

	for (i = 0 ; i < gecosval_overlast ; i += 1) {
	    op->vals[i].vp = NULL ;
	    op->vals[i].vl = 0 ;
	}

	return SR_OK ;
}
/* end subroutine (gecos_finish) */


int gecos_getval(GECOS *op,int i,cchar **rpp)
{

	if (op == NULL) return SR_FAULT ;

	if (i >= gecosval_overlast) return SR_INVALID ;

	if (rpp != NULL) {
	    *rpp = op->vals[i].vp ;
	}

	return op->vals[i].vl ;
}
/* end subroutine (gecos_getval) */


/* create a GECOS field from the object values, put into a buffer */
int gecos_compose(GECOS *op,char rbuf[],int rlen)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int	f_paren = FALSE ;

	    if (op->vals[gecosval_organization].vp != NULL) {
	        gecos_storeit(op,&b,gecosval_organization) ;
	        sbuf_strw(&b,"-",1) ;
	    }

	    if (op->vals[gecosval_realname].vp != NULL) {
	        int		vi = gecosval_realname ;
	        const char	*tp ;
	        tp = strnchr(op->vals[vi].vp,op->vals[vi].vl,'-') ;
	        if (tp != NULL) {
	            gecos_storename(op,&b,tp) ;
	        } else {
	            gecos_storeit(op,&b,gecosval_realname) ;
		}
	    } /* end if (realname) */

/* do we have account and printer-bin information? */

	    if ((op->vals[gecosval_account].vp != NULL) || 
	        (op->vals[gecosval_bin].vp != NULL)) {
	        f_paren = TRUE ;
	        sbuf_char(&b,CH_LPAREN) ;
	    }

	    if (op->vals[gecosval_account].vp != NULL) {
	        gecos_storeit(op,&b,gecosval_account) ;
	    }

	    if (op->vals[gecosval_bin].vp != NULL) {
	        sbuf_strw(&b,",",1) ;
	        gecos_storeit(op,&b,gecosval_bin) ;
	    }

	    if (f_paren) {
	        sbuf_char(&b,CH_RPAREN) ;
	    }

/* do we have the old finger stuff */

	    if (op->vals[gecosval_office].vp != NULL) {
	        if (! f_paren) sbuf_strw(&b,",",1) ;
	        gecos_storeit(op,&b,gecosval_office) ;
	    }

	    if (op->vals[gecosval_wphone].vp != NULL) {
	        sbuf_strw(&b,",",1) ;
	        gecos_storeit(op,&b,gecosval_wphone) ;
	    }

	    if (op->vals[gecosval_hphone].vp != NULL) {
	        sbuf_strw(&b,",",1) ;
	        gecos_storeit(op,&b,gecosval_hphone) ;
	    }

	    if (op->vals[gecosval_printer].vp != NULL) {
	        sbuf_strw(&b,",",1) ;
	        gecos_storeit(op,&b,gecosval_printer) ;
	    }

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (gecos_compose) */


/* private subroutines */


static int gecos_storeit(GECOS *op,SBUF *bp,int vi)
{
	int		rs ;

	rs = sbuf_strw(bp,op->vals[vi].vp,op->vals[vi].vl) ;

	return rs ;
}
/* end subroutine (gecos_storeit) */


static int gecos_storename(GECOS *op,SBUF *bp,cchar *tp)
{
	int		vi = gecosval_realname ;
	int		sl ;
	const char	*sp ;

	sp = op->vals[vi].vp ;
	sl = op->vals[vi].vl ;

/* store the initial segment of the name */

	if ((tp - sp) > 0) {
	    sbuf_strw(bp,sp,(tp - sp)) ;
	    sl -= (tp - sp) ;
	    sp = tp ;
	}

/* make the substitution */

	{
	    sbuf_char(bp,'_') ;
	    sp += 1 ;
	    sl -= 1 ;
	}

/* loop searching for other segments */

	while ((tp = strnchr(sp,sl,'-')) != NULL) {
	    sbuf_strw(bp,sp,(tp - sp)) ;
	    sl -= (tp - sp) ;
	    sp = tp ;
	} /* end while */

	if (sl > 0) {
	    sbuf_strw(bp,sp,sl) ;
	}

	return 0 ;
}
/* end subroutine (gecos_storename) */


