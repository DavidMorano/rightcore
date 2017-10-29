/* bcspec (BC-Spec) */

/* load a Bible-Citation-Specification */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2013-02-07, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object parses and loads a given Bible-Citation-Specification string
        into itself. The given citation-specification string looks like:

		<bookname>[:]<chapter>[:<verse>]
	or
		<booknum>:<chapter>[:<verse>]

	Examples are:

		mat5:1
		mat:5:1
		40:5:1


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"bcspec.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sialnum(const char *,int) ;
extern int	sialpha(const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasalldig(const char *,int) ;
extern int	isalphalatin(int) ;
extern int	isalnumlatin(int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strncasestr(const char *,int,const char *) ;


/* external variables */


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif


/* forward references */

static int	siourbrk(const char *,int,int) ;


/* local variables */


/* exported subroutines */


int bcspec_load(BCSPEC *op,const char *sbuf,int slen)
{
	int		rs = SR_OK ;
	int		sl ;
	cchar		*sp ;

	if (op == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	memset(op,0,sizeof(BCSPEC)) ;
	op->c = 1 ;
	op->v = 1 ;

	if ((sl = sfshrink(sbuf,slen,&sp)) > 0) {
	    int	si ;
	    int	v ;
	    int	ch = MKCHAR(sp[0]) ;
	    if (isalphalatin(ch)) {
		op->np = sp ;
	  	op->nl = sl ;
#if	CF_DEBUGS
		debugprintf("bcspec_load: s=%t\n",sp,sl) ;
#endif
	        if ((si = siourbrk(sp,sl,TRUE)) > 0) {
#if	CF_DEBUGS
		    debugprintf("bcspec_load: dig si=%u\n",si) ;
#endif
	  	    op->nl = si ;
		    sp += si ;
		    sl -= si ;
		    if (sl > 0) {
			ch = MKCHAR(sp[0]) ;
		 	if (ch == ':') {
			    sp += 1 ;
			    sl -= 1 ;
			}
		    }
	        } else {
#if	CF_DEBUGS
		    debugprintf("bcspec_load: no_dig\n") ;
#endif
		    sl = 0 ;
		}
	    } else if (isdigitlatin(ch)) {
	        if ((si = siourbrk(sp,sl,TRUE)) > 0) {
		    rs = cfdeci(sp,si,&v) ;
		    op->b = v ;
	        }
	    } else {
	        rs = SR_DOM ;
	    }
	    if ((rs >= 0) && (sl > 0)) {
		cchar	*tp ;
		if ((tp = strnchr(sp,sl,':')) != NULL) {
		    if ((rs = cfdeci(sp,(tp-sp),&v)) >= 0) {
			op->c = v ;
			sl -= ((tp+1)-sp) ;
			sp = (tp+1) ;
			if (sl > 0) {
		    	    rs = cfdeci(sp,sl,&v) ;
		    	    op->v = v ;
			}
		    }
		} else {
		    rs = cfdeci(sp,sl,&v) ;
		    op->c = v ;
		}
	    }
	} else
	    rs = SR_DOM ;

#if	CF_DEBUGS
	debugprintf("bcspec_parse: ret rs=%d\n",rs) ;
	debugprintf("bcspec_parse: ret b=%d c=%d v=%d\n",op->v,op->c,op->v) ;
#endif

	return rs ;
}
/* end subroutine (bcspec_load) */


/* local subroutines */


static int siourbrk(cchar *sp,int sl,int f_dig)
{
	int		i = -1 ;
	int		ch ;
	int		f = FALSE ;

	for (i = 0 ; i < sl ; i += 1) {
	    ch = MKCHAR(sp[i]) ;
	    if (f_dig) {
		f = isdigitlatin(ch) ;
	    } else {
		f = isalphalatin(ch) ;
	    }
	    f = f || (ch == ':') ;
	    if (f) break ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (siourbrk) */


