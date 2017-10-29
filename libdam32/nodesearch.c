/* nodesearch */

/* support searching for a node name in the cluster */


#define	CF_DEBUGS	0		/* debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object provides multiplexing of two ways to search the
        cluster 'nodes' file. If the file is 'regular', the NODESFILE object is
        used to process it (for speed). If the file is anything other than a
        'regular' file, then a HDBSTR object is used to process it and the file
        is read line-by-line not assuming that the file may be seekable
        (suitable for pipes, FIFOs, sockets, and the like).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdbstr.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"nodesfile.h"
#include	"nodesearch.h"


/* local defines */

#define	TO_CHECK	4
#define	TO_HOLD		2		/* file hold time */

#define	BUFLEN		(2 * MAXPATHLEN)
#define	DATABUFLEN	30


/* external subroutines */

extern int	hdbstr_loadfile1(HDBSTR *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int	nodesearch_filechanged(NODESEARCH *,time_t) ;

#ifdef	COMMENT
static int	hdbstr_search(HDBSTR *,const char *,int) ;
#endif

static int	hdbstr_release(HDBSTR *) ;


/* local variables */


/* exported subroutines */


int nodesearch_open(nsp,fname,filesize,n)
NODESEARCH	*nsp ;
const char	fname[] ;
int		filesize ;
int		n ;
{
	struct ustat	sb ;
	int		rs ;

	if (nsp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	memset(nsp,0,sizeof(NODESEARCH)) ;

/* save the file name */

	nsp->fi.fname = mallocstr(fname) ;

	if (nsp->fi.fname == NULL) {
	    rs = SR_NOMEM ;
	    goto bad0 ;
	}

/* check if the file is not a directory */

	rs = u_stat(nsp->fi.fname,&sb) ;

	if ((rs >= 0) && S_ISDIR(sb.st_mode))
	    rs = SR_ISDIR ;

	if (rs < 0)
		goto bad1 ;

/* load the file itself */

	rs = nodesfile_open(&nsp->a,fname,filesize,O_RDONLY) ;

	if ((rs == SR_PROTO) || (rs == SR_TOOBIG)) {

	    nsp->f.sw = TRUE ;
	    rs = hdbstr_start(&nsp->b,n) ;

#if	CF_DEBUGS
	    debugprintf("nodesearch_open: hdbstr_start() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        rs = hdbstr_loadfile1(&nsp->b,fname) ;

#if	CF_DEBUGS
	        debugprintf("nodesearch_open: hdbstr_loadfile1() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            time_t	daytime = time(NULL) ;

	            nsp->ti_load = daytime ;
	            if ((rs = u_stat(fname,&sb)) >= 0) {
	                nsp->fi.mtime = sb.st_mtime ;
	                nsp->fi.ino = sb.st_ino ;
	                nsp->fi.dev = sb.st_dev ;
	            }

	        }
	    }

	} /* end if (wrong type of file) */

ret0:
	return rs ;

/* bad stuff */
bad1:
	if (nsp->fi.fname != NULL) {
	    uc_free(nsp->fi.fname) ;
	    nsp->fi.fname = NULL ;
	}

bad0:
	return rs ;
}
/* end subroutine (nodesearch_open) */


int nodesearch_close(nsp)
NODESEARCH	*nsp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (nsp == NULL)
	    return SR_FAULT ;

	if (! nsp->f.sw) {
	    rs1 = nodesfile_close(&nsp->a) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    rs1 = hdbstr_finish(&nsp->b) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (nsp->fi.fname != NULL) {
	    rs1 = uc_free(nsp->fi.fname) ;
	    if (rs >= 0) rs = rs1 ;
	    nsp->fi.fname = NULL ;
	}

	return rs ;
}
/* end subroutine (nodesearch_close) */


int nodesearch_check(nsp,daytime)
NODESEARCH	*nsp ;
time_t		daytime ;
{
	int	rs = SR_OK ;


	if ((daytime - nsp->ti_check) < TO_CHECK)
	    return SR_OK ;

	nsp->ti_check = daytime ;
	if (! nsp->f.sw) {

	    rs = nodesfile_check(&nsp->a,daytime) ;

	} else {

	    rs = nodesearch_filechanged(nsp,daytime) ;

	    if (rs > 0) {

	        hdbstr_release(&nsp->b) ;

	        rs = hdbstr_loadfile1(&nsp->b,nsp->fi.fname) ;

#if	CF_DEBUGS
	        debugprintf("nodesearch_open: hdbstr_loadfile1() rs=%d\n",rs) ;
#endif

	        if (rs >= 0)
	            nsp->ti_load = daytime ;

	    }

	} /* end if */

	return rs ;
}
/* end subroutine (nodesearch_check) */


int nodesearch_search(nsp,nodename,nl)
NODESEARCH	*nsp ;
const char	nodename[] ;
int		nl ;
{
	int		rs = SR_NOANODE ;
	int		sw ;
	int		f_found ;
	const char	*cp ;

	if (nsp == NULL) return SR_FAULT ;

	if (nl < 0)
	    nl = strlen(nodename) ;

	sw = nsp->f.sw ;
	switch (sw) {
	case 0:
	    rs = nodesfile_search(&nsp->a,nodename,nl) ;
	    f_found = (rs >= 0) ;
	    break ;
	case 1:
	    rs = hdbstr_fetch(&nsp->b,(char *) nodename,nl,NULL,&cp) ;
	    f_found = (rs >= 0) ;
	    break ;
	} /* end switch */

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (nodesearch_search) */


int nodesearch_curbegin(nsp,curp)
NODESEARCH	*nsp ;
NODESEARCH_CUR	*curp ;
{
	int		rs = SR_NOANODE ;
	int		sw ;

	if (nsp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	sw = nsp->f.sw ;
	switch (sw) {
	case 0:
	    rs = nodesfile_curbegin(&nsp->a,&curp->c1) ;
	    break ;
	case 1:
	    rs = hdbstr_curbegin(&nsp->b,&curp->c2) ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (nodesearch_curbegin) */


int nodesearch_curend(nsp,curp)
NODESEARCH	*nsp ;
NODESEARCH_CUR	*curp ;
{
	int		rs = SR_NOANODE ;
	int		sw ;

	if (nsp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	sw = nsp->f.sw ;
	switch (sw) {
	case 0:
	    rs = nodesfile_curend(&nsp->a,&curp->c1) ;
	    break ;
	case 1:
	    rs = hdbstr_curend(&nsp->b,&curp->c2) ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (nodesearch_curend) */


int nodesearch_enum(nsp,curp,nodename,nl)
NODESEARCH	*nsp ;
NODESEARCH_CUR	*curp ;
char		nodename[] ;
int		nl ;
{
	int		rs = SR_NOANODE ;
	int		sw ;
	int		kl, vl ;
	const char	*kp, *vp ;

	if (nsp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (nodename == NULL) return SR_FAULT ;

	if (nl < 0)
	    nl = strlen(nodename) ;

	sw = nsp->f.sw ;
	switch (sw) {
	case 0:
	    rs = nodesfile_enum(&nsp->a,&curp->c1,nodename,nl) ;
	    break ;
	case 1:
	    rs = hdbstr_enum(&nsp->b,&curp->c2,&kp,&vp,&vl) ;
	    if (rs >= 0) {
	        kl = (nl >= 0) ? MIN(rs,nl) : rs ;
	        strwcpy(nodename,kp,kl) ;
	    }
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (nodesearch_enum) */


/* private subroutines */


static int nodesearch_filechanged(nsp,daytime)
NODESEARCH	*nsp ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs ;
	int	f ;


	rs = u_stat(nsp->fi.fname,&sb) ;
	if (rs < 0) goto ret0 ;

	f = (nsp->fi.mtime > sb.st_mtime) ;
	f = f || (nsp->fi.ino != sb.st_ino) ;
	f = f || (nsp->fi.dev != sb.st_dev) ;

	if (f)
	    f = f && ((daytime - nsp->fi.mtime) >= TO_HOLD) ;

	if (f) {
	    nsp->fi.mtime = sb.st_mtime ;
	    nsp->fi.ino = sb.st_ino ;
	    nsp->fi.dev = sb.st_dev ;
	}

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (nodesearch_filechanged) */


#ifdef	COMMENT

static int hdbstr_search(hsp,nodename,nl)
HDBSTR		*hsp ;
const char	nodename[] ;
int		nl ;
{
	int	rs ;
	int	f_found ;


	if (hsp == NULL)
	    return SR_FAULT ;

	if (nl < 0)
	    nl = strlen(nodename) ;

	rs = hdbstr_fetch(hsp,(char *) nodename,nl,NULL,NULL) ;
	f_found = (rs >= 0) ;

#if	CF_DEBUGS
	debugprintf("nodesfile_search: ret rs=%d \n",rs) ;
#endif

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (hdbstr_search) */

#endif /* COMMENT */


static int hdbstr_release(hsp)
HDBSTR		*hsp ;
{
	HDBSTR_CUR	cur ;

	int	rs = SR_OK ;


	if (hsp == NULL)
	    return SR_FAULT ;

	hdbstr_curbegin(hsp,&cur) ;

	while (hdbstr_enum(hsp,&cur,NULL,NULL,NULL) >= 0)
		hdbstr_delcur(hsp,&cur,0) ;

	hdbstr_curend(hsp,&cur) ;

	return rs ;
}
/* end subroutine (hdbstr_release) */



