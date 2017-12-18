/* passwdent */

/* PASSWD structure management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_EXTRAS	0		/* extra PASSWD entries */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines perform some PASSWD-structure management functions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"passwdent.h"


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sichr(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	passwdent_parseone(PASSWDENT *,STOREITEM *,int,
			cchar *,int) ;
static int	passwdent_parsedefs(PASSWDENT *,STOREITEM *,int) ;

static int	si_copystr(STOREITEM *,char **,const char *) ;


/* local variables */


/* exported subroutines */


int passwdent_parse(PASSWDENT *pwp,char *pwbuf,int pwlen,cchar *sp,int sl)
{
	STOREITEM	ib, *ibp = &ib ;
	int		rs ;
	int		rs1 ;

	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	memset(pwp,0,sizeof(PASSWDENT)) ;
	if ((rs = storeitem_start(ibp,pwbuf,pwlen)) >= 0) {
	    int		fi = 0 ;
	    int		si ;
	    while ((si = sichr(sp,sl,':')) >= 0) {
	        rs = passwdent_parseone(pwp,ibp,fi++,sp,si) ;
	        sl -= (si+1) ;
	        sp += (si+1) ;
	        if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && sl && sp[0]) {
	        rs = passwdent_parseone(pwp,ibp,fi++,sp,sl) ;
	    }
	    if (rs >= 0) {
	        rs = passwdent_parsedefs(pwp,ibp,fi) ;
		fi = rs ;
	    }
	    if ((rs >= 0) && (fi < 6)) rs = SR_BADFMT ;
	    rs1 = storeitem_finish(ibp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (passwdent_parse) */


int passwdent_load(PASSWDENT *pwp,char *pwbuf,int pwlen,const PASSWDENT *spwp)
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	if (pwp == NULL) return SR_FAULT ;
	if (pwbuf == NULL) return SR_FAULT ;
	if (spwp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("passwdent_load: ent\n") ;
#endif

	memcpy(pwp,spwp,sizeof(PASSWDENT)) ;

	if ((rs = storeitem_start(&ib,pwbuf,pwlen)) >= 0) {

#if	CF_DEBUGS
	debugprintf("passwdent_load: name=%s\n",spwp->pw_name) ;
#endif
	    si_copystr(&ib,&pwp->pw_name,spwp->pw_name) ;

#if	CF_DEBUGS
	debugprintf("passwdent_load: passwd\n") ;
#endif
	    si_copystr(&ib,&pwp->pw_passwd,spwp->pw_passwd) ;

#if	CF_DEBUGS
	debugprintf("passwdent_load: gecos\n") ;
#endif
	    si_copystr(&ib,&pwp->pw_gecos,spwp->pw_gecos) ;

#if	CF_DEBUGS
	debugprintf("passwdent_load: dir\n") ;
#endif
	    si_copystr(&ib,&pwp->pw_dir,spwp->pw_dir) ;

#if	CF_DEBUGS
	debugprintf("passwdent_load: shell\n") ;
#endif
	    si_copystr(&ib,&pwp->pw_shell,spwp->pw_shell) ;

#if	CF_EXTRAS

	    si_copystr(&ib,&pwp->pw_age,spwp->pw_age) ;

	    si_copystr(&ib,&pwp->pw_comment,spwp->pw_comment) ;

#endif /* CF_EXTRAS */

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

#if	CF_DEBUGS
	debugprintf("passwdent_load: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (passwdent_load) */


int passwdent_format(const PASSWDENT *pwp,char *rbuf,int rlen)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if (pwp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int	i ;
	    int	v ;
	    for (i = 0 ; i < 7 ; i += 1) {
	        if (i > 0) rs = sbuf_char(&b,':') ;
	        if (rs >= 0) {
	            switch (i) {
	            case 0:
	                rs = sbuf_strw(&b,pwp->pw_name,-1) ;
	                break ;
	            case 1:
	                rs = sbuf_strw(&b,pwp->pw_passwd,-1) ;
	                break ;
	            case 2:
	                v = pwp->pw_uid ;
	                rs = sbuf_deci(&b,v) ;
	                break ;
	            case 3:
	                v = pwp->pw_gid ;
	                rs = sbuf_deci(&b,v) ;
	                break ;
	            case 4:
	                rs = sbuf_strw(&b,pwp->pw_gecos,-1) ;
	                break ;
	            case 5:
	                rs = sbuf_strw(&b,pwp->pw_dir,-1) ;
	                break ;
	            case 6:
	                rs = sbuf_strw(&b,pwp->pw_shell,-1) ;
	                break ;
	            } /* end switch */
	        } /* end if */
	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (passwdent_format) */


int passwdent_size(const PASSWDENT *pp)
{
	int		size = 1 ;
	if (pp->pw_name != NULL) {
	    size += (strlen(pp->pw_name)+1) ;
	}
	if (pp->pw_passwd != NULL) {
	    size += (strlen(pp->pw_passwd)+1) ;
	}
	if (pp->pw_gecos != NULL) {
	    size += (strlen(pp->pw_gecos)+1) ;
	}
	if (pp->pw_dir != NULL) {
	    size += (strlen(pp->pw_dir)+1) ;
	}
	if (pp->pw_shell != NULL) {
	    size += (strlen(pp->pw_shell)+1) ;
	}
#if	CF_EXTRAS
	if (pp->pw_age != NULL) {
	    size += (strlen(pp->pw_age)+1) ;
	}
	if (pp->pw_comment != NULL) {
	    size += (strlen(pp->pw_comment)+1) ;
	}
#endif /* CF_EXTRAS */
	return size ;
}
/* end subroutine (passwdent_size) */


/* local subroutines */


static int passwdent_parseone(PASSWDENT	*pwp,STOREITEM *ibp,int fi,
		cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		v ;
	cchar		**vpp = NULL ;

	switch (fi) {
	case 0:
	    vpp = (const char **) &pwp->pw_name ;
	    break ;
	case 1:
	    vpp = (const char **) &pwp->pw_passwd ;
	    break ;
	case 2:
	    rs = cfdeci(vp,vl,&v) ;
	    pwp->pw_uid = v ;
	    break ;
	case 3:
	    rs = cfdeci(vp,vl,&v) ;
	    pwp->pw_gid = v ;
	    break ;
	case 4:
	    vpp = (const char **) &pwp->pw_gecos ;
	    break ;
	case 5:
	    vpp = (const char **) &pwp->pw_dir ;
	    break ;
	case 6:
	    vpp = (const char **) &pwp->pw_shell ;
	    break ;
	} /* end switch */
	if ((rs >= 0) && (vpp != NULL)) {
	    int		cl ;
	    const char	*cp ;
	    if ((cl = sfshrink(vp,vl,&cp)) >= 0) {
	        rs = storeitem_strw(ibp,cp,cl,vpp) ;
	    }
	}

	return rs ;
}
/* end subroutine (passwdent_parseone) */


/* ARGSUSED */
static int passwdent_parsedefs(PASSWDENT *pwp,STOREITEM *ibp,int sfi)
{
	int		rs = SR_OK ;
	if (sfi == 6) {
	    const char	**vpp = (const char **) &pwp->pw_shell ;
	    const char	*np = pwp->pw_name ;
	    const char	*vp ;
	    vp = (np + strlen(np)) ;
	    sfi += 1 ;
	    rs = storeitem_strw(ibp,vp,0,vpp) ;
	}
	return (rs >= 0) ? sfi : rs ;
}
/* end subroutine (passwdent_parsedefs) */


static int si_copystr(STOREITEM *ibp,char **pp,cchar *p1)
{
	int		rs = SR_OK ;
	cchar		**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (p1 != NULL) {
	    rs = storeitem_strw(ibp,p1,-1,cpp) ;
	}

	return rs ;
}
/* end subroutine (si_copystr) */


