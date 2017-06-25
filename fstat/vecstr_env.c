/* vecstr_env */

/* environment-type string handling */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine extends the VECSTR object to add a key-value pair to the
        vector list. Key-value pairs are two strings (the key and the value)
        joined with an intervening equal sign character ('='). Duplicate entries
        are checked for (after all, this was originally intended for environment
        variables) and are not added if already present.

	Synopsis:

	int vecstr_envadd(op,kp,vp,vl)
	vecstr		*op ;
	cchar		*kp ;
	cchar		*vp ;
	int		vl ;

	Arguments:

	op		vecstr string to add to
	kp		pointer to key string
	vp		pointer to value string
	bl		length of value string

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	120
#endif


/* external subroutines */

extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	vstrkeycmp(cchar **,cchar **) ;

extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	vecstr_addwithin(vecstr *,cchar *,int) ;


/* local variables */


/* exported subroutines */


/* add a key=value string pair to the environment list */
int vecstr_envadd(vecstr *op,cchar *kp,cchar *vp,int vl)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		i = INT_MAX ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if ((rs = vecstr_finder(op,kp,vstrkeycmp,NULL)) == rsn) {
	    rs = vecstr_addkeyval(op,kp,-1,vp,vl) ;
	    i = rs ;
	} /* end if (was not found) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecstr_envadd) */


/* load a series of key=value pairs */
int vecstr_envadds(vecstr *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	cchar		*cp, *tp ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("vecstr_envadds: ent >%t<\n",
	    sp,strnlen(sp,sl)) ;
#endif

	while ((tp = strnpbrk(sp,sl," \t\r\n,")) != NULL) {
	    cp = sp ;
	    cl = tp - sp ;
	    if (cl > 0) {
	        rs = vecstr_addwithin(op,cp,cl) ;
	        if (rs > 0) c += 1 ;
	    } /* end if */
	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = vecstr_addwithin(op,sp,sl) ;
	    if (rs > 0) c += 1 ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("vecstr_envadds: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_envadds) */


/* set a key=value string pair to the environment list */
int vecstr_envset(vecstr *op,cchar *kp,cchar *vp,int vl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if ((rs = vecstr_finder(op,kp,vstrkeycmp,NULL)) >= 0) {
	    int		i = rs ;
	    vecstr_del(op,i) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	if (rs >= 0) {
	    rs = vecstr_addkeyval(op,kp,-1,vp,vl) ;
	}

	return rs ;
}
/* end subroutine (vecstr_envset) */


/* get a string value by searching on the key */
int vecstr_envget(vecstr *op,cchar *kp,cchar **rpp)
{
	int		rs ;
	int		vl = 0 ;
	cchar		*rp ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (rpp != NULL) *rpp = NULL ;

	if ((rs = vecstr_finder(op,kp,vstrkeycmp,&rp)) >= 0) {
	    cchar	*tp ;

	    if ((tp = strchr(rp,'=')) != NULL) {
	        vl = strlen(tp + 1) ;
	    }
	    if (rpp != NULL) {
	        if (tp != NULL) {
	            *rpp = (tp + 1) ;
	        } else {
	            *rpp = (rp + strlen(rp)) ; /* will be the NUL character */
		}
	    } /* end if */

	} /* end if (finder) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (vecstr_envget) */


/* private subroutines */


static int vecstr_addwithin(vecstr *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		f_added = FALSE ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("vecstr_addwithin: ent >%t<\n",
	    sp,strnlen(sp,sl)) ;
#endif

	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	while ((sl > 0) && CHAR_ISWHITE(sp[sl - 1])) {
	    sl -= 1 ;
	}

	if (sl > 0) {
	    int		kl = sl ;
	    int		vl = 0 ;
	    int		f_separated = FALSE ;
	    cchar	*kp = sp ;
	    cchar	*vp = NULL ;
	    cchar	*tp ;

	    if ((tp = strnchr(sp,sl,'=')) != NULL) {

	        vp = tp + 1 ;
	        vl = (kp + kl) - vp ;
	        kl = tp - kp ;

#if	CF_DEBUGS
	        debugprintf("vecstr_addwithin: K=V vl=%d\n",vl) ;
#endif

	        while ((kl > 0) && CHAR_ISWHITE(kp[kl - 1])) {
	            f_separated = TRUE ;
	            kl -= 1 ;
	        }

	        while ((vl > 0) && CHAR_ISWHITE(*vp)) {
	            f_separated = TRUE ;
	            vp += 1 ;
	            vl -= 1 ;
	        }

	        if (kl > 0) {
	            int		f ;
	            char	keybuf[KEYBUFLEN + 1] ;

	            f = (! f_separated) || 
	                (kp[kl] == '\0') || (kp[kl] == '=') ;

	            if (! f) {
	                if (kl <= KEYBUFLEN) {
	                    strwcpy(keybuf,kp,kl) ;
	                    kp = keybuf ;
	                } else {
	                    rs = SR_TOOBIG ;
			}
	            } /* end if */

	            if (rs >= 0) {
		        const int	rsn = SR_NOTFOUND ;
			int		(*vs)(cchar **,cchar **) ;

			vs = vstrkeycmp ;
	                if ((rs = vecstr_finder(op,kp,vs,NULL)) == rsn) {
	                    f_added = TRUE ;
	                    if (f_separated) {
	                        rs = vecstr_addkeyval(op,kp,kl,vp,vl) ;
	                    } else {
	                        rs = vecstr_add(op,sp,sl) ;
			    }
	                } /* end if (was not found) */

	            } /* end if */

	        } /* end if (non-zero key) */

	    } /* end if (had a value even if NUL) */

	} /* end if (non-zero string) */

#if	CF_DEBUGS
	debugprintf("vecstr_addwithin: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_added : rs ;
}
/* end subroutine (vecstr_addwithin) */


