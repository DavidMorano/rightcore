/* defproc */

/* process an "def" (define) file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will read (process) an "def" (define) file and put all
        of the environment variables into an VECSTR object (supplied). New
        environment variables just get added to the list. Old environment
        variables already on the list are deleted with a new definition is
        encountered.

	Synopsis:

	int defproc(dlp,envv,clp,fname)
	vecstr		*dlp ;
	const char	*envv[] ;
	EXPCOOK		*clp ;
	const char	fname[] ;

	Arguments:

	dlp		defines list pointer
	envv		process environment
	clp		cookies list pointer
	fname		file to process

	Returns:

	>=0		count of environment variables
	<0		bad


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<field.h>
#include	<ascii.h>
#include	<char.h>
#include	<buffer.h>
#include	<expcook.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	FBUFLEN
#define	FBUFLEN		(4 * MAXPATHLEN)
#endif

#define	ENVNAMELEN	100		/* should be sufficient? */

#define	BUFLEN		(4 * MAXPATHLEN)

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	vecstr_envadd(VECSTR *,const char *,const char *,int) ;
extern int	isprintlatin(int) ;

extern int	getev(const char **,const char *,int,const char **) ;
extern int	sfthing(const char *,int,const char *,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct subinfo {
	const char	**envv ;
	EXPCOOK		*clp ;
	vecstr		*dlp ;
} ;


/* forward references */

static int	procline(SUBINFO *,const char *,int) ;
static int	checkdeps(SUBINFO *,const char *,int) ;
static int	procvalues(SUBINFO *,BUFFER *,cchar *,cchar *,int) ;
static int	procvalue(SUBINFO *,BUFFER *,const char *,cchar *,int) ;
static int	procsubenv(SUBINFO *,BUFFER *,const char *,int) ;


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x1A, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const uchar	dterms[32] = {
	0x00, 0x02, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	ssb[] = {
	CH_LBRACE, CH_RBRACE, 0
} ;

#ifdef	COMMENT
static const char	ssp[] = {
	CH_LPAREN, CH_RPAREN, 0
} ;
#endif /* COMMENT */


/* exported subroutines */


int defproc(dlp,envv,clp,fname)
vecstr		*dlp ;
const char	*envv[] ;
EXPCOOK		*clp ;
const char	fname[] ;
{
	SUBINFO		li, *lip = &li ;
	bfile		loadfile, *lfp = &loadfile ;
	int		rs ;
	int		c = 0 ;

	lip->envv = envv ;
	lip->clp = clp ;
	lip->dlp = dlp ;

	if ((rs = bopen(lfp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		cl ;
	    const char	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadlines(lfp,lbuf,llen,NULL)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

	        cp = lbuf ;
	        cl = len ;
	        while (cl && CHAR_ISWHITE(*cp)) {
	            cp += 1 ;
	            cl -= 1 ;
	        }

	        if ((cp[0] == '\0') || (cp[0] == '#'))
	            continue ;

	        rs = procline(lip,cp,cl) ;
		c += rs ;

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    bclose(lfp) ;
	} /* end if (file) */

#if	CF_DEBUGS
	debugprintf("defproc: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (defproc) */


/* local subroutines */


static int procline(SUBINFO *lip,cchar *lbuf,int llen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl = llen ;
	int		cl ;
	int		len = 0 ;
	int		f_done = FALSE ;
	const char	*tp, *cp ;
	const char	*sp = lbuf ;
	const char	*enp ;

	while (sl && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

/* extract any dependencies (if we have any) */

	tp = strnpbrk(sp,sl," \t?+=#") ;

	if ((tp != NULL) && (*tp == '?')) {
	    if ((rs1 = checkdeps(lip,sp,(tp - sp))) > 0) {
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	        while (sl && CHAR_ISWHITE(*sp)) {
	            sp += 1 ;
	            sl -= 1 ;
	        }
	        tp = strnpbrk(sp,sl," \t+=#") ;
	    } else {
		f_done = TRUE ;
	    }
	} /* end if (getting dependencies) */

#if	CF_DEBUGS
	debugprintf("defproc/procline: rem=>%t<\n",sp,sl) ;
	debugprintf("defproc/procline: t=%02x t=>%c<\n",
	    *tp,((isprintlatin(*tp)) ? *tp : 0xbf)) ;
#endif

	if (! f_done) {

	cl = 0 ;
	if ((tp != NULL) && ((*tp == '=') || CHAR_ISWHITE(*tp))) {

	    cl = sfshrink(sp,(tp - sp),&cp) ;

#if	CF_DEBUGS
	    debugprintf("defproc/procline: 1 def=>%t<\n",cp,cl) ;
#endif

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	} /* end if (delimiter) */

	if (cl > 0) {
	    const int	nrs = SR_NOTFOUND ;
	    if ((rs = vecstr_findn(lip->dlp,cp,cl)) == nrs) {
	        BUFFER	b ;
	        char	envname[ENVNAMELEN + 1] ;

	        enp = envname ;
	        snwcpy(envname,ENVNAMELEN,cp,cl) ;

/* loop processing the values */

	        if ((sl >= 0) && ((rs = buffer_start(&b,sl)) >= 0)) {

	            if ((rs = procvalues(lip,&b,ssb,sp,sl)) >= 0) {
	                if ((cl = buffer_get(&b,&cp)) > 0) {
	                    rs = vecstr_envadd(lip->dlp,enp,cp,cl) ;
	                }
	            }

	            len = buffer_finish(&b) ;
	            if (rs >= 0) rs = len ;
	        } /* end if (buffer) */
	    } /* end if (not-already-present) */
	} /* end if (non-zero) */

	} /* end if (not-done) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procline) */


static int checkdeps(SUBINFO *lip,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		rs1 = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    int		fl ;
	    const char	*fp ;

	    while ((fl = field_get(&fsb,dterms,&fp)) >= 0) {
	        if (fl > 0) {
#if	CF_DEBUGS
	        debugprintf("defproc/checkdeps: dep=%t\n",fp,fl) ;
#endif
	        rs1 = getev(lip->envv,fp,fl,NULL) ;
#if	CF_DEBUGS
	        debugprintf("defproc/checkdeps: getev() rs=%d\n",rs1) ;
#endif
		}
	        if (rs1 < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? (rs1 >= 0) : rs ;
}
/* end subroutine (checkdeps) */


/* process definition values */
static int procvalues(SUBINFO *lip,BUFFER *bp,cchar *ss,cchar *sp,int sl)
{
	FIELD		fsb ;
	int		rs ;
	int		len = 0 ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	    const int	flen = FBUFLEN ;
	    int		fl ;
	    int		c = 0 ;
	    const char	*fp ;
	    char	fbuf[FBUFLEN + 1] ;

	    fp = fbuf ;
	    while ((fl = field_sharg(&fsb,fterms,fbuf,flen)) >= 0) {
	        if (fl > 0) {

#if	CF_DEBUGS
	            debugprintf("defproc/procvalues: val=%t\n",fp,fl) ;
#endif

	            if (c++ > 0) {
	                rs = buffer_char(bp,' ') ;
	                len += rs ;
	            }

	            if (rs >= 0) {
	                rs = procvalue(lip,bp,ss,fp,fl) ;
	                len += rs ;
	            }

	        } /* end if (had a value) */
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while (looping over values) */

	    field_finish(&fsb) ;
	} /* end if (fields) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procvalues) */


static int procvalue(SUBINFO *lip,BUFFER *bp,cchar *ss,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		kl, cl ;
	int		len = 0 ;
	const char	*kp, *cp ;

#if	CF_DEBUGS
	{
	    debugprintf("defproc/procvalue: s=>%t<\n",sp,sl) ;
	    kl = sfthing(sp,sl,ssb,&kp) ;
	    debugprintf("defproc/procvalue: sfthing() kl=%d\n",kl) ;
	}
#endif

	while ((kl = sfthing(sp,sl,ss,&kp)) >= 0) {

	    cp = sp ;
	    cl = ((kp - 2) - sp) ;

#if	CF_DEBUGS
	    debugprintf("defproc/procvalue: leader=>%t<\n",cp,cl) ;
#endif

	    if (cl > 0) {
	        rs = buffer_strw(bp,cp,cl) ;
	        len += rs ;
	    }

	    if ((rs >= 0) && (kl > 0)) {
	        rs = procsubenv(lip,bp,kp,kl) ;
	        len += rs ;
	    }

	    sl -= ((kp + kl + 1) - sp) ;
	    sp = (kp + kl + 1) ;

#if	CF_DEBUGS
	    debugprintf("defproc/procvalue: remainder=>%t<\n",
	        sp,((sp[sl - 1] == '\n') ? (sl - 1) : sl)) ;
#endif

	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = buffer_strw(bp,sp,sl) ;
	    len += rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procvalue) */


static int procsubenv(SUBINFO *lip,BUFFER *bp,cchar *kp,int kl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (kl >= 0) {
	    int		al = 0 ;
	    int		cl ;
	    const char	*ap = NULL ;
	    const char	*tp, *cp ;

	    if ((tp = strnchr(kp,kl,'=')) != NULL) {
	        ap = (tp + 1) ;
	        al = (kp + kl) - (tp + 1) ;
	        kl = (tp - kp) ;
	    }

/* lookup the environment key-name that we have */

	    if ((cl = getev(lip->envv,kp,kl,&cp)) > 0) {
	        rs = buffer_strw(bp,cp,cl) ;
	        len += rs ;
	    } else if (al > 0) {
	        rs = buffer_strw(bp,ap,al) ;
	        len += rs ;
	    }

	} /* end if (non-zero) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procsubenv) */


