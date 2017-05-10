/* bcontrol */

/* "Basic I/O" package similiar to the standard UNIX® "stdio" package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UNIXAPPEND	1		/* file append works? */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine performs various control functions on the
	file.  Some of these are:

	+ getting the file status from the kernel
	+ locking and unlocking the file
	+ truncating the file
	+ switching file buffering modes
	+ setting or getting the kernel file flags


*******************************************************************************/


#define	BFILE_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */

#if	CF_DEBUGS
struct fcmder {
	char	*n ;
	int	v ;
} ;
#endif /* CF_DEBUGS */


/* forward references */

static int	bcontrol_lock(bfile *,struct flock *,int,int,int) ;

#if	CF_DEBUGS
static char	*fcmdname(int) ;
#endif


/* local variables */

#if	CF_DEBUGS
static const struct fcmder	fcmds[] = {
	{ "OGETLK", 5 },
	{ "SETLK", 6 },
	{ "SETLKW", 7 },
	{ "GETLK", 14 },
	{ "SETLK", 34 },
	{ "SETLKW", 35 },
	{ "GETLK", 33 },
	{ "SETLK64", 34 },
	{ "SETLKW64", 35 },
	{ "GETLK64", 33 },
	{ "SETLK64", 6 },
	{ "SETLKW64", 7 },
	{ "GETLK64", 14 },
	{ NULL, -1 }
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int bcontrol(bfile *fp,int cmd,...)
{
	struct flock	fl, *fsp ;

	va_list		ap ;

	int		rs = SR_OK ;
	int		i, fcmd ;
	int		f_timedcmd = FALSE ;


	if (fp == NULL)
	    return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (fp->f.nullfile) {
	    rs = SR_NOSYS ;
	    goto ret0 ;
	}

	va_begin(ap,cmd) ;
	switch (cmd) {

	case BC_TELL:
	    {
		BFILE_OFF	*bop ;
	        bop = (BFILE_OFF *) va_arg(ap,void *) ;
		if (bop != NULL) {
	            *bop = fp->offset ;
		} else
		    rs = SR_FAULT ;
	    }
	    break ;

	case BC_BUF:
	    fp->bm = bfile_bmall ;
	    break ;

	case BC_LINEBUF:
	    fp->bm = bfile_bmline ;
	    break ;

	case BC_UNBUF:
	    fp->bm = bfile_bmnone ;
	    break ;

	case BC_FD:
	    {
	        int *rip = (int *) va_arg(ap,int *) ;
	        rs = fp->fd ;
		if (rip != NULL) *rip = fp->fd ;
	    }
	    break ;

	case BC_STAT:
	    {
	    	struct ustat *ssp = (struct ustat *) va_arg(ap,void *) ;

		if (ssp != NULL) {
#if	BFILE_LARGEFILE
		    {
		        struct ustat	lsb ;
	                rs = u_fstat(fp->fd,&lsb) ;
	    		memset(ssp,0,sizeof(struct ustat)) ;
	    		ssp->st_dev = (dev_t) lsb.st_dev ;
	    		ssp->st_ino = (ino_t) lsb.st_ino ;
	    		ssp->st_mode = (mode_t) lsb.st_mode ;
	    		ssp->st_nlink = (int) lsb.st_nlink ;
	    		ssp->st_uid = (int) lsb.st_uid ;
	    		ssp->st_gid = (int) lsb.st_gid ;
	    		ssp->st_rdev = (dev_t) lsb.st_rdev ;
	    		ssp->st_size = (int) lsb.st_size ;
	    		ssp->st_atime = (time_t) lsb.st_atime ;
	    		ssp->st_mtime = (time_t) lsb.st_mtime ;
	    		ssp->st_ctime = (time_t) lsb.st_ctime ;
	    		ssp->st_blksize = (blksize_t) lsb.st_blksize ;
	    		ssp->st_blocks = (blkcnt_t) lsb.st_blocks ;
	    		memcpy(ssp->st_fstype,lsb.st_fstype,_ST_FSTYPSZ) ;
		    }
#else
	            rs = u_fstat(fp->fd,ssp) ;
#endif /* BFILE_LARGEFILE */
	        } else
		    rs = SR_FAULT ;
	    }
	    break ;

	case BC_TRUNCATE:
	    {
	        offset_t toff = (offset_t) va_arg(ap,offset_t) ;
	        rs = uc_ftruncate(fp->fd,toff) ;
	    }
	    break ;

	case BC_CHMOD:
	    {
		mode_t fm = (mode_t) va_arg(ap,mode_t) ;
	        rs = u_fchmod(fp->fd,fm) ;
	    }
	    break ;

	case BC_MINMOD:
	    {
		mode_t fm = (mode_t) va_arg(ap,mode_t) ;
	        rs = uc_fminmod(fp->fd,fm) ;
	    }
	    break ;

/* file record locking functions */
	case BC_UNLOCK:
	case BC_LOCKREAD:
	case BC_LOCKWRITE:
	    fsp = &fl ;
	    switch (cmd) {

	    case BC_LOCKREAD:
	        fsp->l_type = F_RDLCK ;
	        f_timedcmd = TRUE ;
	        break ;

	    case BC_LOCKWRITE:
	        fsp->l_type = F_WRLCK ;
	        f_timedcmd = TRUE ;
	        break ;

	    case BC_UNLOCK:
	    default:
	        fsp->l_type = F_UNLCK ;
	        break ;

	    } /* end switch */

	    fsp->l_whence = SEEK_SET ;
	    fsp->l_start = 0L ;
	    fsp->l_len = 0L ;

/* fall through to next case */
/* FALLTHROUGH */
/* enter here for a lock (or unlock) of a requested area of the file */
	case BC_SETLK:
	case BC_SETLKW:
	    if (fp->f.write && (fp->len > 0))
	        rs = bfile_flush(fp) ;

#if	CF_DEBUGS
	    debugprintf("bcontrol: about to set file lock %d %d %ld %ld\n",
	        fsp->l_type,fsp->l_whence,fsp->l_start, fsp->l_len) ;
#endif

/* fall through to next case */

	case BC_GETLK:
	    switch (cmd) {

	    case BC_LOCKWRITE:

#if	CF_DEBUGS
	        debugprintf("bcontrol: WRITE lock set fcmd to F_SETLK\n") ;
#endif

	    case BC_LOCKREAD:
	    case BC_SETLK:
	        fcmd = F_SETLK ;
	        break ;

	    case BC_UNLOCK:
	    case BC_SETLKW:
	        fcmd = F_SETLKW ;
	        break ;

	    case BC_GETLK:
	    default:
	        fcmd = F_GETLK ;
	        break ;

	    } /* end switch */

	    {
		int	to = 0 ;
	        if (f_timedcmd) {
		    to = (int) va_arg(ap,int) ;
	            if (to < 0) {
	                to = INT_MAX ;
	                fcmd = F_SETLKW ;
	            }
	        }

#if	CF_DEBUGS
	    debugprintf("bcontrol: locking fcmd=%s(%u) f_timed=%u to=%d\n",
	        fcmdname(fcmd),fcmd,f_timedcmd,to) ;
#endif

	        rs = bcontrol_lock(fp,fsp,fcmd,f_timedcmd,to) ;
#if	CF_DEBUGS
	    debugprintf("bcontrol: bcontrol_lock() rs=%d\n",rs) ;
#endif

	    }
	    break ;

/* buffer input lines (in spite of what is read off of the FD) */
	case BC_LINEIN:
	    fp->f.linein = TRUE ;
	    break ;

	case BC_GETFL:
	    {
		int	oflags ;
		int *rip = (int *) va_arg(ap,int *) ;
	        rs = u_fcntl(fp->fd,F_GETFL,0) ;
	        oflags = rs ;
	        if (rip != NULL)
	            *rip = (rs >= 0) ? oflags : 0 ;
	    }
	    break ;

	case BC_SETFL:
	    {
		int *rip = (int *) va_arg(ap,int *) ;
		if (rip != NULL) {
		    int	v = *rip ;
	            rs = u_fcntl(fp->fd,F_SETFL,v) ;
	 	} else
		    rs = SR_FAULT ;
	    }
	    break ;

	case BC_GETFDFL:
	    {
		int *rip = (int *) va_arg(ap,int *) ;
		if (rip != NULL) {
		    int	v ;
	            rs = u_fcntl(fp->fd,F_GETFD,0) ;
		    v = rs ;
		    *rip = (rs >= 0) ? v : 0 ;
	 	} else
		    rs = SR_FAULT ;
	    }
	    break ;

	case BC_SETFDFL:
	    {
		int *rip = (int *) va_arg(ap,int *) ;
		if (rip != NULL) {
		    int v = *rip ;
	            rs = u_fcntl(fp->fd,F_SETFD,v) ;
		} else
		    rs = SR_FAULT ;
	    }
	    break ;

	case BC_CLOSEONEXEC:
	    {
		int v = (int) va_arg(ap,int) ;
	        rs = uc_closeonexec(fp->fd,v) ;
	    }
	    break ;

	case BC_BUFSIZE:
	    if ((rs = bfile_flush(fp)) >= 0) {
	 	int bsize = (int) va_arg(ap,int) ;
	        char	*bdata ;
		if (bsize <= 1024) {
		    if (fp->pagesize == 0) fp->pagesize = getpagesize() ;
		    bsize = fp->pagesize ;
		}
	        if ((rs = uc_valloc(bsize,&bdata)) >= 0) {
	            if (fp->bdata != NULL)
	                uc_free(fp->bdata) ;
	            fp->bsize = bsize ;
	            fp->bdata = bdata ;
		    fp->bbp = fp->bdata ;
	            fp->bp = fp->bdata ;
	        } /* end if (successful) */
	    } /* end if (was able to flush) */
	    break ;

	case BC_DSYNC:
	    if ((rs = bfile_flush(fp)) >= 0) {
	        rs = uc_fdatasync(fp->fd) ;
	    } /* end if (was able to flush) */
	    break ;

	case BC_SYNC:
	    if ((rs = bfile_flush(fp)) >= 0) {
	        rs = uc_fsync(fp->fd) ;
	    } /* end if (was able to flush) */
	    break ;

	case BC_ISLINEBUF:
	    rs = (fp->bm == bfile_bmline) ;
	    break ;

	case BC_ISTERMINAL:
	    rs = fp->f.terminal ;
	    break ;

	case BC_SETBUFWHOLE:
	    {
		int f = (int) va_arg(ap,int) ;
	        fp->bm = bfile_bmall ;
		if (f) fp->bm = bfile_bmwhole ;
	    }
	    break ;

	case BC_SETBUFLINE:
	    {
		int f = (int) va_arg(ap,int) ;
		fp->bm = (f) ? bfile_bmline : bfile_bmall ;
	    }
	    break ;

	case BC_SETBUFNONE:
	    {
		int f = (int) va_arg(ap,int) ;
	        fp->bm = bfile_bmall ;
		if (f) fp->bm = bfile_bmnone ;
	    }
	    break ;

	case BC_SETBUFDEF:
	    {
		int f = (int) va_arg(ap,int) ;
	        fp->bm = bfile_bmall ;
		if (f && fp->f.terminal) fp->bm = bfile_bmline ;
	    }
	    break ;

	case BC_GETBUFFLAGS:
	    {
		int *rip = (int *) va_arg(ap,int *) ;
		if (rip != NULL) {
		    int v = 0 ;
		    if (fp->f.linein) v |= BFILE_FLINEIN ;
		    if (fp->f.terminal) v |= BFILE_FTERMINAL ;
		    switch (fp->bm) {
		    case bfile_bmwhole:
		        v |= BFILE_FBUFWHOLE ;
			break ;
		    case bfile_bmline:
		        v |= BFILE_FBUFLINE ;
			break ;
		    case bfile_bmnone:
		        v |= BFILE_FBUFNONE ;
			break ;
		    } /* end switch */
		} else
		    rs = SR_FAULT ;
	    }
	    break ;

	case BC_SETBUFFLAGS:
	    {
		int v = (int) va_arg(ap,int) ;
		if (v & BFILE_FBUFNONE) {
		    fp->bm = bfile_bmwhole ;
		} else if (v & BFILE_FBUFLINE) {
		    fp->bm = bfile_bmline ;
		} else if (v & BFILE_FBUFWHOLE) {
		    fp->bm = bfile_bmwhole ;
		}
		if (v & BFILE_FLINEIN) fp->f.linein = TRUE ;
		if (v & BFILE_FBUFDEF) {
		    int	bm = bfile_bmall ;
		    if (fp->f.terminal) bm = bfile_bmline ;
		}
	    }
	    break ;

	case BC_NONBLOCK:
	    {
		int v = (int) va_arg(ap,int) ;
		int	f ;
		f = (v > 0) ;
		rs = uc_nonblock(fp->fd,f) ;
	    }
	    break ;

/* handle all other commands */
	default:
	    rs = SR_INVALID ;
	    break ;

	} /* end switch (handling different "cmd"s) */
	va_end(ap) ;

ret0:

#if	CF_DEBUGS
	debugprintf("bcontrol: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bcontrol) */


int bsize(bfile *fp)
{
	struct ustat	sb ;
	int	rs ;
	if ((rs = bcontrol(fp,BC_STAT,&sb)) >= 0)
	    rs = (sb.st_size & INT_MAX) ;
	return rs ;
}
/* end subroutine (bsize) */


int bstat(fp,sbp)
bfile		*fp ;
bfile_stat	*sbp ;
{
	return bcontrol(fp,BC_STAT,sbp) ;
}
/* end subroutine (bstat) */


/* private subroutines */


/* perform the lock operations on the file */
static int bcontrol_lock(fp,fsp,fcmd,f_timedcmd,to)
bfile		*fp ;
struct flock	*fsp ;
int		fcmd ;
int		f_timedcmd ;
int		to ;
{
	int	rs = SR_OK ;
	int	i ;


	if (to < 0)
	    to = INT_MAX ;

	for (i = 0 ; i < to ; i += 1) {

	    if ((i > 0) && f_timedcmd)
	        sleep(1) ;

	    rs = u_fcntl(fp->fd,fcmd,fsp) ;
	    if (rs < 0) {

	        switch (rs) {

	        case SR_ACCES:
	        case SR_AGAIN:
	            rs = SR_LOCKED ;
	            break ;

	        case SR_DEADLK:
	            rs = SR_DEADLOCK ;
	            break ;

	        default:
	            rs = SR_NOTOPEN ;
		    break ;

	        } /* end switch */

	    } /* end if */

	    if ((! f_timedcmd) || (rs != SR_LOCKED))
	        break ;

	} /* end for (looping-timing on fcntl) */

	return rs ;
}
/* end subroutine (bcontrol_lock) */


#if	CF_DEBUGS
static char	*fcmdname(v)
int	v ;
{
	int	i ;
	for (i = 0 ; fcmds[i].n != NULL ; i += 1) {
	    if (fcmds[i].v == v) break ;
	} /* end for */
	return (fcmds[i].n != NULL) ? fcmds[i].n : "u" ;
}
#endif /* CF_DEBUGS */



