/* paramopt */

/* paramater option manipulations */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This code module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code module contains subroutines used to add paramters to parameter
        lists and such for later access. Although meant to be relatively
        straight forward, there are several aspects of this object that are
        tailored towards special uses that are not at all obvious.

	All parameter names and values are stored in freshly allocated memory.
	The original storage for parameter names and values can be freed after
	they are stored using these routines.

	One of the more strange uses of this object (and actually the reason
	that it was written) is to gather parameters with values but to also
	organize them in a way that all combinations of parameter values can be
	exponentially enumerated.  This is useful for programs that want to
	enumerate all possible combinations of user supplied values of some
	sort.

	This object stores a funny sort of key-value(s) pairs.  For each key,
	multiple values can be stored along with the same key.  Multiple
	apparent entries with the same key are stored in the SAME record.  This
	is unlike regular database objects that will store multiple entries
	with the same key in different records.


*******************************************************************************/


#define	PARAMOPT_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"paramopt.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	strwcmp(const char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* forward references */

int		paramopt_loadu(PARAMOPT *,const char *,int) ;
int		paramopt_loads(PARAMOPT *,const char *,const char *,int) ;
int		paramopt_load(PARAMOPT *,const char *,const char *,int) ;
int		paramopt_curbegin(PARAMOPT *,PARAMOPT_CUR *) ;
int		paramopt_curend(PARAMOPT *,PARAMOPT_CUR *) ;

static int	paramopt_findkey(PARAMOPT *,const char *,PARAMOPT_NAME **) ;
static int	name_incri(PARAMOPT_NAME *) ;
static int	name_vfind(PARAMOPT_NAME *,const char *,int,PARAMOPT_VALUE **) ;


/* local variables */


/* exported subroutines */


int paramopt_start(PARAMOPT *php)
{

	if (php == NULL) return SR_FAULT ;

	memset(php,0,sizeof(PARAMOPT)) ;
	php->head = php->tail = NULL ;
	php->magic = PARAMOPT_MAGIC ;
	php->f_inited = TRUE ;

	return SR_OK ;
}
/* end subroutine (paramopt_start) */


