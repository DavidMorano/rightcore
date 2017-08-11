/* progfile */

/* process the input files */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* run-time switchable */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we process a message for its addresses.

	Synopsis:

	int progfile(pip,pp,ofp,fname)
	PROGINFO	*pip ;
	PARAMOPT	*pp ;
	bfile		*ofp ;
	const char	fname[] ;

	Arguments:

	- pip		program information pointer
	- pp		paramter option pointer
	- ofp		output (BIO) file pointer
	- fname		file to process

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<nulstr.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<mailmsg.h>
#include	<ema.h>
#include	<localmisc.h>

#include	"config.h"
#include	"ema_local.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	matcasestr(const char **,const char *,int) ;
extern int	mailmsg_loadfile(MAILMSG *,bfile *) ;
extern int	bprintlines(bfile *,int,const char *,int) ;

extern int	progentryinfo(PROGINFO *,bfile *,EMA_ENT *,int) ;
extern int	progentryaddr(PROGINFO *,bfile *,EMA_ENT *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procinfile(PROGINFO *,PARAMOPT *,bfile *,EMA *,cchar *) ;
static int	procinfiler(PROGINFO *,bfile *,EMA *,MAILMSG *,cchar *,int) ;

static int	progfile_info(PROGINFO *,bfile *,EMA_ENT *,int) ;
static int	progfile_addr(PROGINFO *,bfile *,EMA_ENT *,int) ;


/* local variables */

static const char	*emas[] = {
	"message-id",
	"errors-to",
	"return-path",
	"sender",
	"from",
	"forwarded-to",
	"redirected-to",
	"delivered-to",
	"in-reply-to",
	"to",
	"cc",
	"bcc",
	NULL
} ;


/* exported subroutines */


int progfile(PROGINFO *pip,PARAMOPT *pp,bfile *ofp,cchar *fname)
{
	CMD_LOCAL	*lsp = pip->lsp ;
	EMA		adds ;
	EMA_ENT		*ep ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfile: ent file=%s\n",fname) ;
#endif

	if ((rs = ema_start(&adds)) >= 0) {

	    if ((rs = procinfile(pip,pp,ofp,&adds,fname)) >= 0) {
	        int	i ;

	        for (i = 0 ; ema_get(&adds,i,&ep) >= 0 ; i += 1) {
	            if (ep != NULL) {
	                if (! lsp->f.count) {
	                    if (lsp->f.info) {
	                        rs = progfile_info(pip,ofp,ep,0) ;
	                    } else {
	                        rs = progfile_addr(pip,ofp,ep,0) ;
			    }
	                } /* end if (counting only) */
	                n += 1 ;
		    }
	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (procinfile) */

	    rs1 = ema_finish(&adds) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ema) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ret rs=%d entries=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


static int procinfile(pip,pp,ofp,emap,fname)
PROGINFO	*pip ;
PARAMOPT	*pp ;
bfile		*ofp ;
EMA		*emap ;
const char	fname[] ;
{
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progfile/procinfile: ent fname=%s\n",fname) ;
#endif

	if ((fname[0] == '\0') || (fname[0] == '-')) fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    MAILMSG	m ;
	    if ((rs = mailmsg_start(&m)) >= 0) {
	        if ((rs = mailmsg_loadfile(&m,ifp)) >= 0) {
	            PARAMOPT_CUR	cur ;
	            if ((rs = paramopt_curbegin(pp,&cur)) >= 0) {
	                int		hl ;
	                const char	*po = PO_HEADER ;
	                const char	*hp ;
	                while (rs >= 0) {
	                    hl = paramopt_enumvalues(pp,po,&cur,&hp) ;
	                    if (hl == SR_NOTFOUND) break ;
	                    rs = hl ;

#if	CF_DEBUG
			    if (DEBUGLEVEL(4))
			    debugprintf("progfile/procinfile: hdr=>%t<\n",
				hp,hl) ;
#endif

			    if (rs >= 0) {
				rs = procinfiler(pip,ofp,emap,&m,hp,hl) ;
				c += rs ;
			    } /* end if (ok) */

	                } /* end while */
	                rs1 = paramopt_curend(pp,&cur) ;
	    		if (rs >= 0) rs = rs1 ;
	            } /* end if (paramopt-cur) */
	        } /* end if (mailmsg_loadfile) */
	        rs1 = mailmsg_finish(&m) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (open message) */
	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("progfile/procinfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procinfile) */


static int procinfiler(PROGINFO *pip,bfile *ofp, EMA *emap, MAILMSG *mmp,
		cchar *hp,int hl)
{
	NULSTR		n ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*hdr ;
	if ((rs = nulstr_start(&n,hp,hl,&hdr)) >= 0) {
	    int		vl ;
	    cchar	*vp ;
	    if ((vl = mailmsg_hdrval(mmp,hdr,&vp)) >= 0) {
		const int	linelen = pip->linelen ;

#if	CF_DEBUG
			        if (DEBUGLEVEL(4))
				    debugprintf("progfile/procinfile: "
					"val=>%t<\n",vp,strlinelen(vp,vl,50)) ;
#endif

			            if (matcasestr(emas,hp,hl) >= 0) {
	                                rs = ema_parse(emap,vp,vl) ;
				        c += rs ;
				    } else {
				        rs = bprintlines(ofp,linelen,vp,vl) ;
				    }

	    } /* end if (mailmsg_hdrval) */
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procinfiler) */


static int progfile_info(pip,ofp,ep,level)
PROGINFO	*pip ;
bfile		*ofp ;
EMA_ENT		*ep ;
int		level ;
{
	EMA_ENT		*ep2 ;
	int		rs = SR_OK ;
	int		c = 0 ;
#ifdef	COMMENT
	int		f_group ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile_info: level=%u\n",level) ;
#endif

#ifdef	COMMENT
	f_group = ((ep->type == ematype_group) && (ep->listp != NULL)) ;
#endif

	if ((rs = progentryinfo(pip,ofp,ep,level)) >= 0) {
	    c = 1 ;
	    if ((ep->type == ematype_group) && (ep->listp != NULL)) {
	        int	j ;
	        for (j = 0 ; ema_get(ep->listp,j,&ep2) >= 0 ; j += 1) {
	            if (ep2 != NULL) {
	                rs = progfile_info(pip,ofp,ep2,(level + 1)) ;
	                c += rs ;
		    }
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progfile_info) */


static int progfile_addr(pip,ofp,ep,level)
PROGINFO	*pip ;
bfile		*ofp ;
EMA_ENT		*ep ;
int		level ;
{
	CMD_LOCAL	*lsp = pip->lsp ;
	EMA_ENT		*ep2 ;
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfile_addr: level=%u\n",level) ;
#endif

	if ((! lsp->f.expand) ||
	    (ep->type != ematype_group) || (ep->listp == NULL)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progfile_addr: SINGLE progentryaddr() \n") ;
#endif

	    rs = progentryaddr(pip,ofp,ep,level) ;
	    c = 1 ;

	} /* end if */

	if ((rs >= 0) && lsp->f.expand &&
	    (ep->type == ematype_group) && (ep->listp != NULL)) {
	    int	j ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progfile_addr: GROUP progfile_addr() \n") ;
#endif

	    for (j = 0 ; ema_get(ep->listp,j,&ep2) >= 0 ; j += 1) {
	        if (ep2 != NULL) {
	            rs = progfile_addr(pip,ofp,ep2,(level + 1)) ;
	            c += rs ;
		}
	        if (rs < 0) break ;
	    } /* end for */

	} /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progfile_addr: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progfile_addr) */


