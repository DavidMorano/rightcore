/* userports */

/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object reads the USERPORTS DB and provides for queries to it.

	Synopsis:

	int userports_query(op,uid,protoname,port)
	USERPORTS	*op ;
	uid_t		uid ;
	const char	protoname[] ;
	int		port ;

	Arguments:

	op		object pointer
	uid		UID to check under
	protoname	protocol name: if NULL check all protocols
	port		port number to check

	Returns:

	>=0		query found
	<0		error (likely protocol not found)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<pwcache.h>
#include	<filemap.h>
#include	<field.h>
#include	<getax.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"userports.h"


/* local defines */

#undef	ENTRY
#define	ENTRY		struct entry_elem

#define	DEFENTS		20
#define	DEFSIZE		512

#define	MAXFSIZE	(4 * 1024 * 1024)
#define	MAXPWENT	20
#define	MAXPWTTL	(200*60*60)

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	PROTONAME
#define	PROTONAME	"tcp"
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	getportnum(const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct entry_elem {
	uid_t		uid ;
	int		port ;
	int		protoidx ;
	int		portidx ;
} ;


/* forward references */

static int userports_procfile(USERPORTS *) ;
static int userports_procline(USERPORTS *,PWCACHE *,const char *,int) ;
static int userports_procent(USERPORTS *,uid_t,const char *,int) ;
static int userports_procenter(USERPORTS *,uid_t,const char *,const char *) ;


/* local variables */

static const uchar	fterms[] = {
	0x00, 0x02, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*defprotos[] = {
	"tcp",
	"udp",
	"ddp",
	NULL
} ;


/* exported subroutines */


int userports_open(op,fname)
USERPORTS	*op ;
const char	fname[] ;
{
	const int	dents = DEFENTS ;
	const int	dsize = DEFSIZE ;
	const int	vo = 0 ; /* sorting is not needed (now) */
	int		rs ;
	const char	*cp ;

	if (op == NULL)
	    return SR_FAULT ;

	if ((fname == NULL) || (fname[0] == '\0'))
	    fname = USERPORTS_FNAME ;

#if	CF_DEBUGS
	debugprintf("userports_open: fname=%s\n",fname) ;
#endif

	memset(op,0,sizeof(USERPORTS)) ;

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    const int	size = sizeof(ENTRY) ;
	    op->fname = cp ;
	    if ((rs = vecobj_start(&op->ents,size,dents,vo)) >= 0) {
	        if ((rs = vecpstr_start(&op->protos,dents,dsize,0)) >= 0) {
	            if ((rs = vecpstr_start(&op->ports,dents,dsize,0)) >= 0) {
	                if ((rs = userports_procfile(op)) >= 0) {
	                    op->magic = USERPORTS_MAGIC ;
	                } /* end if (procfile) */
		        if (rs < 0)
		            vecpstr_finish(&op->ports) ;
	            } /* end if (ports) */
		    if (rs < 0)
		        vecpstr_finish(&op->protos) ;
	        } /* end if (protos) */
		if (rs < 0)
		    vecobj_finish(&op->ents) ;
	    } /* end if (ents) */
	    if (rs < 0) {
		if (op->fname != NULL) {
		    uc_free(op->fname) ;
		    op->fname = NULL ;
		}
	    }
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (userports_open) */


int userports_close(op)
USERPORTS	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != USERPORTS_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = vecpstr_finish(&op->ports) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecpstr_finish(&op->protos) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->ents) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (userports_close) */


int userports_query(op,uid,protoname,port)
USERPORTS	*op ;
uid_t		uid ;
const char	protoname[] ;
int		port ;
{
	ENTRY		*ep ;
	int		rs = SR_OK ;
	int		protoidx = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != USERPORTS_MAGIC) return SR_NOTOPEN ;

	if (uid < 0) return SR_INVALID ;
	if (port < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("userports_query: ent\n") ;
	debugprintf("userports_query: uid=%d\n",uid) ;
	debugprintf("userports_query: protoname=%s\n",protoname) ;
	debugprintf("userports_query: port=%d\n",port) ;
#endif

	if ((protoname != NULL) && (protoname[0] != '\0')) {
	    rs = vecpstr_already(&op->protos,protoname,-1) ;
	    protoidx = rs ;
	}

#if	CF_DEBUGS
	debugprintf("userports_query: mid rs=%d\n",rs) ;
	debugprintf("userports_query: mid protoidx=%d\n",protoidx) ;
#endif

	if (rs >= 0) {
	    int	i ;
	    int	f ;
	    for (i = 0 ; (rs = vecobj_get(&op->ents,i,&ep)) >= 0 ; i += 1) {
	        if (ep != NULL) {
#if	CF_DEBUGS
		debugprintf("userports_query: e i=%d\n",i) ;
		debugprintf("userports_query: e uid=%d\n",ep->uid) ;
		debugprintf("userports_query: e port=%d\n",ep->port) ;
		debugprintf("userports_query: e protoidx=%d\n",ep->protoidx) ;
#endif
		    f = (uid == ep->uid) ;
		    f = f && (port == ep->port) ;
		    if (f && (protoidx > 0)) {
		        f = (protoidx == ep->protoidx) ;
		    }
		    if (f) break ;
	        }
	    } /* end for */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("userports_query: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (userports_query) */


int userports_curbegin(op,curp)
USERPORTS	*op ;
USERPORTS_CUR	*curp ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	return rs ;
}
/* end subroutine (userports_curbegin) */


int userports_curend(op,curp)
USERPORTS	*op ;
USERPORTS_CUR	*curp ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	return rs ;
}
/* end subroutine (userports_curend) */


int userports_enum(op,curp,entp)
USERPORTS	*op ;
USERPORTS_CUR	*curp ;
USERPORTS_ENT	*entp ;
{
	ENTRY		*ep ;
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (entp == NULL) return SR_FAULT ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

#if	CF_DEBUGS
	debugprintf("userports_enum: i=%u\n",i) ;
#endif

	while ((rs = vecobj_get(&op->ents,i,&ep)) >= 0) {
	    if (ep != NULL) break ;
	    i += 1 ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("userports_enum: mid rs=%d i=%u\n",rs,i) ;
#endif

	if (rs >= 0) {
	    const char	*cp ;
	    entp->uid = ep->uid ;
	    if (rs >= 0) {
#if	CF_DEBUGS
	debugprintf("userports_enum: protoidx=%u\n",ep->protoidx) ;
#endif
	        rs = vecpstr_get(&op->protos,ep->protoidx,&cp) ;
	        entp->protocol = cp ;
#if	CF_DEBUGS
	debugprintf("userports_enum: PROTOS vecpstr_get() rs=%d\n",rs) ;
#endif
	    }
	    if (rs >= 0) {
	        rs = vecpstr_get(&op->ports,ep->portidx,&cp) ;
	        entp->portname = cp ;
#if	CF_DEBUGS
	debugprintf("userports_enum: PORTS vecpstr_get() rs=%d\n",rs) ;
#endif
	    }
	    curp->i = i ;
	} /* end if (found entry) */

#if	CF_DEBUGS
	debugprintf("userports_enum: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (userports_enum) */


int userports_fetch(op,curp,uid,entp)
USERPORTS	*op ;
USERPORTS_CUR	*curp ;
uid_t		uid ;
USERPORTS_ENT	*entp ;
{
	ENTRY		*ep ;
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (entp == NULL) return SR_FAULT ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

	while ((rs = vecobj_get(&op->ents,i,&ep)) >= 0) {
	    if (ep != NULL) {
		if (ep->uid == uid) break ;
	    }
	    i += 1 ;
	} /* end while */

	if (rs >= 0) {
	    const char	*cp ;
	    entp->uid = ep->uid ;
	    if (rs >= 0) {
	        rs = vecpstr_get(&op->protos,ep->protoidx,&cp) ;
	        entp->protocol = cp ;
	    }
	    if (rs >= 0) {
	        rs = vecpstr_get(&op->ports,ep->portidx,&cp) ;
	        entp->portname = cp ;
	    }
	    curp->i = i ;
	} /* end if (found entry) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (userports_fetch) */


/* private subroutines */


static int userports_procfile(USERPORTS *op)
{
	PWCACHE		pwc ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("userports_procfile: fname=%s\n",op->fname) ;
#endif

	if ((rs = pwcache_start(&pwc,MAXPWENT,MAXPWTTL)) >= 0) {
	    struct ustat	sb ;
	    FILEMAP		fm, *fmp = &fm ;
	    const size_t	fsize = MAXFSIZE ;
	    const int	of = O_RDONLY ;
	    int		ll ;
	    const char	*lp ;

#if	CF_DEBUGS
	debugprintf("userports_procfile: filemap_open()\n") ;
#endif

	    if ((rs = filemap_open(fmp,op->fname,of,fsize)) >= 0) {

	        rs = filemap_stat(fmp,&sb) ;

#if	CF_DEBUGS
	debugprintf("userports_procfile: filemap_stat() rs=%d\n",rs) ;
#endif

	        op->fi.mtime = sb.st_mtime ;
	        op->fi.dev = sb.st_dev ;
	        op->fi.ino = sb.st_ino ;

	        while (rs >= 0) {
	            rs = filemap_getline(fmp,&lp) ;
	            ll = rs ;
	            if (rs <= 0) break ;

#if	CF_DEBUGS
	debugprintf("userports_procfile: line=»%t«\n",
		    lp,strlinelen(lp,ll,40)) ;
#endif

		    if (lp[ll-1] == '\n') ll -= 1 ;

		    if (ll > 0) {
	                rs = userports_procline(op,&pwc,lp,ll) ;
	                if (rs > 0) c += rs ;
		    }

	        } /* end if (reading lines) */

#if	CF_DEBUGS
	debugprintf("userports_procfile: mid1 rs=%d\n",rs) ;
#endif

	        filemap_close(fmp) ;
	    } /* end if (filemap) */

#if	CF_DEBUGS
	debugprintf("userports_procfile: mid2 rs=%d\n",rs) ;
#endif
	    rs1 = pwcache_finish(&pwc) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (pwcache) */

#if	CF_DEBUGS
	debugprintf("userports_procfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (userports_procfile) */


static int userports_procline(op,pwcp,lp,ll)
USERPORTS	*op ;
PWCACHE		*pwcp ;
const char	*lp ;
int		ll ;
{
	FIELD		fsb ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		*pwbuf ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    int		fl ;
	    const char	*fp ;

	    if ((fl = field_get(&fsb,fterms,&fp)) > 0) {
		struct passwd	pw ;
		char		un[USERNAMELEN+1] ;

	        strdcpy1w(un,USERNAMELEN,fp,fl) ;

	        if ((rs1 = pwcache_lookup(pwcp,&pw,pwbuf,pwlen,un)) >= 0) {
	            uid_t	uid = pw.pw_uid ;

#if	CF_DEBUGS
		debugprintf("userports_procline: u=%s rs=%d\n",un,rs) ;
#endif

		    while ((rs >= 0) && (fsb.term != '#')) {
	                fl = field_get(&fsb,fterms,&fp) ;
	                if (fl == 0) continue ;
			if (fl < 0) break ;

#if	CF_DEBUGS
		debugprintf("userports_procline: f=%t\n",fp,fl) ;
#endif

	                rs = userports_procent(op,uid,fp,fl) ;
	                if (rs > 0) c += rs ;

#if	CF_DEBUGS
			debugprintf("userports_procline: "
				"userports_procent() rs=%d\n",
				rs) ;
#endif

	            } /* end if (ports) */

	        } else if (rs != SR_NOTFOUND)
	            rs = rs1 ;

	    } /* end if */

	    field_finish(&fsb) ;
	} /* end if (field) */
	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (userports_procline) */


static int userports_procent(op,uid,fp,fl)
USERPORTS	*op ;
uid_t		uid ;
const char	*fp ;
int		fl ;
{
	NULSTR		portstr ;
	int		rs ;
	int		cl = 0 ;
	int		c = 0 ;
	const char	*tp, *cp ;
	const char	*portspec ;

	cp = NULL ;
	if ((tp = strnchr(fp,fl,':')) != NULL) {
	    cp = fp ;
	    cl = (tp-fp) ;
	    fl = ((fp+fl)-(tp+1)) ;
	    fp = (tp+1) ;
	}

#if	CF_DEBUGS
	debugprintf("userports_procent: portname=»%t«\n",fp,fl) ;
	debugprintf("userports_procent: protocol=»%t«\n",cp,cl) ;
#endif

	if ((rs = nulstr_start(&portstr,fp,fl,&portspec)) >= 0) {
	    const char	*pn ;

	    if ((cp != NULL) && (cl > 0)) {
	        NULSTR	protostr ;

	        if ((rs = nulstr_start(&protostr,cp,cl,&pn)) >= 0) {

	            rs = userports_procenter(op,uid,pn,portspec) ;
	            if (rs > 0) c += 1 ;

	            nulstr_finish(&protostr) ;
	        } /* end if (nulstr) */

	    } else {
	        int	i ;

	        for (i = 0 ; defprotos[i] != NULL ; i += 1) {
	            pn = defprotos[i] ;

	            rs = userports_procenter(op,uid,pn,portspec) ;
	            if (rs > 0) c += 1 ;

	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if */

	    nulstr_finish(&portstr) ;
	} /* end if (nulstr) */

#if	CF_DEBUGS
	debugprintf("userports_procent: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (userports_procent) */


static int userports_procenter(op,uid,pn,portspec)
USERPORTS	*op ;
uid_t		uid ;
const char	*pn ;
const char	*portspec ;
{
	ENTRY		e ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("userports_procenter: protocol=»%s«\n",pn) ;
	debugprintf("userports_procenter: portname=»%s«\n",portspec) ;
#endif

	memset(&e,0,sizeof(ENTRY)) ;
	e.uid = uid ;

	if ((rs1 = getportnum(pn,portspec)) >= 0) {
	    e.port = rs1 ;
	    f = TRUE ;

	    if (rs >= 0) {
		vecpstr	*plp = &op->protos ;
	        rs = vecpstr_adduniq(plp,pn,-1) ;
		if (rs == INT_MAX) rs = vecpstr_find(plp,pn) ;
	        e.protoidx = rs ;
	    }

	    if (rs >= 0) {
		vecpstr	*plp = &op->ports ;
	        rs = vecpstr_adduniq(plp,portspec,-1) ;
		if (rs == INT_MAX) rs = vecpstr_find(plp,portspec) ;
	        e.portidx = rs ;
	    }

	    if (rs >= 0)
	        rs = vecobj_add(&op->ents,&e) ;

	} else if (rs1 != SR_NOTFOUND)
	    rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("userports_procenter: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (userports_procenter) */


