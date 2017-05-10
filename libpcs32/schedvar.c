/* schedvar */

/* creates the substitution varaiables for 'scheduled' type operations */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGLIST	1		/* extra debugging */
#define	CF_SAFE		1		/* short-cut faulting */
#define	CF_SINGLE	0		/* force only single-letter keys */
#define	CF_SEARCH	0		/* use searching? */
#define	CF_ENVADD	1		/* use 'vecstr_envadd(3dam)' */
#define	CF_SNCPY2W	1		/* use |sncpy2w(3dam)| */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object is used to set up the substitution variables for use
	by 'scheduled' types of operations.  These are operations that take a
	schedule of templates to follow in carrying out their objective.  A
	good example of a 'scheduled' operation is provided by the 'permsched'
	subroutine.  Check out that subroutine for more information.

	Notes:

	Note that the local subroutine 'mkvarstr()' is identical to 
	'sncpy3w(3dam)' with the middle string being '='.  Just saying ....


*******************************************************************************/


#define	SCHEDVAR_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"schedvar.h"


/* local defines */

#define	SCHEDVAR_NE	8		/* default number of entries */

#define	BUFLEN		(MAXPATHLEN + MAXNAMELEN + 1)
#define	MAXBUFLEN	(40 * MAXPATHLEN)
#define	KEYBUF(c)	(keybuf[0] = (c),keybuf[1] = '\0',keybuf)


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	sncpy3w(char *,int,const char *,const char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

#if	CF_SNCPY2W
#define		mkvarstr(dbuf,dlen,s1,s2,s2w) sncpy2w(dbuf,dlen,s1,s2,s2w)
#else /* CF_SNCPY2W */
#if	CF_ENVADD
#else
static int	mkvarstr(char *,int,const char *,const char *,int) ;
#endif
#endif /* CF_SNCPY2W */


/* local variables */


/* exported subroutines */


int schedvar_start(svp)
SCHEDVAR	*svp ;
{
	const int	ne = SCHEDVAR_NE ;
	int		rs ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("schedvar_start: ent\n") ;
#endif

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	opts = VECSTR_OSORTED ;
	rs = vecstr_start(svp,ne,opts) ;

	return rs ;
}
/* end subroutine (schedvar_start) */


int schedvar_finish(svp)
SCHEDVAR	*svp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	rs1 = vecstr_finish(svp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (schedvar_finish) */


int schedvar_add(svp,key,vp,vl)
SCHEDVAR	*svp ;
const char	key[] ;
const char	vp[] ;
int		vl ;
{
	int		rs ;
	int		i ;

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	if (key == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("schedvar_add: key=%s val=%t\n",key,vp,vl) ;
#endif

#if	CF_SINGLE
	if (key[1] != '\0')
	    return SR_INVALID ;
#endif

	if (vl < 0) vl = strlen(vp) ;

#if	CF_ENVADD
	{
	    if ((i = vecstr_finder(svp,key,vstrkeycmp,NULL)) >= 0) {
	        vecstr_del(svp,i) ;
	    }
	    rs = vecstr_envadd(svp,key,vp,vl) ;
	}
#else /* CF_ENVADD */
	{
	    const int	tlen = (strlen(key) + 1 + vl) ;

	    if (tlen <= MAXBUFLEN) {
	        char	tbuf[tlen+1] ;
	        if ((i = vecstr_finder(svp,key,vstrkeycmp,NULL)) >= 0) {
	            vecstr_del(svp,i) ;
		}
	        if ((rs = mkvarstr(tbuf,tlen,key,vp,vl)) >= 0) {
	            rs = vecstr_add(svp,tbuf,tlen) ;
		}
	    } else
	        rs = SR_TOOBIG ;

	}
#endif /* CF_ENVADD */

#if	CF_DEBUGS
	debugprintf("schedvar_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (schedvar_add) */


int schedvar_curbegin(svp,curp)
SCHEDVAR	*svp ;
SCHEDVAR_CUR	*curp ;
{

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (schedvar_curbegin) */


int schedvar_curend(svp,curp)
SCHEDVAR	*svp ;
SCHEDVAR_CUR	*curp ;
{


#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (schedvar_curend) */


int schedvar_enum(svp,curp,kbuf,klen,vbuf,vlen)
SCHEDVAR	*svp ;
SCHEDVAR_CUR	*curp ;
char		kbuf[], vbuf[] ;
int		klen, vlen ;
{
	int		rs = SR_OK ;
	int		i ;
	int		kl ;
	int		vl = 0 ;
	const char	*tp ;
	const char	*cp ;

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	kbuf[0] = '\0' ;
	if (vbuf != NULL)
	    vbuf[0] = '\0' ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;
	while (((rs = vecstr_get(svp,i,&cp)) >= 0) && (cp == NULL))
	    i += 1 ;

	if (rs >= 0) {

	    kl = -1 ;
	    if ((tp = strchr(cp,'=')) != NULL)
	        kl = (tp - cp) ;

	    rs = snwcpy(kbuf,klen,cp,kl) ;

	    if ((rs >= 0) && (vbuf != NULL) && (tp != NULL)) {
	        cp = (tp + 1) ;
	        rs = sncpy1(vbuf,vlen,cp) ;
	        vl = rs ;
	    }

	} /* end if */

	if (rs >= 0)
	    curp->i = i ;

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (schedvar_enum) */


int schedvar_findkey(svp,key,rpp)
SCHEDVAR	*svp ;
const char	key[] ;
const char	**rpp ;
{
	int		rs ;

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

#if	CF_SEARCH
	rs = vecstr_search(svp,key,vstrkeycmp,rpp) ;
#else
	rs = vecstr_finder(svp,key,vstrkeycmp,rpp) ;
#endif /* CF_SEARCH */

	return rs ;
}
/* end subroutine (schedvar_findkey) */


int schedvar_del(svp,key)
SCHEDVAR	*svp ;
const char	key[] ;
{
	int		rs ;

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	if ((rs = vecstr_finder(svp,key,vstrkeycmp,NULL)) >= 0) {
	    rs = vecstr_del(svp,rs) ;
	}

	return rs ;
}
/* end subroutine (schedvar_del) */


int schedvar_expand(svp,dbuf,dlen,sp,sl)
SCHEDVAR	*svp ;
char		dbuf[] ;
int		dlen ;
const char	sp[] ;
int		sl ;
{
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_SAFE
	if (svp == NULL) return SR_FAULT ;
#endif

	if (dbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("scedvar_expand: ent slen=%u s=%t\n",sl,sp,sl) ;
#endif

	dbuf[0] = '\0' ;
	if (dlen <= 0) return SR_TOOBIG ;

#ifdef	OPTIONAL
	rs = vecstr_sort(svp,vstrkeycmp) ;
#endif /* OPTIONAL */

#if	CF_DEBUGS && CF_DEBUGLIST
	{
	    int	i ;
	    debugprintf("scedvar_expand: list begin\n") ;
	    for (i = 0 ; vecstr_get(svp,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;
	        debugprintf("scedvar_expand: svar=>%s<\n",cp) ;
	    }
	    debugprintf("scedvar_expand: list end\n") ;
	}
#endif /* CF_DEBUGS */

/* do the deed */

	if (rs >= 0) {
	    SBUF	b ;
	    if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
	        const char	*lfp ;
	        const char	*fp, *cp ;
	        char		keybuf[2] ;

#if	CF_DEBUGS
	        debugprintf("scedvar_expand: for-before\n") ;
#endif

	        lfp = (sp + sl) ;
	        for (fp = sp ; (fp < lfp) && *fp && (rs >= 0) ; fp += 1) {

#if	CF_DEBUGS
	            debugprintf("scedvar_expand: char=>%c<\n",*fp) ;
#endif

	            if (*fp == '%') {

	                fp += 1 ;
	                if (! *fp)
	                    break ;

#if	CF_DEBUGS
	                debugprintf("scedvar_expand: key=>%c<\n",*fp) ;
#endif

	                if (*fp == '%') {

	                    rs = sbuf_char(&b,'%') ;

	                } else {

#if	CF_DEBUGS
	                    debugprintf("scedvar_expand: got key=>%c<\n",*fp) ;
#endif

#if	CF_SEARCH
	                    rs = vecstr_search(svp,KEYBUF(*fp),vstrkeycmp,&cp) ;
#else
	                    rs = vecstr_finder(svp,KEYBUF(*fp),vstrkeycmp,&cp) ;
#endif

#if	CF_DEBUGS
	                    debugprintf("scedvar_expand: rs=%d kv=>%s<\n",
	                        rs,cp) ;
#endif

	                    if (rs >= 0) {
	                        const char	*tp ;

	                        rs = 0 ;
	                        if ((tp = strchr(cp,'=')) != NULL) {

#if	CF_DEBUGS
	                            debugprintf("scedvar_expand: st v=%s\n",
	                                (cp2 + 1)) ;
#endif

	                            rs = sbuf_strw(&b,(tp+1),-1) ;

#if	CF_DEBUGS
	                            debugprintf("scedvar_expand: st rs=%d\n",
	                                rs) ;
#endif

	                        } /* end if (it had a value) */

	                    } /* end if */

	                } /* end if (tried to expand a key) */

	            } else {

#if	CF_DEBUGS
	                debugprintf("schedvar_expand: storing char=>%c<\n",
	                    *fp) ;
#endif

	                rs = sbuf_char(&b,*fp) ;

	            } /* end if */

#if	CF_DEBUGS
	            {
	                int	len = sbuf_getlen(&b) ;
	                debugprintf("schedvar_expand: for-bot rs=%d\n",
	                    rs) ;
	                debugprintf("schedvar_expand: dlen=%d buf=%s\n",
	                    len,buf) ;
	            }
#endif /* CF_DEBUGS */

	        } /* end for */

	        len = sbuf_finish(&b) ;
	        if (rs >= 0) rs = len ;
	    } /* end if (sbuf) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("schedvar_expand: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (schedvar_expand) */


/* private subroutines */


#if	CF_SNCPY2W
#else /* CF_SNCPY2W */
#if	CF_ENVADD
#else

static int mkvarstr(dbuf,dlen,k,vp,vl)
char		dbuf[] ;
int		dlen ;
const char	k[] ;
const char	vp[] ;
int		vl ;
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,key,-1) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(dbuf,dlen,i,'=') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,vp,vl) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkvarstr) */

#endif /* (! CF_ENVADD) */
#endif /* CF_SNCPY2W */


