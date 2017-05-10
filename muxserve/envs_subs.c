/* envs_subs */

/* process the cookie substitutions for environment variables */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_EXPAND	0		/* perform expand-cookie */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine takes raw strings that are supposed to be environment
        variables and processes any substitutions that are found in those
        strings. It also cleans up those environment variables that are supposed
        to be directory paths of some kind.

	Synopsis:

	int envs_subs(nlp,clp,pvp,evp)
	ENVS		*nlp ;
	EXPCOOK		*clp ;
	VECSTR		*pvp ;
	vecstr		*evp ;

	Arguments:

	nlp		new-list-pointer, new (forming) environment list
	clp		cookies list pointer
	pvp		pointer to VECSTR of path-vars
	evp		pointer to VECSTR of exportable environment

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
#include	<buffer.h>
#include	<sbuf.h>
#include	<expcook.h>
#include	<localmisc.h>

#include	"envs.h"


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

#define	DEFNAMELEN	120

#define	ENVNAMELEN	120

#define	EBUFLEN		1024

#define	BUFLEN		(4 * MAXPATHLEN)

#ifndef	DEFNPATHS
#define	DEFNPATHS	20
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	pathclean(char *,const char *,int) ;
extern int	vecstr_adduniq(VECSTR *,const char *,int) ;
extern int	vecstr_envadd(VECSTR *,const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strnnlen(cchar *,int,int) ;
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct subinfo {
	ENVS		*nlp ;
	EXPCOOK		*clp ;
	vecstr		*pvp ;
	vecstr		*evp ;
} ;


/* forward references */

static int	procvar(SUBINFO *,const char *,int) ;
static int	procvarnorm(SUBINFO *,const char *,int) ;
static int	procvarpath(SUBINFO *,const char *,int) ;
static int	procpathsplit(SUBINFO *,VECSTR *,const char *,int) ;
static int	procsub(SUBINFO *,BUFFER *,const char *,int) ;
static int	pathjoin(VECSTR *,char *,int) ;


/* local variables */


/* exported subroutines */


int envs_subs(ENVS *nlp,EXPCOOK *clp,VECSTR *pvp,vecstr *evp)
{
	SUBINFO		si, *sip = &si ;
	ENVS_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS && 0
	{
	    int	rs1 ;
	    const char	*cp ;
	    rs1 = expcook_findkey(clp,VARHOME,-1,&cp) ;
	    debugprintf("envs_subs: expcook_findkey() rs=%d HOME=%s\n",
	        rs1,cp) ;
	}
#endif /* CF_DEBUGS */

	sip->evp = evp ;
	sip->clp = clp ;
	sip->pvp = pvp ;
	sip->nlp = nlp ;

	if ((rs = envs_curbegin(nlp,&cur)) >= 0) {
	    int		kl ;
	    const char	*kp ;

	    while (rs >= 0) {

	        kl = envs_enumkey(nlp,&cur,&kp) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;

	        if (rs >= 0) {
	            c += 1 ;
	            rs = procvar(sip,kp,kl) ;
	        }

	    } /* end while */

	    rs1 = envs_curend(nlp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

#if	CF_DEBUGS
	debugprintf("envs_subs: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (envs_subs) */


/* local subroutines */


static int procvar(SUBINFO *sip,cchar *kp,int kl)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if ((rs1 = vecstr_findn(sip->pvp,kp,kl)) >= 0) {
	    rs = procvarpath(sip,kp,kl) ;
	} else if (rs1 == SR_NOTFOUND) {
	    rs = procvarnorm(sip,kp,kl) ;
	} else
	    rs = rs1 ;

	return rs ;
}
/* end subroutine (procvar) */


static int procvarnorm(SUBINFO *sip,cchar *kp,int kl)
{
	BUFFER		b ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = buffer_start(&b,EBUFLEN)) >= 0) {
	    ENVS_CUR	cur ;
	    int		bl ;
	    const char	*bp ;

	    if ((rs = envs_curbegin(sip->nlp,&cur)) >= 0) {
	        int		vl ;
	        const char	*vp ;

	        while (rs >= 0) {

	            vl = envs_fetch(sip->nlp,kp,kl,&cur,&vp) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;

	            if ((rs >= 0) && (vl > 0)) {

#ifdef	COMMENT
	                if (c++ > 0)
	                    rs = buffer_char(&b,' ') ;
#endif

	                if (rs >= 0) {
	                    rs = procsub(sip,&b,vp,vl) ;
			}

	            } /* end if */

	        } /* end while */

	        rs1 = envs_curend(sip->nlp,&cur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */

	    if (rs >= 0) {
	        if ((bl = buffer_get(&b,&bp)) >= 0) {
	            rs = vecstr_envadd(sip->evp,kp,bp,bl) ;
		}
	    } /* end if */

	    rs1 = buffer_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (buffer) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procvarnorm) */


static int procvarpath(SUBINFO *sip,cchar *kp,int kl)
{
	VECSTR		paths ;
	int		rs ;
	int		rs1 ;
	int		opts ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("envs_subs/procvarpath: k=>%t<\n",kp,kl) ;
#endif

	opts = (VECSTR_OCOMPACT | VECSTR_OSTSIZE) ;
	if ((rs = vecstr_start(&paths,DEFNPATHS,opts)) >= 0) {
	    BUFFER	b ;

	    if ((rs = buffer_start(&b,MAXPATHLEN)) >= 0) {
		ENVS_CUR	cur ;

/* split out into individual components */

	        if ((rs = envs_curbegin(sip->nlp,&cur)) >= 0) {
		    int		vl, bl ;
		    const char	*vp, *bp ;

	            while (rs >= 0) {
	                vl = envs_fetch(sip->nlp,kp,kl,&cur,&vp) ;
	                if (vl == SR_NOTFOUND) break ;
	                rs = vl ;

#if	CF_DEBUGS
		debugprintf("envs_subs/procvarpath: v=>%t</n",vp,vl) ;
#endif

	                if ((rs >= 0) && (vp[0] != ':')) {
	                    buffer_reset(&b) ;
	                    if ((rs = procsub(sip,&b,vp,vl)) >= 0) {
	                        if ((bl = buffer_get(&b,&bp)) >= 0) {
	                            rs = procpathsplit(sip,&paths,bp,bl) ;
				}
	                    } /* end if */
	                } /* end if */

	            } /* end while */

	            rs1 = envs_curend(sip->nlp,&cur) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (cursor) */

#if	CF_DEBUGS
		debugprintf("envs_subs/procvarpath: mid1 rs=%d\n",rs) ;
#endif

/* join the path-components */

	        if (rs >= 0) {
	            const int	size = vecstr_strsize(&paths) ;
	            const int	npaths = vecstr_count(&paths) ;
	            int		bufl ;
	            char	*bufp ;

	            bufl = (size + npaths + 1) ;
	            if ((rs = uc_malloc(bufl,&bufp)) >= 0) {
			cchar	*sp = bufp ;
			int	sl = 0 ;
	                if ((rs = pathjoin(&paths,bufp,bufl)) > 0) {
			    sl = rs ;
				while (sl && (sp[0] == ':')) {
				    sp += 1 ;
				    sl -= 1 ;
				}
			    len = sl ;
	                    rs = vecstr_envadd(sip->evp,kp,sp,sl) ;
#if	CF_DEBUGS
			    debugprintf("envs_subs/procvarpath: "
			       "vecstr_envadd() rs=%d\n", rs) ;
#endif
			} /* end if (pathjoin) */

	                uc_free(bufp) ;
	            } /* end if (allocation) */

	        } /* end if (ok) */

#if	CF_DEBUGS
		debugprintf("envs_subs/procvarpath: fin1 rs=%d\n",rs) ;
#endif

	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */

#if	CF_DEBUGS
		debugprintf("envs_subs/procvarpath: fin2 rs=%d\n",rs) ;
#endif

	    rs1 = vecstr_finish(&paths) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr-paths) */

#if	CF_DEBUGS
	debugprintf("envs_subs/procvarpath: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procvarpath) */


static int procpathsplit(SUBINFO *sip,VECSTR *plp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		pl, cl ;
	int		c = 0 ;
	const char	*tp, *cp ;
	char		pathbuf[MAXPATHLEN + 1] ;

	if (sip == NULL)
	    return SR_FAULT ;

	while ((tp = strnpbrk(vp,vl,":;")) != NULL) {

	    cp = vp ;
	    cl = (tp - vp) ;

#if	CF_DEBUGS
	debugprintf("procpathsplit: c=>%t<\n",cp,cl) ;
#endif

	        c += 1 ;
	        if ((pl = pathclean(pathbuf,cp,cl)) >= 0)
	            rs = vecstr_adduniq(plp,pathbuf,pl) ;

	    if ((rs >= 0) && (tp[0] == ';'))
	        rs = vecstr_adduniq(plp,";",1) ;

	    vl -= ((tp + 1) - vp) ;
	    vp = (tp + 1) ;

	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && ((c == 0) || (vl > 0))) {

	    c += 1 ;
	    if ((pl = pathclean(pathbuf,vp,(tp - vp))) >= 0)
	        rs = vecstr_adduniq(plp,pathbuf,pl) ;

	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procpathsplit) */


#if	CF_EXPAND
static int procsub(SUBINFO *sip,BUFFER *bp,cchar *vp,int vl)
{
	int		rs ;
	int		rs1 ;
	int		i ;
	int		buflen = MAX(EBUFLEN,vl) ;
	char		*buf = NULL ;
	char		*p ;

#if	CF_DEBUGS
	{
	    char	*cp ;
	    rs1 = expcook_findkey(sip->clp,VARHOME,-1,&cp) ;
	    debugprintf("envs_subs/procsub: "
	        "expcook_findkey() rs=%d HOME=%s\n",
	        rs1,cp) ;
	}
#endif /* CF_DEBUGS */

/* dynamic determination of acceptable buffer size */

	rs1 = SR_NOTFOUND ;
	for (i = 0 ; (rs = uc_malloc(buflen,&p)) >= 0 ; i += 1) {

	    buf = p ;
	    rs1 = expcook_exp(sip->clp,0,buf,buflen,vp,vl) ;
	    if (rs1 >= 0)
	        break ;

	    buflen += EBUFLEN ;

	    uc_free(buf) ;
	    buf = NULL ;
	} /* end while */

	if ((rs >= 0) && (rs1 > 0)) {
	    rs = buffer_strw(bp,buf,rs1) ;
	}

	if (buf != NULL)
	    uc_free(buf) ;

	return rs ;
}
/* end subroutine (procsub) */
#else /* CF_EXPAND */
static int procsub(SUBINFO *sip,BUFFER *bp,cchar *vp,int vl)
{
	return buffer_strw(bp,vp,vl) ;
}
/* end subroutine (procsub) */
#endif /* CF_EXPAND */


static int pathjoin(VECSTR *plp,char jbuf[],int jlen)
{
	SBUF		b ;
	int		rs ;
	int		c = 0 ;
	int		bl = 0 ;
	int		f_semi = FALSE ;

	if ((rs = sbuf_start(&b,jbuf,jlen)) >= 0) {
	    int		sc ;
	    int		i ;
	    const char	*cp ;

	    for (i = 0 ; vecstr_get(plp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {

#if	CF_DEBUGS
	        debugprintf("envs_subs/pathjoin: cp=>%s<\n",cp) ;
#endif

	        if (cp[0] != ';') {

	            if (c++ > 0) {
	                if (f_semi) {
	                    f_semi = FALSE ;
	                    sc = ';' ;
	                } else {
	                    sc = ':' ;
			}
	                rs = sbuf_char(&b,sc) ;
	            }

	            if (rs >= 0) {
	                rs = sbuf_strw(&b,cp,-1) ;
		    }

	        } else {
	            f_semi = TRUE ;
		}

		}
	        if (rs < 0) break ;
	    } /* end for */

	    bl = sbuf_finish(&b) ;
	    if (rs >= 0) rs = bl ;
	} /* end if (sbuf) */

#if	CF_DEBUGS
	debugprintf("envs_subs/pathjoin: ret rs=%d bl=%u\n",rs,bl) ;
#endif

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (pathjoin) */


