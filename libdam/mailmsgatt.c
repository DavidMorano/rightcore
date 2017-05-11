/* mailmsgatt */

/* mail-message attachment processing */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This module was originally written.

	= 2001-01-03, David A­D­ Morano
        I changed the 'mailmsgattent_type()' subroutine slightly. I changed it
        so that when no content type if found, it will assume a binary content
        type rather than returning a SR_NOTFOUND error.

*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object processes and manipulates email mailmsgattment and address.


*******************************************************************************/


#define	MAILMSGATT_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecitem.h>
#include	<mimetypes.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"mailmsgatt.h"


/* local defines */

#define	MAILMSGATT_DEFENTS	10

#ifndef	MAILMSGATTENT
#define	MAILMSGATTENT		MAILMSGATT_ENT
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sisub(const char *,int,const char *) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int	mailmsgattent_start(MAILMSGATTENT *,cchar *,cchar *,cchar *,int) ;
int	mailmsgattent_type(MAILMSGATTENT *,MIMETYPES *) ;
int	mailmsgattent_finish(MAILMSGATTENT *) ;
int	mailmsgattent_isplaintext(MAILMSGATTENT *) ;

#ifdef	COMMENT
static int mailmsgatt_cmp() ;
#endif /* COMMENT */

static int	utypes_init(cchar **) ;

static int	freeit(void *) ;


/* local variables */

static cchar	*str_text = "text" ;
static cchar	*str_plain = "plain" ;
static cchar	*str_binary = "binary" ;


/* exported subroutines */


int mailmsgatt_start(MAILMSGATT *rhp)
{
	const int	n = MAILMSGATT_DEFENTS ;
	const int	opts = VECITEM_PNOHOLES ;
	int		rs ;

	rs = vecitem_start(rhp,n,opts) ;

	return rs ;
}
/* end subroutine (mailmsgatt_start) */


