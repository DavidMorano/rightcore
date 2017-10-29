/* commandments */

/* COMMANDMENTS object implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_EMPTYTERM	0		/* terminate entry on empty line */


/* revision history:

	= 1998-12-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the COMMANDMENTS datbase.


*******************************************************************************/


#define	COMMANDMENTS_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<sbuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"commandments.h"


/* local defines */

#define	COMMANDMENTS_DBNAME	"commandments"
#define	COMMANDMENTS_DBDNAME	"share/commandments"
#define	COMMANDMENTS_DBTITLE	"Commandments"
#define	COMMANDMENTS_DEFENTS	11
#define	COMMANDMENTS_NE		4

#define	COMMANDMENTS_E		struct commandments_e
#define	COMMANDMENTS_EL		struct commandments_eline

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	TO_CHECK	4


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* local structures */

struct commandments_eline {
	uint		loff ;
	uint		llen ;
} ;

struct commandments_e {
	COMMANDMENTS_EL	*lines ;
	int		n ;
	int		e, i ;
} ;


/* forward references */

static int	commandments_fileloadbegin(COMMANDMENTS *) ;
static int	commandments_fileloadend(COMMANDMENTS *) ;
static int	commandments_filemapbegin(COMMANDMENTS *) ;
static int	commandments_filemapend(COMMANDMENTS *) ;
static int	commandments_fileproc(COMMANDMENTS *) ;
static int	commandments_entfins(COMMANDMENTS *) ;
static int	commandments_checkupdate(COMMANDMENTS *,time_t) ;
static int	commandments_loadbuf(COMMANDMENTS *,
			char *,int,COMMANDMENTS_E *) ;

static int	entry_start(COMMANDMENTS_E *,int,uint,uint) ;
static int	entry_add(COMMANDMENTS_E *,uint,uint) ;
static int	entry_finish(COMMANDMENTS_E *) ;
static int	entry_mkcmp(COMMANDMENTS_E *,int) ;

static int	isempty(const char *,int) ;
static int	isstart(const char *,int,int *,int *) ;
static int	hasourdig(const char *,int) ;

static int	vecmp(const void *,const void *) ;

#if	CF_DEBUGS
static int	linenlen(const char *,int,int) ;
#endif

static int	isNotAccess(int) ;


/* exported variables */

COMMANDMENTS_OBJ	commandments = {
	"commandments",
	sizeof(COMMANDMENTS),
	sizeof(int)
} ;


/* local variables */


/* exported subroutines */


