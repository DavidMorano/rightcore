/* lastlogfile */

/* manage reading or writing a LASTLOG file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-08-22, David A­D­ Morano
        This subroutine module was adopted for use from some previous code that
        performed the similar sorts of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code is used to manage one LASTLOG type file. This sort of file is
        usually used to track the last time that a person has logged in. This
        function was implemented as part of PCS long before it was adopted as
        standard (or pseudo standard) practice in UNIX® proper.

        This code represents a shift for PCS related software from using the
        older proprietary LASTLOG feature to the newer UNIX® standard (pseudo
        standard -- whatever) mechanism.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"lastlogfile.h"


/* local defines */

#define	LASTLOGFILE_ENTSIZE	sizeof(LASTLOGFILE_ENT)
#define	LASTLOGFILE_OPENTIME	30	/* seconds */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		lastlogfile_close(LASTLOGFILE *) ;

static int	lastlogfile_checkopen(LASTLOGFILE *) ;
static int	lastlogfile_fileclose(LASTLOGFILE *) ;


/* exported subroutines */


int lastlogfile_open(LASTLOGFILE *llp,cchar *fname,int oflags)
{
	int		rs ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("lastlogfile_open: ent filename=%s\n",fname) ;
	debugprintf("lastlogfile_open: entry size=%d\n",
	    sizeof(LASTLOGFILE_ENT)) ;
#endif

	if (llp == NULL) return SR_FAULT ;

	if (fname == NULL) fname = LASTLOGFILE_FILEPATH ;
	if (fname[0] == '\0') fname = LASTLOGFILE_FILEPATH ;

	memset(llp,0,sizeof(LASTLOGFILE)) ;
	llp->oflags = oflags ;
	llp->fd = -1 ;
	llp->pagesize = getpagesize() ;

/* try to store the file name */

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    llp->fname = cp ;
	    if ((rs = lastlogfile_checkopen(llp)) >= 0) {
		struct ustat	sb ;
	        if ((rs = u_fstat(llp->fd,&sb)) >= 0) {
	            llp->fsize = sb.st_size ;
	            llp->mtime = sb.st_mtime ;
	            llp->magic = LASTLOGFILE_MAGIC ;
		}
		if (rs < 0)
		    lastlogfile_fileclose(llp) ;
	    } /* end if (file-open) */
	    if (rs < 0) {
	        uc_free(llp->fname) ;
	        llp->fname = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("lastlogfile_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (lastlogfile_open) */


/* close this lastlogfile data structure */
int lastlogfile_close(LASTLOGFILE *llp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (llp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

	if (llp->fd >= 0) {
	    rs1 = u_close(llp->fd) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (llp->fname != NULL) {
	    rs1 = uc_free(llp->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    llp->fname = NULL ;
	}

	llp->magic = 0 ;
	return rs ;
}
/* end subroutine (lastlogfile_close) */


/* read an entry */
int lastlogfile_readentry(LASTLOGFILE *llp,uid_t uid,LASTLOGFILE_ENT *ep)
{
	LASTLOGFILE_ENT	e ;
	offset_t	loc ;
	int		rs = SR_OK ;

	if (llp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

/* check if operation is allowed */

	if (((llp->oflags & O_RDONLY) != O_RDONLY) &&
	    ((llp->oflags & O_RDWR) != O_RDWR))
	    return SR_ACCESS ;

	loc = uid * LASTLOGFILE_ENTSIZE ;
	if (loc >= llp->fsize)
	    return SR_EOF ;

/* proceed with operation */

	if (ep == NULL)
	    ep = &e ;

	if (llp->fd < 0)
	    rs = lastlogfile_checkopen(llp) ;

	if (rs >= 0)
	    rs = u_pread(llp->fd,ep,LASTLOGFILE_ENTSIZE,loc) ;

	if ((rs >= 0) && (ep->ll_time == 0))
	    rs = SR_BADSLT ;

	return rs ;
}
/* end subroutine (lastlogfile_readentry) */


/* write an entry */
int lastlogfile_writeentry(LASTLOGFILE *llp,uid_t uid,LASTLOGFILE_ENT *ep)
{
	offset_t	loc ;
	int		rs = SR_OK ;

	if (llp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

/* check if operation is allowed */

	if (((llp->oflags & O_WRONLY) != O_WRONLY) &&
	    ((llp->oflags & O_RDWR) != O_RDWR))
	    return SR_ACCESS ;

/* proceed with operation */

	if (llp->fd < 0) {
	    rs = lastlogfile_checkopen(llp) ;
	}

	if (rs >= 0) {
	    loc = uid * LASTLOGFILE_ENTSIZE ;
	    rs = u_pwrite(llp->fd,ep,LASTLOGFILE_ENTSIZE,loc) ;
	}

	return rs ;
}
/* end subroutine (lastlogfile_writeentry) */


/* read the information from an entry */
int lastlogfile_readinfo(LASTLOGFILE *llp,uid_t uid,time_t *tp,char *line,
		char *hostname)
{
	LASTLOGFILE_ENT	e ;
	offset_t	loc ;
	const int	of = llp->oflags ;
	const int	esize = LASTLOGFILE_ENTSIZE ;
	int		rs = SR_OK ;

	if (llp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;
	if (uid < 0) return SR_INVALID ;

/* check if operation is allowed */

	if (((of & O_RDONLY) != O_RDONLY) && ((of & O_RDWR) != O_RDWR))
	    return SR_ACCESS ;

	loc = (uid * esize) ;
	if (loc >= llp->fsize)
	    return SR_EOF ;

/* proceed with operation */

	if (llp->fd < 0)
	    rs = lastlogfile_checkopen(llp) ;

	if (rs >= 0) {
	    if ((rs = u_pread(llp->fd,&e,esize,loc)) >= 0) {
		if ((rs > 0) && (e.ll_time != 0)) {

	    	    if (tp != NULL)
	                *tp = e.ll_time ;

	            if (hostname != NULL)
	                strwcpy(hostname,e.ll_host,LASTLOGFILE_LHOST) ;

	            if (line != NULL)
	                strwcpy(line,e.ll_line,LASTLOGFILE_LLINE) ;

	        } else {
	            rs = 0 ;

	            if (tp != NULL)
	                *tp = 0 ;

	            if (hostname != NULL)
	                hostname[0] = '\0' ;

	            if (line != NULL)
	                line[0] = '\0' ;

	        } /* end if */
	    } /* end if (u_pread) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (lastlogfile_readinfo) */


/* write information to an entry */
int lastlogfile_writeinfo(LASTLOGFILE *llp,uid_t uid,time_t t,cchar *line,
		cchar *hostname)
{
	LASTLOGFILE_ENT	e ;
	offset_t	loc ;
	int		rs = SR_OK ;

	if (llp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

/* check if operation is allowed */

	if (((llp->oflags & O_WRONLY) != O_WRONLY) &&
	    ((llp->oflags & O_RDWR) != O_RDWR))
	    return SR_ACCESS ;

/* proceed with operation */

	if (llp->fd < 0) {
	    rs = lastlogfile_checkopen(llp) ;
	}

	if (rs >= 0) {

	    (void) memset(&e,0,sizeof(LASTLOGFILE_ENT)) ;

	    if (t < 0)
	        t = time(NULL) ;

	    e.ll_time = t ;

	    if (hostname != NULL) {
	        strncpy(e.ll_host,hostname,LASTLOGFILE_LHOST) ;
	    }

	    if (line != NULL) {
	        strncpy(e.ll_line,line,LASTLOGFILE_LLINE) ;
	    }

	    loc = uid * LASTLOGFILE_ENTSIZE ;
	    rs = u_pwrite(llp->fd,&e,LASTLOGFILE_ENTSIZE,loc) ;

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (lastlogfile_writeinfo) */


/* check up on the object */
int lastlogfile_check(LASTLOGFILE *llp,time_t daytime)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		f_close = FALSE ;

	if (llp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

	if (llp->fd < 0)
	    return SR_OK ;

	if ((u_fstat(llp->fd,&sb) < 0) || (sb.st_mtime > llp->mtime))
	    f_close = TRUE ;

	if (! f_close) {
	    if (daytime <= 0) daytime = time(NULL) ;
	    if (daytime > (llp->otime + LASTLOGFILE_OPENTIME))
	        f_close = TRUE ;
	}

	if (f_close) {
	    rs = u_close(llp->fd) ;
	    llp->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (lastlogfile_check) */


/* initialize a cursor for enumeration */
int lastlogfile_curbegin(LASTLOGFILE *llp,LASTLOGFILE_CUR *curp)
{

	if (llp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (lastlogfile_curbegin) */


/* free up a cursor */
int lastlogfile_curend(LASTLOGFILE *llp,LASTLOGFILE_CUR *curp)
{

	if (llp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (lastlogfile_curend) */


/* enumerate entries */
int lastlogfile_enuminfo(LASTLOGFILE *llp,LASTLOGFILE_CUR *curp,uid_t *up,
		time_t *tp,char *line,char *hostname)
{
	int		rs = SR_OK ;
	int		i ;

	if (llp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (llp->magic != LASTLOGFILE_MAGIC) return SR_NOTOPEN ;

	i = (curp->i < 0) ? 0 : (curp->i + 1) ;

#if	CF_DEBUGS
	debugprintf("lastlogfile_enuminfo: i=%d\n",i) ;
#endif

	while (rs >= 0) {
	    if ((rs = lastlogfile_readinfo(llp,i,tp,line,hostname)) > 0) break ;
	    i += 1 ;
	} /* end while */

	if (rs >= 0) {
	    if (up != NULL) *up = i ;
	    curp->i = i ;
	}

	return rs ;
}
/* end subroutine (lastlogfile_enuminfo) */


/* private subroutines */


static int lastlogfile_checkopen(LASTLOGFILE *llp)
{
	int		rs = SR_OK ;

	if (llp->fd < 0) {
	    rs = u_open(llp->fname,llp->oflags,0660) ;
	    llp->fd = rs ;
	    llp->otime = time(NULL) ;
	}

	return (rs >= 0) ? llp->fd : rs ;
}
/* end subroutine (lastlogfile_checkopen) */


static int lastlogfile_fileclose(LASTLOGFILE *llp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (llp->fd >= 0) {
	    rs1 = u_close(llp->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    llp->fd = -1 ;
	}
	return rs ;
}
/* end subroutine (lastlogfile_fileclose) */


