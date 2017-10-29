/* nodesfile */

/* read (process) a standard UNIX® "nodes" file */


#define	CF_DEBUGS	0		/* debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object supports reading the cluster 'nodes' file by mapping
        it into memory. No data is copied out of the file map! This is assumed
        to provide some performance advantage over reading it line by line and
        then allocating space for found strings.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<hdb.h>
#include	<localmisc.h>

#include	"nodesfile.h"


/* local defines */

#define	NODESFILE_DEFNODES	200

#define	TO_CHECK		4
#define	TO_HOLD			2


/* external subroutines */

extern uint	uceil(uint,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */

static int	nodesfile_parse(NODESFILE *) ;
static int	nodesfile_filechanged(NODESFILE *,time_t) ;
static int	nodesfile_filemap(NODESFILE *) ;
static int	nodesfile_fileunmap(NODESFILE *) ;

static int	hdb_release(HDB *) ;


/* local variables */


/* exported subroutines */


int nodesfile_open(nfp,fname,maxsize,oflags)
NODESFILE	*nfp ;
const char	fname[] ;
int		maxsize ;
int		oflags ;
{
	struct ustat	sb ;

	size_t	msize ;

	int	rs ;
	int	fd ;
	int	mflags ;
	int	mprot ;

	const char	*cp ;


	if (nfp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	rs = uc_open(fname,oflags,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto bad0 ;

	rs = u_fstat(fd,&sb) ;
	if (rs < 0)
	    goto bad1 ;

	if (! S_ISREG(sb.st_mode)) {

	    rs = SR_PROTO ;
	    goto bad1 ;
	}

	if (sb.st_size > maxsize) {

	    rs = SR_TOOBIG ;
	    goto bad1 ;
	}

	memset(nfp,0,sizeof(NODESFILE)) ;

	nfp->maxsize = maxsize ;
	nfp->pagesize = getpagesize() ;

/* fill in the file information */

	nfp->fi.oflags = oflags ;
	rs = uc_mallocstrw(fname,-1,&cp) ;
	if (rs < 0) goto bad1 ;

	nfp->fi.fname = cp ;
	nfp->fi.mtime = sb.st_mtime ;
	nfp->fi.ino = sb.st_ino ;
	nfp->fi.dev = sb.st_dev ;

/* continue and map the file */

	nfp->filesize = sb.st_size ;
	msize = uceil((uint) sb.st_size,nfp->pagesize) ;

	if (msize == 0)
	    msize = nfp->pagesize ;

	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,msize,mprot,mflags,fd,0L,&nfp->mapbuf) ;
	if (rs < 0) goto bad2 ;

	nfp->mapsize = msize ;

/* OK, initialize the search-accessing data structure */

	rs = hdb_start(&nfp->nodes,NODESFILE_DEFNODES,0,NULL,NULL) ;
	if (rs < 0)
	    goto bad3 ;

/* parse the file */

	rs = nodesfile_parse(nfp) ;
	if (rs < 0)
	    goto bad4 ;

/* normal return */
ret1:
	u_close(fd) ;

ret0:
	return rs ;

/* bad stuff */
bad4:
	hdb_finish(&nfp->nodes) ;

bad3:
	u_munmap(nfp->mapbuf,(size_t) nfp->mapsize) ;
	nfp->mapbuf = NULL ;
	nfp->mapsize = 0 ;

bad2:
	if (nfp->fi.fname != NULL) {
	    uc_free(nfp->fi.fname) ;
	    nfp->fi.fname = NULL ;
	}

bad1:
	u_close(fd) ;

bad0:
	goto ret1 ;
}
/* end subroutine (nodesfile_open) */


int nodesfile_close(nfp)
NODESFILE	*nfp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (nfp == NULL)
	    return SR_FAULT ;

	rs1 = hdb_finish(&nfp->nodes) ;
	if (rs >= 0) rs = rs1 ;

	if (nfp->mapbuf != NULL) {
	    size_t	ms = nfp->mapsize ;
	    caddr_t	ma = (caddr_t) nfp->mapbuf ;
	    rs1 = u_munmap(ma,ms) ;
	    if (rs >= 0) rs = rs1 ;
	    nfp->mapbuf = NULL ;
	    nfp->mapsize = 0 ;
	}

	if (nfp->fi.fname != NULL) {
	    rs1 = uc_free(nfp->fi.fname) ;
	    if (rs >= 0) rs = rs1 ;
	    nfp->fi.fname = NULL ;
	}

	return rs ;
}
/* end subroutine (nodesfile_close) */


int nodesfile_check(nfp,daytime)
NODESFILE	*nfp ;
time_t		daytime ;
{
	int	rs = SR_OK ;
	int	f_changed = FALSE ;


	if (nfp == NULL)
	    return SR_FAULT ;

	if (daytime == 0) daytime = time(NULL) ;

	if ((daytime - nfp->ti_check) >= TO_CHECK) {
	    nfp->ti_check = daytime ;
	    if ((rs = nodesfile_filechanged(nfp,daytime)) >= 0) {

	        f_changed = TRUE ;
	        hdb_release(&nfp->nodes) ;

	        rs = nodesfile_parse(nfp) ;

	        if (rs >= 0)
	            nfp->ti_load = daytime ;

	    } /* end if */
	} /* end if (timeout) */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (nodesfile_check) */


int nodesfile_search(nfp,nodename,nl)
NODESFILE	*nfp ;
const char	nodename[] ;
int		nl ;
{
	HDB_DATUM	key, value ;

	int	rs = SR_OK ;


	if (nfp == NULL)
	    return SR_FAULT ;

	if (nfp->mapbuf == NULL)
	    return SR_NOTOPEN ;

	if (nl < 0)
	    nl = strlen(nodename) ;

	key.buf = (void *) nodename ;
	key.len = nl ;
	rs = hdb_fetch(&nfp->nodes,key,NULL,&value) ;

#if	CF_DEBUGS
	debugprintf("nodesfile_search: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (nodesfile_search) */


int nodesfile_curbegin(nfp,curp)
NODESFILE	*nfp ;
NODESFILE_CUR	*curp ;
{
	int	rs = SR_OK ;


	if (nfp == NULL)
	    return SR_FAULT ;

	if (curp == NULL)
	    return SR_FAULT ;

	rs = hdb_curbegin(&nfp->nodes,&curp->cur) ;

	return rs ;
}
/* end subroutine (nodesfile_curbegin) */


int nodesfile_curend(nfp,curp)
NODESFILE	*nfp ;
NODESFILE_CUR	*curp ;
{
	int	rs ;


	if (nfp == NULL)
	    return SR_FAULT ;

	if (curp == NULL)
	    return SR_FAULT ;

	rs = hdb_curend(&nfp->nodes,&curp->cur) ;

	return rs ;
}
/* end subroutine (nodesfile_curend) */


int nodesfile_enum(nfp,curp,nodename,nl)
NODESFILE	*nfp ;
NODESFILE_CUR	*curp ;
char		nodename[] ;
int		nl ;
{
	HDB_DATUM	key, val ;

	int	rs ;
	int	cl = 0 ;

	const char	*cp ;


	if (nfp == NULL)
	    return SR_FAULT ;

	if (curp == NULL)
	    return SR_FAULT ;

	if ((rs = hdb_enum(&nfp->nodes,&curp->cur,&key,&val)) >= 0) {
	    cp = (const char *) key.buf ;
	    cl = (nl >= 0) ? MIN(key.len,nl) : key.len ;
	    strwcpy(nodename,cp,cl) ;
	}

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (nodesfile_enum) */


/* private subroutines */


static int nodesfile_parse(nfp)
NODESFILE	*nfp ;
{
	HDB_DATUM	key, value ;

	int	rs = SR_OK ;
	int	i ;
	int	sl, cl ;
	int	n = 0 ;

	const char	*sp, *cp ;
	const char	*ep ;


	value.buf = NULL ;
	value.len = 0 ;

	n = 0 ;
	sp = nfp->mapbuf ;
	sl = nfp->filesize ;

	ep = sp + sl ;
	while (sp < ep) {

	    while ((cl = nextfield(sp,sl,&cp)) > 0) {

#if	CF_DEBUGS
	        debugprintf("nodesfile_parse: cp=%t\n",cp,cl) ;
#endif

	        if (cp[0] == '#') {

	            while ((cp < ep) && (*cp != '\n'))
	                cp += 1 ;

	            sl -= (cp - sp) ;
	            sp = cp ;

	        } else {

	            sl -= ((cp + cl) - sp) ;
	            sp = (cp + cl) ;

/* remove trailing comments */

		    for (i = 0 ; i < cl ; i += 1) {
			if (cp[i] == '#') {
				cl = i ;
				break ;
			}
		    } /* end for */

/* store it */

	            key.buf = (void *) cp ;
	            key.len = cl ;
	            rs = hdb_store(&nfp->nodes,key,value) ;

	            n += 1 ;

	        } /* end if */

	            if (rs < 0) break ;
	    } /* end while (fields) */

#ifdef	COMMENT /* old semantics of 'nextfield(3dam)' */
	    if ((cp < ep) && (*cp == '\n'))
	        cp += 1 ;
#else /* safer semantics */
	    cp = sp ;
	    while ((cp < ep) && (*cp != '\n'))
	        cp += 1 ;

	    if ((cp < ep) && (*cp == '\n'))
	        cp += 1 ;
#endif

	    sl -= (cp - sp) ;
	    sp = cp ;

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (nodesfile_parse) */


static int nodesfile_filechanged(nfp,daytime)
NODESFILE	*nfp ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs ;
	int	f = FALSE ;


	rs = u_stat(nfp->fi.fname,&sb) ;
	if (rs < 0)
	    goto ret0 ;

	f = (nfp->fi.mtime > sb.st_mtime) ;
	f = f || (nfp->fi.ino != sb.st_ino) ;
	f = f || (nfp->fi.dev != sb.st_dev) ;

	if (f)
	    f = f && ((daytime - nfp->fi.mtime) >= TO_HOLD) ;

	if (f) {

	    nfp->filesize = sb.st_size ;

	    nfp->fi.mtime = sb.st_mtime ;
	    nfp->fi.ino = sb.st_ino ;
	    nfp->fi.dev = sb.st_dev ;

/* if the file SIZE grew, do a complete remap */

	    if (sb.st_size > nfp->mapsize) {

	        u_munmap(nfp->mapbuf,(size_t) nfp->mapsize) ;

	        nfp->mapbuf = NULL ;
	        rs = nodesfile_filemap(nfp) ;

	    }

	} /* end if (file changed) */

ret0:
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (nodesfile_filechanged) */


static int nodesfile_filemap(nfp)
NODESFILE	*nfp ;
{
	int	rs ;


	if ((rs = uc_open(nfp->fi.fname,nfp->fi.oflags,0666)) >= 0) {
	    int	fd = rs ;
	    if (nfp->filesize <= nfp->maxsize) {
		size_t	mapsize = uceil(nfp->filesize,nfp->pagesize) ;
		if (mapsize == 0) mapsize = nfp->pagesize ;
	        {
	            size_t	ms = mapsize ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*p ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&p)) >= 0) {
		        nfp->mapbuf = p ;
		        nfp->mapsize = mapsize ;
	            }
	        } /* end block */
	    } else
	        rs = SR_TOOBIG ;
	    u_close(fd) ;
	} /* end if (file-open) */

	return rs ;
}
/* end subroutine (nodesfile_filemap) */


static int nodesfile_fileunmap(nfp)
NODESFILE	*nfp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (nfp->mapbuf != NULL) {
	    caddr_t	ma = (caddr_t) nfp->mapbuf ;
	    size_t	ms = nfp->mapsize ;
	    rs1 = u_munmap(ma,ms) ;
	    if (rs >= 0) rs = rs1 ;
	    nfp->mapbuf = NULL ;
	    nfp->mapsize = 0 ;
	}

	return rs ;
}
/* end subroutine (nodesfile_fileunmap) */


static int hdb_release(hsp)
HDB		*hsp ;
{
	HDB_CUR	cur ;

	int	rs = SR_OK ;


	if (hsp == NULL)
	    return SR_FAULT ;

	hdb_curbegin(hsp,&cur) ;

	while (hdb_enum(hsp,&cur,NULL,NULL) >= 0)
	    hdb_delcur(hsp,&cur,0) ;

	hdb_curend(hsp,&cur) ;

	return rs ;
}
/* end subroutine (hdb_release) */