int commandments_open(COMMANDMENTS *op,cchar pr[],cchar dbname[])
{
	const int	n = COMMANDMENTS_DEFENTS ;
	const int	size = sizeof(COMMANDMENTS_E) ;
	int		rs ;
	int		c = 0 ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = COMMANDMENTS_DBNAME ;

	if ((rs = vecobj_start(&op->db,size,n,0)) >= 0) {
	    const char	*cname = COMMANDMENTS_DBDNAME ;
	    char	ubuf[USERNAMELEN+1] ;
	    char	hbuf[MAXPATHLEN+1] ;
	    rs = getusername(ubuf,USERNAMELEN,-1) ;
	    if (rs >= 0) rs = getuserhome(hbuf,MAXPATHLEN,ubuf) ;
	    if (rs >= 0) rs = mkpath3(tmpfname,hbuf,cname,dbname) ;
	    if (rs >= 0) {
	        struct ustat	sb ;
		rs = u_stat(tmpfname,&sb) ;
		if ((rs >= 0) && (! S_ISREG(sb.st_mode))) rs = SR_ISDIR ;
		if (isNotAccess(rs)) {
	   	    if ((rs = mkpath3(tmpfname,pr,cname,dbname)) >= 0) {
			rs = u_stat(tmpfname,&sb) ;
		        if ((rs >= 0) && (! S_ISREG(sb.st_mode))) 
				rs = SR_ISDIR ;
		    }
		}
		if (rs >= 0) {
		    const char	*cp ;
		    if ((rs = uc_mallocstrw(tmpfname,-1,&cp)) >= 0) {
		        op->fname = cp ;
			if ((rs = commandments_fileloadbegin(op)) >= 0) {
			    c = rs ;
			    op->magic = COMMANDMENTS_MAGIC ;
			}
			if (rs < 0) {
	    		    uc_free(op->fname) ;
	                    op->fname = NULL ;
			}
		    } /* end if (memory-allocation) */
		} /* end if */
	    } /* end if (vecobj_start) */
	    if (rs < 0)
		vecobj_finish(&op->db) ;
	} /* end if (vecobj_start) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandments_open) */


/* free up the entire vector string data structure object */
int commandments_close(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

	rs1 = commandments_fileloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	rs1 = vecobj_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (commandments_close) */


int commandments_count(COMMANDMENTS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

	rs = vecobj_count(&op->db) ;

	return rs ;
}
/* end subroutine (commandments_count) */


int commandments_max(COMMANDMENTS *op)
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		max = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

/* check for update */

	if (op->ncursors == 0) {
	    if (dt == 0) dt = time(NULL) ;
	    rs = commandments_checkupdate(op,dt) ;
	}

	if (rs >= 0) {
	    COMMANDMENTS_E	*ep ;
	    int		i ;
	    for (i = 0 ; vecobj_get(&op->db,i,&ep) >= 0 ; i += 1) {
	        if (ep == NULL) continue ;
	        if (ep->n > max)
	            max = ep->n ;
	    } /* end for */
	} /* end if (ok) */

	return (rs >= 0) ? max : rs ;
}
/* end subroutine (commandments_max) */


int commandments_audit(COMMANDMENTS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

	rs = vecobj_audit(&op->db) ;

	return rs ;
}
/* end subroutine (commandments_audit) */


/* get a string by its index */
int commandments_get(COMMANDMENTS *op,int i,char rbuf[],int rlen)
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

	if (i < 0) return SR_INVALID ;

/* check for update */

	if (op->ncursors == 0) {
	    rs = commandments_checkupdate(op,dt) ;
	}

	if (rs >= 0) {
	    COMMANDMENTS_E	e, *ep ;
	    entry_mkcmp(&e,i) ;
	    if ((rs = vecobj_search(&op->db,&e,vecmp,&ep)) >= 0) {
	        if (ep != NULL) {
	            rs = commandments_loadbuf(op,rbuf,rlen,ep) ;
	        }
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("commandments_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandments_get) */


/* enumerate entries */
int commandments_enum(COMMANDMENTS *op,int i,int *np,char rbuf[],int rlen)
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

	if (i < 0) return SR_INVALID ;

/* check for update */

	if (op->ncursors == 0) {
	    rs = commandments_checkupdate(op,dt) ;
	}

	if (rs >= 0) {
	    COMMANDMENTS_E	*ep ;
	    if ((rs = vecobj_get(&op->db,i,&ep)) >= 0) {
	        if (ep != NULL) {
	            if (np != NULL) {
	                *np = ep->n ;
	            }
	            rs = commandments_loadbuf(op,rbuf,rlen,ep) ;
	        } /* end if (non-null) */
	    } /* end if (vecobj_get) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (commandments_enum) */


/* private subroutines */


static int commandments_fileloadbegin(COMMANDMENTS *op)
{
	int		rs ;
	int		c = 0 ;

	if ((rs = commandments_filemapbegin(op)) >= 0) {
	    rs = commandments_fileproc(op) ;
	    c = rs ;
	    if (rs < 0) {
	        commandments_entfins(op) ;
	        commandments_filemapend(op) ;
	    }
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandments_fileloadbegin) */


static int commandments_fileloadend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = commandments_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = commandments_filemapend(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (commandments_fileloadend) */


static int commandments_filemapbegin(COMMANDMENTS *op)
{
	const time_t	dt = time(NULL) ;
	int		rs ;

	if ((rs = u_open(op->fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    const int		fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
	            size_t	ms = (size_t) sb.st_size ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*md ;
	            op->filesize = (sb.st_size & UINT_MAX) ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                op->mapdata = md ;
	                op->mapsize = ms ;
	                op->ti_map = dt ;
		        op->ti_mod = sb.st_mtime ;
	                op->ti_lastcheck = dt ;
	            } /* end if (u_mmap) */
	        } else
	            rs = SR_NOTSUP ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (file) */

	return rs ;
}
/* end subroutine (commandments_filemapbegin) */


static int commandments_filemapend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapdata != NULL) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	}

	return rs ;
}
/* end subroutine (commandments_filemapend) */


static int commandments_fileproc(COMMANDMENTS *op)
{
	COMMANDMENTS_E	e ;
	uint		foff = 0 ;
	int		rs = SR_OK ;
	int		ml, ll ;
	int		si ;
	int		len ;
	int		pn = -1 ;		/* previous value of 'n' */
	int		n = 0 ;
	int		c = 0 ;
	int		f_ent = FALSE ;
	int		f_notsorted = FALSE ;
	const char	*tp, *mp, *lp ;

	mp = op->mapdata ;
	ml = (op->filesize & INT_MAX) ;

	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if (! isempty(lp,ll)) {

	        if ((tp = strnchr(lp,ll,'#')) != NULL)
	            ll = (tp - lp) ;

	        if (isstart(lp,ll,&n,&si)) {

	            if (f_ent) {
	                c += 1 ;
	                rs = vecobj_add(&op->db,&e) ;
	                if (rs < 0)
	                    entry_finish(&e) ;
			f_ent = FALSE ;
	            }

	            if (rs >= 0) {
	                if ((rs = entry_start(&e,n,(foff+si),(ll-si))) >= 0) {
	                    f_ent = TRUE ;
			    if (n < pn)
				f_notsorted = TRUE ;
			    pn = n ;
			}
	            } /* end if (ok) */

	        } else {

	            if (f_ent) {
	                rs = entry_add(&e,foff,ll) ;
		    }

	        } /* end if (entry start of add) */

	    } else {

#if	CF_EMPTYTERM
	        if (f_ent) {
	            c += 1 ;
	            rs = vecobj_add(&op->db,&e) ;
	            if (rs < 0)
	                entry_finish(&e) ;
	            f_ent = FALSE ;
	        }
#else
	        rs = SR_OK ;
#endif /* CF_EMPTYTERM */

	    } /* end if (not empty) */

	    foff += len ;
	    ml -= len ;
	    mp += len ;

	    if (rs < 0) break ;
	} /* end while (readling lines) */

	if ((rs >= 0) && f_ent) {
	    c += 1 ;
	    f_ent = FALSE ;
	    rs = vecobj_add(&op->db,&e) ;
	    if (rs < 0) {
	        entry_finish(&e) ;
	    }
	}

	if (f_ent)
	    entry_finish(&e) ;

/* conidtionally sort the entries */

	if ((rs >= 0) && (c > 1) && f_notsorted) {
	    vecobj_sort(&op->db,vecmp) ;
	}

#if	CF_DEBUGS
	debugprintf("commandments_fileproc: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandments_fileproc) */


static int commandments_entfins(COMMANDMENTS *op)
{
	COMMANDMENTS_E	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

	for (i = 0 ; vecobj_get(&op->db,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        c += 1 ;
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandments_entfins) */


static int commandments_checkupdate(COMMANDMENTS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op->ncursors == 0) {
	    struct ustat	sb ;
	    if (dt == 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	    op->ti_lastcheck = dt ;
	    if ((rs = u_stat(op->fname,&sb)) >= 0) {
	        if ((sb.st_mtime > op->ti_mod) || (sb.st_mtime > op->ti_map)) {
	            f = TRUE ;
	            commandments_fileloadend(op) ;
	            rs = commandments_fileloadbegin(op) ;
	        } /* end if (update) */
	    } else if (isNotPresent(rs))
	       rs = SR_OK ;
	    } /* end if (time-out) */
	} /* end if (no cursors out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (commandments_checkupdate) */


static int commandments_loadbuf(COMMANDMENTS *op,char rbuf[],int rlen,
		COMMANDMENTS_E *ep)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

#ifdef	COMMENT
	if (ep == NULL) return SR_FAULT ;
#endif

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    COMMANDMENTS_EL	*elp = ep->lines ;
	    int			j ;
	    const char		*ebp ;
	    for (j = 0 ; j < ep->i ; j += 1) {
	        if (j > 0) sbuf_char(&b,' ') ;
	        ebp = (op->mapdata + elp->loff) ;
	        rs = sbuf_strw(&b,ebp,elp->llen) ;
	        elp += 1 ;
	        if (rs < 0) break ;
	    } /* end for */

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (commandments_loadbuf) */


static int entry_start(COMMANDMENTS_E *ep,int n,uint off,uint len)
{
	const int	ne = COMMANDMENTS_NE ;
	int		rs = SR_OK ;
	int		size ;
	void		*p ;

	if (ep == NULL)
	    return SR_FAULT ;

	memset(ep,0,sizeof(COMMANDMENTS_E)) ;

	ep->n = n ;
	size = ne * sizeof(struct commandments_eline) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    COMMANDMENTS_EL	*elp = p ;
	    ep->lines = p ;
	    ep->e = ne ;
	    elp->loff = off ;
	    elp->llen = len ;
	    ep->i += 1 ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(COMMANDMENTS_E *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->lines != NULL) {
	    rs1 = uc_free(ep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->lines = NULL ;
	}

	ep->i = 0 ;
	ep->e = 0 ;
	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_add(COMMANDMENTS_E *ep,uint off,uint len)
{
	COMMANDMENTS_EL	*elp ;
	int		rs = SR_OK ;
	int		ne ;
	int		size ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->i == ep->e) {
	    ne = ep->e + COMMANDMENTS_NE ;
	    size = ne * sizeof(struct commandments_eline) ;
	    if ((rs = uc_realloc(ep->lines,size,&elp)) >= 0) {
	        ep->e = ne ;
	        ep->lines = elp ;
	    }
	}

	if (rs >= 0) {
	    elp = (ep->lines + ep->i) ;
	    elp->loff = off ;
	    elp->llen = len ;
	    ep->i += 1 ;
	}

	return rs ;
}
/* end subroutine (entry_add) */


static int entry_mkcmp(COMMANDMENTS_E *ep,int n)
{

	if (ep == NULL) return SR_FAULT ;

	ep->n = n ;
	ep->e = 0 ;
	ep->i = 0 ;
	ep->lines = NULL ;
	return SR_OK ;
}
/* end subroutine (entry_mkcmp) */


static int isempty(cchar *lp,int ll)
{
	int		cl ;
	int		f = FALSE ;
	const char	*cp ;

	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
	    cl = sfskipwhite(lp,ll,&cp) ;
	    f = f || (cl == 0) ;
	    f = f || (cp[0] == '#') ;
	}

	return f ;
}
/* end subroutine (isempty) */


static int isstart(cchar *lp,int ll,int *np,int *sip)
{
	int		cl ;
	int		f = FALSE ;
	const char	*tp, *cp ;

	*np = -1 ;
	*sip = 0 ;
	if ((tp = strnchr(lp,ll,'.')) != NULL) {

	    cp = lp ;
	    cl = (tp - lp) ;
	    f = hasourdig(cp,cl) && (cfdeci(cp,cl,np) >= 0) ;

	    if (f) {
	        *sip = ((tp + 1) - lp) ;
	    }

	} /* end if */

	return f ;
}
/* end subroutine (isstart) */


static int hasourdig(cchar *sp,int sl)
{
	int		cl ;
	int		f = FALSE ;
	const char	*cp ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {

	    f = TRUE ;
	    while (cl && *cp) {
	        if (! isdigitlatin(*cp)) {
	            f = FALSE ;
	            break ;
	        }
	        cp += 1 ;
	        cl -= 1 ;
	    } /* end while */

	} /* end if */

	return f ;
}
/* end subroutine (hasourdig) */


static int vecmp(const void *v1p,const void *v2p)
{
	COMMANDMENTS_E	**e1pp = (COMMANDMENTS_E **) v1p ;
	COMMANDMENTS_E	**e2pp = (COMMANDMENTS_E **) v2p ;
	int		rc = 0 ;
	if (*e1pp != NULL) {
	    if (*e2pp != NULL) {
	        rc = ((*e1pp)->n - (*e2pp)->n) ;
	    } else
	        rc = -1 ;
	} else
	    rc = 1 ;
	return rc ;
}
/* end subroutine (vecmp) */


#if	CF_DEBUGS
static int linenlen(cchar *lp,int ll,int ml)
{
	int		len = INT_MAX ;
	const char	*tp ;
	if (lp == NULL) return 0 ;
	if (ll > 0) len = MIN(len,ll) ;
	if (ml > 0) len = MIN(len,ml) ;
	if ((tp = strnchr(lp,len,'\n')) != NULL) {
	    len = (tp - lp) ;
	}
	return len ;
}
/* end subroutine (linenlen) */
#endif /* CF_DEBUGS */


int isNotAccess(int rs)
{
	int		f = isNotPresent(rs) ;
	if ((! f) && (rs == SR_ISDIR)) f = TRUE ;
	return f ;
}
/* end subroutine (isNotAccess) */