int paramopt_finish(PARAMOPT *php)
{
	PARAMOPT_NAME	*np, *nnp ;
	PARAMOPT_VALUE	*vp, *nvp ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (php == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

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
	php->f_inited = FALSE ;
	php->magic = 0 ;
	return rs ;
}
/* end subroutine (paramopt_finish) */


/* load a parameter with an unknown "name" and values all in string 's' */

/****

This loads someting that looks like:
	key=v1,v2,v3,...
Notice that the keyname is extrcted from the supplied string.

****/

int paramopt_loadu(PARAMOPT *php,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;

	if (php == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("paramopt_loadu: ent\n") ;
#endif

	if ((i = sibreak(sp,sl,"=\t")) >= 0) {
	    int		cl ;
	    const char	*cp ;
	    if ((cl = sfshrink((sp + i),(sl- i),&cp)) > 0) {
		char	*name ;
		if ((rs = uc_malloc((cl+1),&name)) >= 0) {
		    strwcpy(name,cp,cl) ;
	            i += 1 ;
	            rs = paramopt_loads(php,name,(sp + i),(sl- i)) ;
	            c += rs ;
		    uc_free(name) ;
		} /* end if (memory-allocation) */
	    } /* end if */
	} else
	    rs = SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("paramopt_loadu: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramopt_loadu) */


/* load parameter with a known "name" and string of values (in 's') */

/****

This loads someting that looks like:
	v1,v2,v3,...
given that a keyname is specified explicitly.

****/

int paramopt_loads(PARAMOPT *php,cchar *name,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	const char	*cp ;

	if (php == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("paramopt_loads: name=%s\n",name) ;
#endif

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("paramopt_loads: sl=%d\n",sl) ;
#endif

	while ((i = sibreak(sp,sl," ,\t\r\n\v\f")) >= 0) {
	    cp = (sp+i) ;
	    if (i > 0) {
	        rs = paramopt_load(php,name,sp,i) ;
	        c += rs ;
	    }
	    sp = (cp + 1) ;
	    sl -= (i + 1) ;
	    if (rs < 0) break ;
	}  /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    rs = paramopt_load(php,name,sp,sl) ;
	    c += rs ;
	}

#if	CF_DEBUGS
	debugprintf("paramopt_loads: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramopt_loads) */


/* load a single key=value pair */
int paramopt_load(PARAMOPT *php,cchar *name,cchar *vbuf,int vlen)
{
	int		rs = SR_OK ;
	int		vl ;
	int		f = FALSE ;
	const char	*vp ;

	if (php == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (name[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("paramopt_load: name=%s\n",name) ;
#endif

/* clean up the value a little */

	if (vlen < 0)
	    vlen = strlen(vbuf) ;

#if	CF_DEBUGS
	debugprintf("paramopt_load: value=>%t< len=%d\n",value,vlen,vlen) ;
#endif

	if ((vl = sfshrink(vbuf,vlen,&vp)) > 0) {
	    PARAMOPT_VALUE	*ovp, *nvp ;
	    PARAMOPT_NAME	*pp ;
	    const int		rsn = SR_NOTFOUND ;
	    int			size ;
	    const char		*cp ;

	    while ((vl > 0) && 
	        (CHAR_ISWHITE(vp[vl - 1]) || (vp[vl - 1] == ','))) {
	        vl -= 1 ;
	    }

#if	CF_DEBUGS
	    debugprintf("paramopt_load: len=%d v=\"%t\"\n",
	        vl,vp,vl) ;
#endif

/* do we have one of these named keys already? */

	    if ((rs = paramopt_findkey(php,name,&pp)) == rsn) {

#if	CF_DEBUGS
	        debugprintf("paramopt_load: did not find already\n") ;
#endif

/* make a new parameter header block, insert at head */

	        size = sizeof(PARAMOPT_NAME) ;
	        if ((rs = uc_malloc(size,&pp)) >= 0) {
	            pp->c = 0 ;
	            pp->next = php->head ;	/* insert at head */
	            pp->head = NULL ;
	            pp->tail = NULL ;
	            pp->current = NULL ;
	            if ((rs = uc_mallocstrw(name,-1,&cp)) >= 0) {
	                pp->name = cp ;
	                php->head = pp ;	/* insert at head */
	            }
	            if (rs < 0)
	                uc_free(pp) ;
	        } /* end if */

	    } /* end if (adding a new parameter block on the list) */

/* OK, now we have the parameter block that we are looking for in 'pp' */

#if	CF_DEBUGS
	    debugprintf("paramopt_load: about to check if we have value\n") ;
#endif

	    if (rs >= 0) {
	        if ((rs = name_vfind(pp,vp,vl,&ovp)) == rsn) {
	            f = TRUE ;
	            size = sizeof(PARAMOPT_VALUE) ;
	            if ((rs = uc_malloc(size,&nvp)) >= 0) {
	                nvp->next = NULL ;
	                if ((rs = uc_mallocstrw(vp,vl,&cp)) >= 0) {
	                    nvp->value = cp ;
	                    ovp = pp->tail ;
	                    if (pp->head == NULL)
	                        pp->head = nvp ;
	                    pp->c += 1 ;
	                    pp->tail = nvp ;
	                    pp->current = nvp ;
	                    if (ovp != NULL)
	                        ovp->next = nvp ;
	                }
	                if (rs < 0)
	                    uc_free(nvp) ;
	            } /* end if (new value) */
		} /* end if (name_vfind) */
	    } /* end if (ok) */

	} /* end if (non-zero) */

#if	CF_DEBUGS
	debugprintf("paramopt_load: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (paramopt_load) */


int paramopt_havekey(PARAMOPT *php,cchar *name)
{
	PARAMOPT_NAME	*pp ;
	int		rs ;
	int		c = 0 ;

	if (php == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (name[0] == '\0') return SR_INVALID ;

	if ((rs = paramopt_findkey(php,name,&pp)) >= 0) {
	    c = pp->c ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramopt_havekey) */


/* enumerate on all of the keyes */
int paramopt_enumkeys(PARAMOPT *php,PARAMOPT_CUR *curp,cchar **rpp)
{
	PARAMOPT_NAME	*kp ;
	int		rs = SR_OK ;
	int		kl = 0 ;

	if (php == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = NULL ;

	if (curp->keyp == NULL) {
	    kp = php->head ;		/* get the next key-pointer */
	    curp->valuep = NULL ;
	} else {
	    kp = curp->keyp ;		/* get the current key-pointer */
	    kp = kp->next ;		/* get the next key-pointer */
	}

	if (kp != NULL) {
	    curp->keyp = kp ;		/* update to the current key-pointer */
	    if (rpp != NULL) *rpp = kp->name ;
	    kl = strlen(kp->name) ;
	} else
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (paramopt_enumkeys) */


/* enumerate on a key's values */
int paramopt_fetch(PARAMOPT *php,cchar *key,PARAMOPT_CUR *curp,cchar **rpp)
{
	PARAMOPT_NAME	*kp = NULL ;
	PARAMOPT_VALUE	*vp ;
	PARAMOPT_CUR	ncur ;
	int		rs = SR_OK ;
	int		vl = 0 ;

#if	CF_DEBUGS
	debugprintf("paramopt_fetch: ent key=%s\n",key) ;
#endif

	if (php == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (key[0] == '\0') return SR_INVALID ;

	if (curp == NULL) {
	    curp = &ncur ;
	    curp->keyp = NULL ;
	    curp->valuep = NULL ;
	}

	if (rpp != NULL)
	    *rpp = NULL ;

/* do we have this key? */

	if (curp->keyp == NULL) {
	    if ((rs = paramopt_findkey(php,key,&kp)) >= 0) {
	        curp->keyp = kp ;
	        vp = kp->head ;
	    }
	} else {
	    kp = curp->keyp ;
	    vp = (curp->valuep)->next ;
	} /* end if */

	if ((rs >= 0) && ((kp == NULL) || (vp == NULL)))
	    rs = SR_NOTFOUND ;

	if (rs >= 0) {
	    if (rpp != NULL) *rpp = vp->value ;
	    if (vp->value != NULL) vl = strlen(vp->value) ;
	    curp->valuep = vp ;
	} /* end if */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (paramopt_fetch) */


int paramopt_enumvalues(PARAMOPT *php,cchar *key,PARAMOPT_CUR *curp,
		cchar **rpp)
{

	return paramopt_fetch(php,key,curp,rpp) ;
}
/* end subroutine (paramopt_enumvalues) */


int paramopt_haveval(PARAMOPT *php,cchar *key,cchar *vp,int vl)
{
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		c = 0 ;

	if (php == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (key[0] == '\0') return SR_INVALID ;

	if ((rs = paramopt_curbegin(php,&cur)) >= 0) {
	    int		f ;
	    const char	*ccp ;
	    while (paramopt_enumvalues(php,key,&cur,&ccp) >= 0) {
	        if (ccp == NULL) continue ;
	        f = (strwcmp(ccp,vp,vl) == 0) ;
	        c += f ;
	    } /* end while */
	    paramopt_curend(php,&cur) ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramopt_haveval) */


/* increment the parameters */
int paramopt_incr(PARAMOPT *php)
{
	PARAMOPT_NAME	*pp ;
	PARAMOPT_VALUE	*vp ;
	int		rs ;

	if (php == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	pp = php->head ;
	rs = -1 ;
	if (pp->next != NULL) {
	    rs = name_incri(pp->next) ;
	}

/* increment ourselves if we are at bottom or if previous guy carried */

	if (rs < 0) {
	    vp = pp->current ;
	    if (vp->next == NULL) {
	        pp->current = pp->head ;
	        rs = -1 ;
	    } else {
	        pp->current = vp->next ;
	        rs = 0 ;
	    }
	} /* end if (error) */

	return rs ;
}
/* end subroutine (paramopt_incr) */


/* initialize a cursor */
int paramopt_curbegin(PARAMOPT *php,PARAMOPT_CUR *curp)
{

	if (php == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	curp->keyp = NULL ;
	curp->valuep = NULL ;
	return SR_OK ;
}
/* end subroutine (paramopt_curbegin) */


/* free up a cursor */
int paramopt_curend(PARAMOPT *php,PARAMOPT_CUR *curp)
{

	if (php == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	curp->keyp = NULL ;
	curp->valuep = NULL ;
	return SR_OK ;
}
/* end subroutine (paramopt_curend) */


/* find the number of values for a given key */
int paramopt_countvals(PARAMOPT *php,cchar *key)
{
	PARAMOPT_NAME	*kp ;
	int		rs ;
	int		c = 0 ;

	if (php == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (key[0] == '\0') return SR_INVALID ;

/* do we have this key? */

	if ((rs = paramopt_findkey(php,key,&kp)) >= 0) {
	    c = kp->c ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramopt_countvals) */


/* private subroutines */


/* find a parameter by key-name */
static int paramopt_findkey(PARAMOPT *php,cchar *name,PARAMOPT_NAME **rpp)
{
	PARAMOPT_NAME	*pp ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("paramopt_findkey: ent len=%d n=%s\n",
	    strlen(name),name) ;
#endif

	for (pp = php->head ; pp != NULL ; pp = pp->next) {

#if	CF_DEBUGS
	    debugprintf("paramopt_findkey: top loop len=%d n=%s\n",
	        strlen(pp->name),pp->name) ;
#endif

	    if (strcmp(pp->name,name) == 0)
	        break ;

#if	CF_DEBUGS
	    debugprintf("paramopt_findkey: bottom of loop\n") ;
#endif

	} /* end for */

#if	CF_DEBUGS
	debugprintf("paramopt_findkey: ret %s n=%s\n",
	    (pp == NULL) ? "NOT FOUND" : "FOUND", name) ;
#endif

	if (pp != NULL) c = pp->c ;

	if (rpp != NULL)
	    *rpp = pp ;

	return (pp != NULL) ? c : SR_NOTFOUND ;
}
/* end subroutine (paramopt_findkey) */


#ifdef	COMMENT

/* find a paramter by key/value pair */
static int paramopt_findvalue(php,key,value,vlen,rpp)
PARAMOPT	*php ;
PARAMOPT_VALUE	**rpp ;
const char	key[] ;
const char	value[] ;
int		vlen ;
{
	PARAMOPT_NAME	*kp ;
	int		rs ;

	if (php == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (vlen < 0)
	    vlen = strlen(value) ;

/* do we have this key? */

	if ((rs = paramopt_findkey(php,key,&kp)) >= 0)
	    rs = name_vfind(kp,value,vlen,rpp) ;

	return rs ;
}
/* end subroutine (paramopt_findvalue) */

#endif /* COMMENT */


static int name_incri(PARAMOPT_NAME *pp)
{
	PARAMOPT_VALUE	*vp ;
	int		rs = SR_NOSYS ;

	if (pp->next != NULL) {
	    rs = name_incri(pp->next) ;
	}

/* increment ourselves if we are at bottom or if previous guy carried */

	if (rs < 0) {
	    vp = pp->current ;
	    if (vp->next == NULL) {
	        pp->current = pp->head ;
	        rs = -1 ;
	    } else {
	        pp->current = vp->next ;
	        rs = 0 ;
	    }
	}

	return rs ;
}
/* end subroutine (name_incri) */


/* find a paramter by value? */
static int name_vfind(PARAMOPT_NAME *pp,cchar *vp,int vl,PARAMOPT_VALUE **rpp)
{
	PARAMOPT_VALUE	*vep ;
	int		c = 0 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("name_vfind: ent v=%t\n",
	    vp,strnlen(vp,vl)) ;
#endif /* CF_DEBUGS */

	if (vl < 0) vl = strlen(vp) ;

	for (vep = pp->head ; vep != NULL ; vep = vep->next) {
	    f = (strncmp(vep->value,vp,vl) == 0) ;
	    f = f && (vep->value[vl] == '\0') ;
	    if (f) break ;
	    c += 1 ;
	} /* end for */

	if (rpp != NULL)
	    *rpp = vep ;

	return (f) ? c : SR_NOTFOUND ;
}
/* end subroutine (name_vfind) */