/* free up the mailmsgattments list object */
int mailmsgatt_finish(MAILMSGATT *rhp)
{
	MAILMSGATTENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecitem_get(rhp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = mailmsgattent_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecitem_finish(rhp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailmsgatt_finish) */


/* add an attachment (w/ default content-type and content-encoding) */
int mailmsgatt_add(MAILMSGATT *rhp,cchar *ct,cchar *ce,cchar *nbuf,int nlen)
{
	MAILMSGATTENT	ve ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: ent\n") ;
#endif

	if (rhp == NULL) return SR_FAULT ;
	if (rhp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: continuing\n") ;
#endif

	if (nlen < 0) nlen = strlen(nbuf) ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: nlen=%d\n",nlen) ;
#endif

	if ((rs = mailmsgattent_start(&ve,ct,ce,nbuf,nlen)) >= 0) {
	    rs = vecitem_add(rhp,&ve,sizeof(MAILMSGATTENT)) ;
	    if (rs < 0)
	        mailmsgattent_finish(&ve) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsgatt_add) */


/* delete an mailmsgattment */
int mailmsgatt_del(MAILMSGATT *alp,int i)
{
	MAILMSGATTENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (alp == NULL) return SR_FAULT ;

	if ((rs = vecitem_get(alp,i,&ep)) >= 0) {
	    if (ep != NULL) {
	        rs1 = mailmsgattent_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = vecitem_del(alp,i) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (mailmsgatt_del) */


/* return the number of hosts seen so far */
int mailmsgatt_count(MAILMSGATT *rhp)
{
	int		rs ;

	if (rhp == NULL) return SR_FAULT ;

	rs = vecitem_count(rhp) ;

	return rs ;
}
/* end subroutine (mailmsgatt_count) */


/* enumerate */
int mailmsgatt_enum(MAILMSGATT *rhp,int i,MAILMSGATTENT **epp)
{
	int		rs ;

	rs = vecitem_get(rhp,i,epp) ;

	return rs ;
}
/* end subroutine (mailmsgatt_enum) */


/* find content types for all of the mailmsgattments using a MIME-types DB */
int mailmsgatt_typeatts(MAILMSGATT *rhp,MIMETYPES *mtp)
{
	MAILMSGATTENT	*ep ;
	int		rs = SR_OK ;
	int		i ;

	if (rhp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_typeatts: ent\n") ;
#endif

	for (i = 0 ; mailmsgatt_enum(rhp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs = mailmsgattent_type(ep,mtp) ;
	        if (rs < 0) break ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("mailmsgatt_typeatts: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsgatt_typeatts) */


/* the managed (enclosed) object */


/* start a new attachment (w/ default content-type and content-encoding) */
int mailmsgattent_start(MAILMSGATTENT *ep,cchar *ct,cchar *ce,
		cchar *nbuf,int nlen)
{
	int		rs = SR_OK ;
	int		i, i2, j ;
	int		fnlen ;
	int		sl, cl ;
	const char	*utypes[3] ;
	const char	*fn ;
	const char	*sp, *cp ;
	char		fname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_start: ent\n") ;
#endif

	if (ep == NULL) return SR_FAULT ;
	if (nbuf == NULL) return SR_FAULT ;

	if (nlen < 0) nlen = strlen(nbuf) ;

/* yes, amazingly, the C language disallows this to be defined statically */

	utypes_init(utypes) ;

/* continue */

	memset(ep,0,sizeof(MAILMSGATTENT)) ;
	ep->cte = -1 ;
	ep->clen = -1 ;
	ep->clines = -1 ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_start: continuing\n") ;
#endif

	if ((i = sisub(nbuf,nlen,"=")) >= 0) {
	    sp = (nbuf+i) ;

#if	CF_DEBUGS
	    debugprintf("mailmsgattent_start: split type=filename\n") ;
#endif

	    sp += 1 ;
	    if (nlen < 0) {
	        sl = strlen(sp) ;
	    } else {
	        sl = (nlen - i - 1) ;
	    }

	    fnlen = sfshrink(sp,sl,&fn) ;

	    if (fn[fnlen] != '\0') {
	        if (fnlen > (MAXPATHLEN + 1)) fnlen = (MAXPATHLEN - 1) ;
	        strwcpy(fname,fn,fnlen) ;
	        fn = fname ;
	    }

	    if (strcmp(fn,"-") != 0) {

#if	CF_DEBUGS
	        debugprintf("mailmsgattent_start: not STDIN fn=%s\n",fn) ;
#endif

	        rs = perm(fn,-1,-1,NULL,R_OK) ;

#if	CF_DEBUGS
	        debugprintf("mailmsgattent_start: perm() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            goto ret0 ;

	    }

	    if ((ep->attfname = mallocstrw(fn,fnlen)) == NULL)
	        goto badmem ;

	    if ((i2 = sisub(nbuf,i,"/")) >= 0) {
	        sp = (nbuf+i) ;

/* type */

	        cl = sfshrink(nbuf,i2,&cp) ;

	        ep->type = mallocstrw(cp,cl) ;

/* subtype */

	        cl = sfshrink((nbuf + i2 + 1),(i - i2 - 1),&cp) ;

	        ep->subtype = mallocstrw(cp,cl) ;
	        if ((ep->type == NULL) || (ep->subtype == NULL))
	            goto badmem ;

	    } else if (i > 0) {

	        cl = sfshrink(nbuf,i,&cp) ;

	        if ((j = matcasestr(utypes,cp,cl)) >= 0) {

	            ep->type = mallocstr(utypes[j]) ;
	            if (ep->type == NULL)
	                goto badmem ;

	        } else {

	            ep->ext = mallocstrw(cp,cl) ;
	            if (ep->ext == NULL)
	                goto badmem ;

	        } /* end if */

	    } /* end if */

	} else {

#if	CF_DEBUGS
	    debugprintf("mailmsgattent_start: filename only\n") ;
#endif

	    fnlen = sfshrink(nbuf,nlen,&fn) ;

	    if (fn[fnlen] != '\0') {
	        if (fnlen > (MAXPATHLEN + 1)) fnlen = (MAXPATHLEN - 1) ;
	        strwcpy(fname,fn,fnlen) ;
	        fn = fname ;
	    }

	    if (strcmp(fn,"-") != 0) {

#if	CF_DEBUGS
	        debugprintf("mailmsgattent_start: not STDIN fn=%s\n",fn) ;
#endif

	        rs = perm(fn,-1,-1,NULL,R_OK) ;

#if	CF_DEBUGS
	        debugprintf("mailmsgattent_start: perm() rs=%d\n",rs) ;
#endif

	        if (rs < 0) goto ret0 ;

	    }

	    if ((ep->attfname = mallocstrw(fn,fnlen)) == NULL)
	        goto badmem ;

	} /* end if */

/* content_type */

	if (rs >= 0) {
	if ((ct != NULL) && (ep->ext == NULL) && (ep->type == NULL)) {

	    cl = strlen(ct) ;

	    if ((i2 = sisub(ct,cl,"/")) >= 0) {
	        cp = (ct+i2) ;

	        ep->type = mallocstrw(ct,i2) ;

	        ep->subtype = mallocstrw(ct + i2 + 1,cl - i2 - 1) ;

	        if ((ep->type == NULL) || (ep->subtype == NULL))
	            goto badmem ;

	    } else {

	        if ((j = matcasestr(utypes,ct,cl)) >= 0) {

	            ep->type = mallocstr(utypes[j]) ;
	            if (ep->type == NULL)
	                goto badmem ;

	        } else {

	            ep->ext = mallocstrw(ct,cl) ;
	            if (ep->ext == NULL)
	                goto badmem ;

	        } /* end if */

	    } /* end if */

	} /* end if (tried to use running default content-type) */
	} /* end if (ok) */

/* content_encoding */

	if ((rs >= 0) && (ce != NULL)) {
	    ep->encoding = mallocstr(ce) ;
	    if (ep->encoding == NULL) rs = SR_NOMEM ;
	}

	if (rs >= 0) {
	    ep->magic = MAILMSGATTENT_MAGIC ;
	} else {
	    mailmsgattent_finish(ep) ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("mailmsgattent_start: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
badmem:
	rs = SR_NOMEM ;
	mailmsgattent_finish(ep) ;
	goto ret0 ;
}
/* end subroutine (mailmsgattent_start) */


int mailmsgattent_finish(MAILMSGATTENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	ep->clen = -1 ;
	ep->clines = -1 ;
	ep->cte = -1 ;

	rs1 = freeit(&ep->type) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->subtype) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->encoding) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->attfname) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->ext) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = freeit(&ep->description) ;
	if (rs >= 0) rs = rs1 ;

	if (ep->auxfname != NULL) {
	    if (ep->auxfname[0] != '\0') {
	        u_unlink(ep->auxfname) ;
	    }
	    rs1 = freeit(&ep->auxfname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (mailmsgattent_finish) */


/* find the content-type for this particular attachment */
int mailmsgattent_type(MAILMSGATTENT *ep,MIMETYPES *mtp)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_type: filename=%s\n", ep->attfname) ;
	debugprintf("mailmsgattent_type: type=%s\n",ep->type) ;
	debugprintf("mailmsgattent_type: ext=%s\n",ep->ext) ;
#endif

	if ((ep->type != NULL) && (ep->type[0] == '\0')) {
	    uc_free(ep->type) ;
	    ep->type = NULL ;
	}

	if (ep->type == NULL) {
	    cchar	*tp ;
	    char	typespec[MIMETYPES_TYPELEN + 1] ;

/* we didn't have a type so we look the hard way */

	    if (ep->subtype != NULL) {
	        uc_free(ep->subtype) ;
	        ep->subtype = NULL ;
	    }

	    if ((ep->ext != NULL) && (ep->ext[0] == '\0')) {
	        uc_free(ep->ext) ;
	        ep->ext = NULL ;
	    }

/* if there is no explicit extension, then we get it from the filename */

	    if ((ep->ext == NULL) && (ep->attfname != NULL)) {
	        if ((tp = strrchr(ep->attfname,'.')) != NULL) {
		    cchar	*cp ;
		    if ((rs = uc_mallocstrw((tp+1),-1,&cp)) >= 0) {
	                ep->ext = cp ;
		    }
	        } /* end if */
	    } /* end if (extracting filename extension) */

/* continue */

	    if (rs >= 0) {
		cchar	*vp = NULL ;
		int	vl = -1 ;
	        if (ep->ext != NULL) {
	            if ((rs = mimetypes_find(mtp,typespec,ep->ext)) >= 0) {
	                if ((tp = strchr(typespec,'/')) != NULL) {
			    cchar	*cp ;
			    vp = typespec ;
			    vl = (tp-typespec) ;
			    if ((rs = uc_mallocstrw((tp+1),-1,&cp)) >= 0) {
	                        ep->subtype = cp ;
			    }
	                } else {
	                    vp = typespec ;
		        }
	            } else if (rs == SR_NOENT) {
			rs = SR_OK ;
	                vp = "binary" ;
	            }
	        } else {
		    vp = "binary" ;
	        }
		if ((rs >= 0) && (vp != NULL)) {
		    cchar	*cp ;
		    if ((rs = uc_mallocstrw(vp,vl,&cp)) >= 0) {
			f = TRUE ;
			ep->type = cp ;
		    }
		}
	    } /* end if (ok) */

	} /* end if (needed a type) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailmsgattent_type) */


int mailmsgattent_typeset(MAILMSGATTENT *ep,cchar *tstr,cchar *ststr)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->type != NULL) {
	    rs1 = uc_free(ep->type) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->type = NULL ;
	}

	if (ep->subtype != NULL) {
	    rs1 = uc_free(ep->subtype) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->subtype = NULL ;
	}

	if (rs >= 0) {

	    if ((tstr != NULL) && (tstr[0] != '\0')) {
	        const char	*cp ;
	        if ((rs = uc_mallocstrw(tstr,-1,&cp)) >= 0) {
	            ep->type = cp ;
	            if ((ststr != NULL) && (ststr[0] != '\0')) {
	                if ((rs = uc_mallocstrw(ststr,-1,&cp)) >= 0) {
	                    ep->subtype = cp ;
	                } /* end if (memory-allocation) */
	            }
	            if (rs < 0) {
	                uc_free(ep->type) ;
	                ep->type = NULL ;
	            }
	        } /* end if (memory-allocation) */
	    } /* end if (tstr) */

	    if (rs >= 0) {
	        rs = mailmsgattent_isplaintext(ep) ;
	    }

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (mailmsgattent_typeset) */


int mailmsgattent_isplaintext(MAILMSGATTENT *ep)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if ((ep->type != NULL) && (ep->subtype != NULL)) {
	    f = (strcmp(ep->type,str_text) == 0) ;
	    f = f && (strcmp(ep->subtype,str_plain) == 0) ;
	}
	ep->f_plaintext = f ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailmsgattent_isplaintext) */


/* private subroutines */


static int utypes_init(cchar **utypes)
{
	int		i ;
	for (i = 0 ; i < 3 ; i += 1) {
	    switch (i) {
	    case 0:
	        utypes[i] = str_binary ;
	        break ;
	    case 1:
	        utypes[i] = str_text ;
	        break ;
	    case 2:
	        utypes[i] = NULL ;
	        break ;
	    } /* end switch */
	} /* end for */
	return 0 ;
}
/* end subroutine (utypes_init) */


static int freeit(void *p)
{
	int		rs = SR_OK ;
	void		**pp = (void **) p ;
	if (*pp != NULL) {
	    rs = uc_free(*pp) ;
	    *pp = NULL ;
	}
	return rs ;
}
/* end subroutine (freeit) */


