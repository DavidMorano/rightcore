/* varsub_extras */

/* variable manipulation (like for configuration files!) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-12, David A­D­ Morano
        This subroutine was written to replace some older variable substitution
        stuff from the old Automation Rsearch days with the old SAT tool stuff.

	= 2001-09-11, David A­D­ Morano
        This junk is not aging well! And it was really rewritten from
        essentially scratch in 1997 (only a few years ago)! This code has many
        qualities that are common with crap code. When a new variable was
        "exported" from a configuration file and it didn't have any value part
        specified, and when the same variable was not in the existing
        environment (like if a daemon is executed directly from 'init(1m)' or
        something similar), then the variable name would end up in the exported
        environment variables list as just a variable key with no value! I fixed
        this by not putting anything into the exported environment if it doesn't
        have a value (at least a value of zero length). I *think* that a NULL
        value and and a zero-length value distinquish the case of the variable
        having an empty value string from one where it did not have any value
        specified at all.

*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These routines operate on 'defined' and 'export'ed variables from a
        configuration file. What we want to do is to perform a substitution on
        these variables from existing variables in the database or from the
        environment otherwise. Variables that are already in the database get
        merged like in a path string with colon characters (':') separating the
        original values of the variables.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif


/* external subroutines */

extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern cchar	*getenv3(cchar *,int,cchar **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* forward references */

static int	cmpkey(const char *,const char *,int) ;
static int	cmpvalue(const char *,const char *,int) ;


/* local variables */


/* exported subroutines */


/* load from VECTSR (vector of strings) object */
int varsub_addvec(varsub *op,VECSTR *vsp)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	const char	*tp, *kp, *vp ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("varsub_addvec: ent\n") ;
#endif

	for (i = 0 ; vecstr_get(vsp,i,&sp) >= 0 ; i += 1) {
	    if (sp != NULL) {

	    kp = sp ;
	    if ((tp = strchr(sp,'=')) != NULL) {
		int	ch = MKCHAR(kp[0]) ;
		int	f ;

		f = isprintlatin(ch) ;
	        vp = (tp + 1) ;
	        if (f) {
		    ch = MKCHAR(vp[0]) ;
		    f = ((ch == '\0') || isprintlatin(ch)) ;
	        }
	        if (f) {
	            rs = varsub_add(op,kp,(tp - kp),vp,-1) ;
		    if (rs < INT_MAX) c += 1 ;
	        } /* end if */

	    } /* end if */

	    } /* end if (non-null) */
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("varsub_addvec: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varsub_addvec) */


int varsub_subbuf(var1p,var2p,s1,s1len,s2,s2len)
varsub	*var1p, *var2p ;
char	*s1, *s2 ;
int	s1len, s2len ;
{
	int		rs ;
	int		rl = 0 ;

	if (var1p == NULL) return SR_FAULT ;
	if (s1 == NULL) return SR_FAULT ;
	if (s2 == NULL) return SR_FAULT ;

	rs = varsub_expand(var1p,s2,s2len,s1,s1len) ;
	rl = rs ;

	if ((var2p != NULL) && (rs <= 0)) {
	    rs = varsub_expand(var2p,s2,s2len,s1,s1len) ;
	    rl = rs ;
	}

#if	CF_DEBUGS
	debugprintf("varsub_subbuf: ret rs=%d rl=%d\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (varsub_subbuf) */


/* merge variables that are the same into just one (and update all DBs) */
int varsub_merge(varp,vsp,buf,buflen)
varsub		*varp ;
vecstr		*vsp ;
char		buf[] ;
int		buflen ;
{
	int		rs = SR_OK ;
	int		size ;
	int		i ;
	int		klen, vlen ;
	int		olen, nlen ;
	int		f_novalue = TRUE ;
	int		f_done = FALSE ;
	cchar		*kp, *vp ;
	cchar		*tp, *sp, *cp ;
	cchar		*ep ;
	char		*buf2  = NULL ;

#if	CF_DEBUGS
	debugprintf("varsub_merge: ent\n") ;
#endif

/* separate the string into its key and value */

	ep = buf ;
	kp = buf ;
	vp = NULL ;
	vlen = -1 ;
	f_novalue = TRUE ;
	if ((tp = strnchr(buf,buflen,'=')) != NULL) {
	    klen = tp - buf ;
	    f_novalue = FALSE ;
	    cp = buf + klen ;
	    vp = cp + 1 ;
	    vlen = strnlen(vp,(buflen - klen - 1)) ;
	} else {
	    klen = strnlen(buf,buflen) ;
	}

#if	CF_DEBUGS
	debugprintf("varsub_merge: kp=\"%s\" klen=%d\n",kp,klen) ;
	debugprintf("varsub_merge: vp=\"%s\" vlen=%d\n",vp,vlen) ;
#endif /* CF_DEBUGS */


/* search for this key in the given vector string list (exports) */

	for (i = 0 ; (rs = vecstr_get(vsp,i,&sp)) >= 0 ; i += 1) {
	    if (sp != NULL) {
	        if (cmpkey(sp,kp,klen) == 0) break ;
	    }
	} /* end for */

	nlen = klen + ((vlen >= 0) ? (vlen + 1) : 0) ;

#if	CF_DEBUGS
	debugprintf("varsub_merge: nlen=%d klen=%d vlen=%d\n",
	    nlen,klen,vlen) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("varsub_merge: found it already\n") ;
#endif

	    if (vlen >= 0) {

#if	CF_DEBUGS
	        debugprintf("varsub_merge: value has non-negative length\n") ;
#endif

/* we already had an entry like this one */

/* do we have this particular value in this variable's value list already? */

	        f_done = (cmpvalue(sp,vp,vlen) == 0) ;

/* prepare to add this new value to the end of the existing values! */

		if (! f_done) {

	        olen = strlen(sp) ;
	        nlen = olen + ((vlen >= 0) ? (vlen + 1) : 0) ;
		size = (nlen + 1) ;
		if ((rs = uc_malloc(size,&buf2)) >= 0) {

	        ep = buf2 ;
	        kp = buf2 ;
	        strcpy(buf2,sp) ;

/* delete it from the vector string list */

#if	CF_DEBUGS
	        debugprintf("varsub_merge: deleting from VECSTR\n") ;
#endif

	        vecstr_del(vsp,i) ;

/* delete the old one from the substitution array */

	        if (varp != NULL) {
#if	CF_DEBUGS
	            debugprintf("varsub_merge: deleting from VARSUB\n") ;
#endif
		    varsub_del(varp,kp,klen) ;
#if	CF_DEBUGS
	            debugprintf("varsub_merge: deleted from VARSUB\n") ;
#endif
	        } /* end if (deleting from VARSUB DB) */

/* add this new part to the end of what we already have */

#if	CF_DEBUGS
	        debugprintf("varsub_merge: rest 1\n") ;
#endif

	        buf2[olen] = ':' ;
	        strwcpy((buf2 + olen + 1),vp,vlen) ;

	        vp = buf2 + klen + 1 ;
	        vlen = nlen - klen - 1 ;

		} else {
		    buf2 = NULL ;
		} /* end if (m-a) */

		} /* end if (not done) */
	    } else
	        f_done = TRUE ;

	} else {

#if	CF_DEBUGS
	    debugprintf("varsub_merge: we did not have it already\n") ;
#endif

/* it did NOT exist already */

	    if (vlen < 0) {

	        vlen = 0 ;
	        if ((vp = getenv3(kp,klen,&ep)) != NULL) {

/* we let anyone who cares to figure out these lengths by themselves */

	            nlen = -1 ;
	            vlen = -1 ;

	        } /* end if (we had it as an environment variable) */

#if	CF_DEBUGS
	        debugprintf("varsub_merge: getenv3, vp=%s\n",vp) ;
#endif

	    } /* end if (vlen < 0) */

	} /* end if */

/* add the new variable to the various DBs */

	if ((rs >= 0) && (! f_done) && (vp != NULL)) {

/* add the new string variable to the running string list */

#if	CF_DEBUGS
	    debugprintf("varsub_merge: bottom part, ep=\"%s\" nlen=%d\n",
	        ep,nlen) ;
#endif

	    if ((rs = vecstr_add(vsp,ep,nlen)) < 0) {

#if	CF_DEBUGS
	        debugprintf("varsub_merge: returning after vecstr_add\n") ;
#endif

	        return rs ;
	    }

/* add the new string variable to the variable_substitution_array */

#if	CF_DEBUGS
	    debugprintf("varsub_merge: add to varsub (for defines)\n") ;
	    debugprintf("varsub_merge: klen=%d kp=%W\n",klen,kp,klen) ;
	    debugprintf("varsub_merge: vlen=%d vp=%W\n",vlen,vp,vlen) ;
#endif /* CF_DEBUGS */

	    if ((varp != NULL) && (vlen >= 0)) {

#if	CF_DEBUGS
	        debugprintf("varsub_merge: adding to the VARSUB DB\n") ;
#endif

	        rs = varsub_add(varp,kp,klen,vp,vlen) ;

#if	CF_DEBUGS && 0
	{
		int	rs1 ;
	        rs1 = varsub_find(varp,kp,klen,&cp2,&len2) ;
	        debugprintf("varsub_merge: was it indeed added rs=%d kp=%s\n",
	            rs1,kp) ;
	}
#endif /* CF_DEBUGS */

	    } /* end if (adding to the variable substitution VARSUB DB) */

#if	CF_DEBUGS
	    debugprintf("varsub_merge: freeing buf\n") ;
#endif

	} /* end if (adding new variable to DBs) */

	if (buf2 != NULL)
	    uc_free(buf2) ;

#if	CF_DEBUGS && 0
	if (varp != NULL) {
	    int	len1 ;
	    char	*cp1 ;
	    debugprintf("varsub_merge: VSA so far :\n") ;
	    for (i = 0 ; 
	        varsub_get(varp,i,&cp1,&len1,&cp2,&len2) >= 0 ; i += 1) {
	        if (cp1 == NULL)
	            debugprintf("varsub_merge: kp=NULL\n") ;
	        else
	            debugprintf("varsub_merge: kp=%s vp=%s\n",cp1,cp2) ;
	    } /* end for */
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("varsub_merge: ret rs=%d f_novalue=%u\n",rs,f_novalue) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? f_novalue : rs ;
}
/* end subroutine (varsub_merge) */


/* private subroutines */


/* our own key comparison routine (to handle the '=' character) */
static int cmpkey(cchar *s,cchar *k,int klen)
{
	int		rc = -1 ;

#if	CF_DEBUGS
	debugprintf("cmpkey: s=\"%s\" k=\"%s\" klen=%d\n",
	    s,k,klen) ;
#endif

	if ((strncmp(s,k,klen) == 0) && (s[klen] == '=')) {
	    rc = 0 ;
	}

#if	CF_DEBUGS
	debugprintf("cmpkey: ret rc=%d\n",rc) ;
#endif

	return rc ;
}
/* end subroutine (cmpkey) */


/* compare a new value with the exiting values of a variable */
static int cmpvalue(cchar *sp,cchar *vp,int vlen)
{
	char		*cp ;

#if	CF_DEBUGS
	debugprintf("var/cmpvalue: ent\n") ;
	debugprintf("var/cmpvalue: sp=%s\n",sp) ;
	debugprintf("var/cmpvalue: vp=%W\n",vp,vlen) ;
#endif

	if ((cp = strchr(sp,'=')) == NULL)
	    return -1 ;

	sp = cp + 1 ;

#if	CF_DEBUGS
	debugprintf("var/cmpvalue: 2 sp=%s\n",sp) ;
#endif

	while (*sp) {

	    if ((strncmp(sp,vp,vlen) == 0) &&
	        ((sp[vlen] == '\0') || (sp[vlen] == ':')))
	        return 0 ;

	    if ((cp = strchr(sp,':')) == NULL)
	        break ;

	    sp = cp + 1 ;

	} /* end while */

	return -1 ;
}
/* end subroutine (cmpvalue) */


