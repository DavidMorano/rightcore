/* fsdir */

/* object to read directory entries in the UNIX® file system */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-06-16, David A­D­ Morano
        This subroutine was written so that we could use a single file-system
        directory interface due to all of the changes in the POSIX standard and
        previous UNIX® standards.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code module provides a platform independent implementation of UNIX®
        file system directory access.

        This module uses the system call 'getdent(2)' to read the directories
        and to format them into entries. This is on Solaris® and perhaps some
        others (even some more but in slightly different forms), but it is not
        generally portable. A portable version of this object is located in
        'fsdirport(3dam)'. It would have been colocated in this file (with
        appropriate compile-time define switches) but it is just way too ugly to
        look at. Besides, depending on platform, it itself is not always
        multi-thread-safe or reentrant. If you want it you know where to find
        it.


*******************************************************************************/


#define	FSDIR_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/dirent.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"fsdir.h"


/* local defines */

#define	FSDIR_BMODULUS	512		/* seems to be what OS uses */
#define	FSDIR_MINBUFLEN	512		/* minimum buffer length */

#define	TO_AGAIN	10
#define	TO_NOLCK	20


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	iceil(int,int) ;
extern int	ifloor(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local typedefs */


/* local structures */


/* forward referecnces */

static int	fsdir_begin(FSDIR *,const char *) ;
static int	fsdir_end(FSDIR *) ;


/* local variables */


/* exported subroutines */


int fsdir_open(fsdir *op,cchar *dname)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (dname == NULL) return SR_FAULT ;
	if (dname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(FSDIR)) ;

#if	CF_DEBUGS
	debugprintf("fsdir_open: dname=%s\n",dname) ;
#endif

	if ((rs = fsdir_begin(op,dname)) >= 0) {
	    struct ustat	sb ;
	        if ((rs = u_fstat(op->dfd,&sb)) >= 0) {
		    if (S_ISDIR(sb.st_mode)) {
	    	        const int	psize = getpagesize() ;
	    	        int		dsize = (sb.st_size & SIZE_MAX) ;
			int		size ;
	    	        char		*bp ;
			if (dsize < FSDIR_MINBUFLEN) dsize = FSDIR_MINBUFLEN ;
			size = MIN(psize,dsize) ;
	                if ((rs = uc_valloc(size,&bp)) >= 0) {
		            op->bdata = bp ;
			    op->bsize = size ;
	                    op->magic = FSDIR_MAGIC ;
	                }
		    } else
		        rs = SR_NOTDIR ;
	        } /* end if (stat) */
#if	CF_DEBUGS
	debugprintf("fsdir_open: close-on-exec rs=%d\n",rs) ;
#endif
	    if (rs < 0) {
	        fsdir_end(op) ;
	    }
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("fsdir_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (fsdir_open) */


int fsdir_close(fsdir *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	if (op->bdata != NULL) {
	    rs1 = uc_free(op->bdata) ;
	    if (rs >= 0) rs = rs1 ;
	    op->bdata = NULL ;
	}

	rs1 = fsdir_end(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (fsdir_close) */


#ifdef	COMMENT
typedef struct dirent {
	uino_t		d_ino;		/* "inode number" of entry */
	off_t		d_off;		/* offset of disk directory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[1];	/* name of file */
} dirent_t;
#endif /* COMMENT */

int fsdir_read(fsdir *op,fsdir_ent *dirslotp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (dirslotp == NULL) return SR_FAULT ;
	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	if (op->ei >= op->blen) {
	    dirent_t	*dp = (dirent_t *) op->bdata ;
	    rs = u_getdents(op->dfd,dp,op->bsize) ;
#if	CF_DEBUGS
	debugprintf("fsdir_read: u_getdents() rs=%d\n",rs) ;
#endif
	    op->blen = rs ;
	    op->doff += rs ;
	    op->ei = 0 ;
	}

	if ((rs >= 0) && (op->blen > 0)) { /* greater-than-zero */
	    dirent_t	*dp = (dirent_t *) (op->bdata + op->ei) ;
	    const int	nlen = MAXNAMELEN ;
	    int		ml ;
	    ml = (dp->d_reclen-18) ;
	    dirslotp->ino = (uino_t) dp->d_ino ;
	    dirslotp->off = (offset_t) dp->d_off ;
	    dirslotp->reclen = (ushort) dp->d_reclen ;
	    if ((rs = snwcpy(dirslotp->name,nlen,dp->d_name,ml)) >= 0) {
	        op->eoff = dp->d_off ;
		op->ei += dp->d_reclen ;
	    }
	} else {
	    dirslotp->ino = 0 ;
	    dirslotp->off = 0 ;
	    dirslotp->reclen = 0 ;
	    dirslotp->name[0] = '\0' ;
	} /* end if */

	return rs ;
}
/* end subroutine (fsdir_read) */


int fsdir_tell(fsdir *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	rs = (op->eoff & INT_MAX) ;
	return rs ;
}
/* end subroutine (fsdir_tell) */


int fsdir_seek(fsdir *op,int o)
{
	offset_t	oo = (offset_t) o ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;
	if (o < 0) return SR_INVALID ;

	if ((rs = u_seek(op->dfd,oo,SEEK_SET)) >= 0) {
	    op->blen = 0 ;
	    op->doff = o ;
	    op->eoff = o ;
	    op->ei = 0 ;
	}

	return rs ;
}
/* end subroutine (fsdir_seek) */


/* rewind the directory */
int fsdir_rewind(fsdir *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	if (op->doff > 0) {
	    op->blen = 0 ;
	    op->doff = 0 ;
	    rs = u_rewind(op->dfd) ;
	}
	op->eoff = 0 ;
	op->ei = 0 ;

	return rs ;
}
/* end subroutine (fsdir_rewind) */


int fsdir_audit(fsdir *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	if ((op->bdata == NULL) || (op->bsize == 0)) rs = SR_BADFMT ;

	return rs ;
}
/* end subroutine (fsdir_audit) */


/* private subroutines */


static int fsdir_begin(FSDIR *op,const char *dname)
{
	int		rs ;

	if (dname[0] == '*') {
	    int	v ;
	    if ((rs = cfdeci((dname+1),-1,&v)) >= 0) {
		if ((rs = u_dup(v)) >= 0) {
		    op->f.descname = TRUE ;
		}
	    }
	} else {
	    rs = uc_open(dname,O_RDONLY,0666) ;
	}
	if (rs >= 0) {
	    op->dfd = rs ;
	    rs = uc_closeonexec(op->dfd,TRUE) ;
	    if (rs < 0) {
		u_close(op->dfd) ;
		op->dfd = -1 ;
	    }
	}
	return rs ;
}
/* end subroutine (fsdir_begin) */


static int fsdir_end(FSDIR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->dfd >= 0) {
	    rs1 = u_close(op->dfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dfd = -1 ;
	}

	return rs ;
}
/* end subroutine (fsdir_end) */


