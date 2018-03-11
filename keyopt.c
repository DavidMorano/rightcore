/* keyopt */

/* paramater option manipulations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This code module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is similar to the PARAMOPT object in some ways.  This
	object does not provide (natually) any exponetial enumeration of the
	gather options (like PARAMOPT does).  Rather it allows several options
	(different options) to be specified together (or strung together)
	separated by commas.  This is actually quite natural for most
	applications.


*******************************************************************************/


#define	KEYOPT_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"keyopt.h"


/* local defines */

#define	KEYBUFLEN	100


/* external subroutines */

extern int	sncpy(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	strkeycmp(const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local subroutines */


/* forward references */

int		keyopt_loads(KEYOPT *,const char *,int) ;
int		keyopt_load(KEYOPT *,const char *,int) ;
int		keyopt_loadvalue(KEYOPT *,const char *,const char *,int) ;

#ifdef	COMMENT
int		keyopt_loadx() ;
#endif

static int	keyopt_findkey(KEYOPT *,const char *,int,KEYOPT_NAME **) ;
static int	keyopt_loadpair(KEYOPT *,const char *,int) ;

static int	keyname_incri(KEYOPT_NAME *) ;
static int	keyname_findv(KEYOPT_NAME *,const char *,int,KEYOPT_VALUE **) ;


/* local variables */


/* exported subroutines */


/* initialize a parameter structure */
int keyopt_start(KEYOPT *php)
{

	if (php == NULL) return SR_FAULT ;

	php->head = NULL ;
	php->tail = NULL ;
	php->magic = KEYOPT_MAGIC ;
	return SR_OK ;
}
/* end subroutine (keyopt_start) */


/* initialize a parameter structure */
int keyopt_finish(KEYOPT *php)
{
	KEYOPT_NAME	*np, *nnp ;
	KEYOPT_VALUE	*vp, *nvp ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (php == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	for (np = php->head ; np != NULL ; np = nnp) {

	    for (vp = np->head ; vp != NULL ; vp = nvp) {
	        if (vp->value != NULL) {
	            rs1 = uc_free(vp->value) ;
		    if (rs >= 0) rs = rs1 ;
	        }
	        nvp = vp->next ;
	        rs1 = uc_free(vp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end for */

	    if (np->name != NULL) {
	        rs1 = uc_free(np->name) ;
		if (rs >= 0) rs = rs1 ;
	    }

	    nnp = np->next ;
	    rs1 = uc_free(np) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	php->head = NULL ;
	php->tail = NULL ;
	php->magic = 0 ;
	return rs ;
}
/* end subroutine (keyopt_finish) */


/* load a series of key-value pairs */
int keyopt_loads(KEYOPT *php,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*cp, *tp ;

	if (php == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	if (sl <= 0)
	    sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("keyopt_loads: ent >%t<\n",
	    sp,strnlen(sp,sl)) ;
#endif

	while ((tp = strnpbrk(sp,sl,",\t\n\r ")) != NULL) {

	    cp = sp ;
	    cl = tp - sp ;
	    if (cl > 0) {
	        rs = keyopt_loadpair(php,cp,cl) ;
	        c += rs ;
	    }

	    sl -= ((tp + 1) - sp) ;
	    sp = (tp + 1) ;

	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = keyopt_loadpair(php,sp,sl) ;
	    c += rs ;
	}

#if	CF_DEBUGS
	debugprintf("keyopt_loads: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (keyopt_loads) */


/* load a single pair */
int keyopt_load(KEYOPT *php,cchar *sp,int sl)
{
	int		rs ;

	if (php == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	rs = keyopt_loadpair(php,sp,sl) ;

	return rs ;
}
/* end subroutine (keyopt_load) */


/* load a single key with a possible value */
int keyopt_loadvalue(KEYOPT *php,cchar *key,cchar *vbuf,int vlen)
{
	KEYOPT_VALUE	*vp, *nvp ;
	KEYOPT_NAME	*pp, *tpp ;
	int		rs = SR_OK ;
	int		klen ;
	const char	*tp ;
	const char	*cp ;

	if (php == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

/* clean up the value a little */

	if (vlen < 0)
	    vlen = (vbuf != NULL) ? strlen(vbuf) : 0 ;

#if	CF_DEBUGS
	debugprintf("keyopt_loadvalue: 2 vlen=%d vbuf=>%t<\n",
	    vlen,vbuf,vlen) ;
#endif

/* do we have one of these named keys already? */

	klen = -1 ;
	if ((tp = strchr(key,'=')) != NULL) {
	    klen = (tp - key) ;
	}

	if (keyopt_findkey(php,key,klen,&pp) == SR_NOTFOUND) {
	    const int	nsize = sizeof(KEYOPT_NAME) ;

#if	CF_DEBUGS
	    debugprintf("keyopt_loadvalue: did not find already\n") ;
#endif

/* make a new parameter header block */

	    if ((rs = uc_malloc(nsize,&pp)) >= 0) {

	        pp->count = 0 ;
	        pp->next = NULL ;
	        pp->head = NULL ;
	        pp->tail = NULL ;
	        pp->current = NULL ;

#if	CF_DEBUGS
	        debugprintf("keyopt_loadvalue: storing=%s\n",key) ;
#endif

	        if ((rs = uc_mallocstrw(key,klen,&cp)) >= 0) {
	            pp->name = cp ;
	            tpp = php->tail ;
	            php->tail = pp ;
	            if (tpp != NULL)
	                tpp->next = pp ;
	            if (php->head == NULL)
	                php->head = pp ;
	        } else {
	            uc_free(pp) ;
	            pp = NULL ;
	        }

	    } /* end if (memory allocation) */

	} /* end if (adding a new parameter block on the list) */

/* OK, now we have the parameter block that we are looking for in 'pp' */

	if (rs >= 0) {
	    const int	vsize = sizeof(KEYOPT_VALUE) ;
	    if ((rs = uc_malloc(vsize,&nvp)) >= 0) {

	    nvp->next = NULL ;
	    nvp->value = NULL ;
	    if (vbuf != NULL) {
	        if ((rs = uc_mallocstrw(vbuf,vlen,&cp)) >= 0) {
	            nvp->value = cp ;
	        } else {
	            uc_free(nvp) ;
		}
	    } /* end if (new value) */

	    if (rs >= 0) {
	        if (pp->head != NULL) {
	            vp = pp->tail ;
	            vp->next = nvp ;
	            pp->tail = nvp ;
	        } else {
	            pp->head = nvp ;
	            pp->tail = nvp ;
	        }
	        pp->count += 1 ;
	    } /* end if */

	    if (rs >= 0) php->count += 1 ;

	    } /* end if */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("keyopt_loadvalue: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (keyopt_loadvalue) */


int keyopt_count(KEYOPT *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	return op->count ;
}
/* end subroutine (keyopt_count) */


/* enumerate on all of the keys */
int keyopt_enumkeys(KEYOPT *php,KEYOPT_CUR *curp,cchar **rpp)
{
	KEYOPT_NAME	*kp ;
	int		rs = SR_NOTFOUND ;

	if (php == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if (curp->keyp == NULL) {
	    kp = php->head ;
	    curp->valuep = NULL ;
	} else {
	    kp = curp->keyp ;
	    kp = kp->next ;
	}

	curp->keyp = kp ;

#if	CF_DEBUGS
	debugprintf("keyopt_enumkeys: enum midway\n") ;
#endif

	if (kp != NULL) {
	    if (rpp != NULL) *rpp = kp->name ;
	    rs = strlen(kp->name) ;
	} /* end if */

	return rs ;
}
/* end subroutine (keyopt_enumkeys) */


int keyopt_fetch(KEYOPT *php,cchar *kname,KEYOPT_CUR *curp,cchar **rpp)
{
	KEYOPT_NAME	*kp ;
	KEYOPT_VALUE	*vp ;
	KEYOPT_CUR	dcur ;
	int		rs = SR_OK ;
	int		klen ;
	const char	*tp ;

	if (php == NULL) return SR_FAULT ;
	if (kname == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("keyopt_fetch: key=%s\n",kname) ;
#endif

	if (curp == NULL) {
	    curp = &dcur ;
	    curp->keyp = NULL ;
	    curp->valuep = NULL ;
	}

	if (curp->keyp == NULL) {

#if	CF_DEBUGS
	    debugprintf("keyopt_fetch: cursor-NULL\n") ;
#endif

/* do we have this key? */

	    klen = -1 ;
	    if ((tp = strchr(kname,'=')) != NULL) {
	        klen = (tp - kname) ;
	    }

	    if ((rs = keyopt_findkey(php,kname,klen,&kp)) >= 0) {
	        curp->keyp = kp ;
	        vp = kp->head ;
	    }

	} else {

#if	CF_DEBUGS
	    debugprintf("keyopt_fetch: cursor-nonNULL\n") ;
#endif

	    kp = curp->keyp ;

#if	CF_DEBUGS
	    debugprintf("keyopt_fetch: getting value\n") ;
#endif

	    vp = NULL ;
	    if (curp->valuep != NULL) {
	        vp = (curp->valuep)->next ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("keyopt_fetch: midway\n") ;
#endif

	if (rpp != NULL)
	    *rpp = NULL ;

	if (rs >= 0) {

	    if ((kp == NULL) || (vp == NULL)) {
	        rs = SR_NOENT ;
	    }

	    if (rs >= 0) {

	        if (rpp != NULL)
	            *rpp = vp->value ;

	        rs = 0 ;
	        if (vp->value != NULL) {

#if	CF_DEBUGS
	            debugprintf("keyopt_fetch: v=%t\n",
	                vp->value,strnlen(vp->value,50)) ;
#endif

	            rs = strlen(vp->value) ;

	        }

	        curp->valuep = vp ;

	    } /* end if */

	} /* end if (got one) */

	return rs ;
}
/* end subroutine (keyopt_fetch) */


int keyopt_enumvalues(KEYOPT *php,cchar *key,KEYOPT_CUR *curp,cchar **rpp)
{
	int		rs ;

	rs = keyopt_fetch(php,key,curp,rpp) ;

	return rs ;
}
/* end subroutine (keyopt_enumvalues) */


/* increment the parameters */
int keyopt_incr(KEYOPT *php)
{
	KEYOPT_NAME	*pp ;
	KEYOPT_VALUE	*vp ;
	int		rs = SR_NOTFOUND ;

	if (php == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	pp = php->head ;
	if (pp->next != NULL) {
	    rs = keyname_incri(pp->next) ;
	}

/* increment ourselves if we are at bottom or if previous guy carried */

	if (rs == SR_NOTFOUND) {
	    vp = pp->current ;
	    if (vp->next == NULL) {
	        pp->current = pp->head ;
	        rs = -1 ;
	    } else {
	        pp->current = vp->next ;
	        rs = 0 ;
	    }
	} /* end if (not found) */

	return rs ;
}
/* end subroutine (keyopt_incr) */


/* initialize a cursor */
int keyopt_curbegin(KEYOPT *php,KEYOPT_CUR *curp)
{

	if (php == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	curp->keyp = NULL ;
	curp->valuep = NULL ;
	return SR_OK ;
}
/* end subroutine (keyopt_curbegin) */


/* free up a cursor */
int keyopt_curend(KEYOPT *php,KEYOPT_CUR *curp)
{

	if (php == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	curp->keyp = NULL ;
	curp->valuep = NULL ;
	return SR_OK ;
}
/* end subroutine (keyopt_curend) */


/* find a paramter by key/value pair */
int keyopt_findvalue(KEYOPT *php,cchar *key,cchar *value,int vlen,
		KEYOPT_VALUE **rpp)
{
	KEYOPT_NAME	*kp ;
	int		rs ;
	int		klen ;
	const char	*tp ;


#if	CF_DEBUGS
	debugprintf("keyopt_findvalue: ent, n=%s v=%t\n",
	    key,value,strnlen(value,vlen)) ;
#endif

	if (php == NULL) return SR_FAULT ;
	if (value == NULL) return SR_FAULT ;

	if (php->magic != KEYOPT_MAGIC) return SR_NOTOPEN ;

	if (vlen < 0) vlen = strlen(value) ;

/* do we have this key? */

	klen = -1 ;
	if ((tp = strchr(key,'=')) != NULL)
	    klen = (tp - key) ;

	if ((rs = keyopt_findkey(php,key,klen,&kp)) >= 0) {
	    if (vlen < 0) vlen = strlen(value) ;
	    rs = keyname_findv(kp,value,vlen,rpp) ;
	}

	return rs ;
}
/* end subroutine (keyopt_findvalue) */


/* private subroutines */


/* find a parameter by key */
static int keyopt_findkey(php,key,klen,rpp)
KEYOPT		*php ;
const char	key[] ;
int		klen ;
KEYOPT_NAME	**rpp ;
{
	KEYOPT_NAME	*pp ;
	int		f ;

#if	CF_DEBUGS
	debugprintf("keyopt_findkey: ent len=%d n=%s\n",
	    strnlen(key,klen),key) ;
#endif

	for (pp = php->head ; pp != NULL ; pp = pp->next) {

#if	CF_DEBUGS
	    debugprintf("keyopt_findkey: top loop len=%d key=%s\n",
	        strlen(pp->name),pp->name) ;
#endif

	    if (klen < 0) {
	        f = (strkeycmp(pp->name,key) == 0) ;
	    } else
	        f = (strncmp(pp->name,key,klen) == 0) &&
	            ((pp->name[klen] == '=') || (pp->name[klen] == '\0')) ;

	    if (f)
	        break ;

#if	CF_DEBUGS
	    debugprintf("keyopt_findkey: bottom of loop\n") ;
#endif

	} /* end for */

#if	CF_DEBUGS
	debugprintf("keyopt_findkey: returning >%s< n=%s\n",
	    (pp == NULL) ? "NOT FOUND" : "FOUND", key) ;
#endif

	if (rpp != NULL)
	    *rpp = pp ;

	return (pp != NULL) ? SR_OK : SR_NOTFOUND ;
}
/* end subroutine (keyopt_findkey) */


static int keyopt_loadpair(KEYOPT *php,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		klen ;
	const char	*keyp ;

	if (sp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("keyopt_loadpair: ent >%t<\n",sp,strnlen(sp,sl)) ;
#endif

	if ((klen = sfshrink(sp,sl,&keyp)) > 0) {
	    int		vlen = 0 ;
	    cchar	*valuep = NULL ;
	    cchar	*tp ;
	    char	keybuf[KEYBUFLEN + 1] ;

	    if ((tp = strnchr(keyp,klen,'=')) != NULL) {

	        valuep = (tp + 1) ;
	        vlen = (keyp + klen) - valuep ;
	        klen = (tp - keyp) ;

#if	CF_DEBUGS
	        debugprintf("keyopt_loadpair: K=V vlen=%d\n",vlen) ;
#endif

	        while ((klen > 0) && CHAR_ISWHITE(keyp[klen - 1])) {
	            klen -= 1 ;
		}

	        while ((vlen > 0) && CHAR_ISWHITE(*valuep)) {
	            valuep += 1 ;
	            vlen -= 1 ;
	        }

	    } /* end if */

#if	CF_DEBUGS
	    debugprintf("keyopt_loadpair: copying key klen=%d\n",klen) ;
#endif

	    strwcpy(keybuf,keyp,MIN(klen,KEYBUFLEN)) ; /* cannot fail */
	    rs = keyopt_loadvalue(php,keybuf,valuep,vlen) ;

	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("keyopt_loadpair: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (keyopt_loadpair) */


/* KEYNAME begin */

static int keyname_incri(KEYOPT_NAME *pp)
{
	KEYOPT_VALUE	*vp ;
	int		rs = SR_NOTFOUND ;

	if (pp->next != NULL) {
	    rs = keyname_incri(pp->next) ;
	}

/* increment ourselves if we are at bottom or if previous guy carried */

	if (rs == SR_NOTFOUND) {
	    vp = pp->current ;
	    if (vp->next == NULL) {
	        pp->current = pp->head ;
	        rs = -1 ;
	    } else {
	        pp->current = vp->next ;
	        rs = 0 ;
	    } /* end if */
	} /* end if (not found) */

	return rs ;
}
/* end subroutine (keyname_incri) */


/* find a paramter by value? */
static int keyname_findv(KEYOPT_NAME *pp,cchar *vbuf,int vlen,KEYOPT_VALUE **rp)
{
	KEYOPT_VALUE	*vp ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("keyname_findv: ent vlen=%d v=%t\n",
	    vlen,
	    vbuf,
	    strnlen(vbuf,vlen)) ;
#endif /* CF_DEBUGS */

	if (vlen < 0)
	    vlen = strlen(vbuf) ;

	for (vp = pp->head ; vp != NULL ; vp = vp->next) {
	    f = (strncmp(vp->value,vbuf,vlen) == 0) ;
	    f = f && (vp->value[vlen] == '\0') ;
	    if (f) break ;
	} /* end for */

	if (rp != NULL)
	    *rp = vp ;

	return (f) ? SR_OK : SR_NOTFOUND ;
}
/* end subroutine (keyname_findv) */

/* KEYNAME end */


