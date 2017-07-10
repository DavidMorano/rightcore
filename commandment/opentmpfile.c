/* opentmpfile */

/* make and open a temporary file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Make and open a temporary file.

	Synopsis:

	int opentmpfile(inname,of,om,obuf)
	const char	inname[] ;
	int		of ;
	mode_t		om ;
	char		obuf[] ;

	Arguments:

	inname		input file name template string
	of		open flags
	om		file type and creation mode
	obuf		output buffer to hold resultant file name

	Returns:

	>=0		file descriptor
	<0		failure returning the error number


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/time.h>		/* for |gethetime(3c)| */
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<cthex.h>
#include	<ugetpid.h>
#include	<sigblock.h>
#include	<sockaddress.h>
#include	<localmisc.h>


/* local defines */

#define	MAXLOOP		1000

#define	OTM_DGRAM	(1<<0)		/* open-type-mask DGRAM */
#define	OTM_STREAM	(1<<1)		/* open-type-mask STREAM */

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#undef	RANDBUFLEN
#define	RANDBUFLEN	(sizeof(ULONG)*2)


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkexpandpath(char *,cchar *,int) ;


/* external variables */


/* forward reference */

static int	opentmpx(const char *,int,mode_t,int,char *) ;
static int	opentmpxer(const char *,int,mode_t,int,char *) ;
static int	randload(ULONG *) ;
static int	mktmpname(char *,ULONG,cchar *) ;


/* local variables */


/* exported subroutines */


int opentmpfile(cchar *inname,int of,mode_t om,char *rbuf)
{
	const int	otm = OTM_STREAM ;
	return opentmpx(inname,of,om,otm,rbuf) ;
}
/* end subroutine (opentmpfile) */


int opentmpusd(cchar *inname,int of,mode_t om,char *rbuf)
{
	const int	otm = OTM_DGRAM ;
	om |= (S_IFSOCK | 0600) ;
	return opentmpx(inname,of,om,otm,rbuf) ;
}
/* end subroutine (opentmpusd) */


int opentmpuss(cchar *inname,int of,mode_t om,char *rbuf)
{
	const int	otm = OTM_STREAM ;
	om |= (S_IFSOCK | 0600) ;
	return opentmpx(inname,of,om,otm,rbuf) ;
}
/* end subroutine (opentmpuss) */


int opentmp(cchar *dname,int of,mode_t om)
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	const char	*template = "otXXXXXXXXXXXX" ;
	char		inname[MAXPATHLEN + 1] ;

	of &= (~ O_ACCMODE) ;
	of |= O_RDWR ;

	om |= 0600 ;
	if (dname == NULL) dname = getenv(VARTMPDNAME) ;
	if ((dname == NULL) || (dname[0] == '\0')) dname = TMPDNAME ;

	if ((rs = mkpath2(inname,dname,template)) >= 0) {
	    const int	olen = MAXPATHLEN ;
	    char	*obuf ;
	    if ((rs = uc_malloc((olen+1),&obuf)) >= 0) {
	        SIGBLOCK	blocker ;
	        if ((rs = sigblock_start(&blocker,NULL)) >= 0) {
		    const int	otm = OTM_STREAM ;

	            if ((rs = opentmpx(inname,of,om,otm,obuf)) >= 0) {
		        fd = rs ;
	                if (obuf[0] != '\0') uc_unlink(obuf) ;
	            }

	            rs1 = sigblock_finish(&blocker) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (sigblock) */
		rs1 = uc_free(obuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a) */
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (mkpath) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opentmp) */


/* local subroutines */


static int opentmpx(cchar *inname,int of,mode_t om,int opt,char *obuf)
{
	const int	plen = MAXPATHLEN ;
	int		rs ;
	int		fd = -1 ;
	char		*pbuf ;
	if ((rs = uc_malloc((plen+1),&pbuf)) >= 0) {
	    if ((rs = mkexpandpath(pbuf,inname,-1)) > 0) {
		rs = opentmpxer(pbuf,of,om,opt,obuf) ;
		fd = rs ;
	    } else if (rs == 0) {
		rs = opentmpxer(inname,of,om,opt,obuf) ;
		fd = rs ;
	    }
	    uc_free(pbuf) ;
	} /* end if (m-a) */
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opentmpx) */


static int opentmpxer(cchar *inname,int of,mode_t om,int opt,char *obuf)
{
	ULONG		rv ;
	const mode_t	operm = (om & S_IAMB) ;
	int		rs = SR_OK ;
	int		stype = 0 ;
	int		loop = 0 ;
	int		fd = 0 ;
	int		f_exit = FALSE ;
	int		f_abuf = FALSE ;
	int		f ;

#if	CF_DEBUGS
	debugprintf("opentmpx: ent\n") ;
#endif

	if (inname == NULL) return SR_FAULT ;
	if (obuf == NULL) return SR_FAULT ;

	if (inname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("opentmpx: infname=%s\n",inname) ;
	debugprintf("opentmpx: of=\\x%06X\n",of) ;
	debugprintf("opentmpx: operm=\\o%06o\n",operm) ;
#endif

	obuf[0] = '\0' ;

	if (S_ISSOCK(om)) {
	    if (opt & OTM_DGRAM) {
		stype = SOCK_DGRAM ;
	    } else if (opt & OTM_STREAM) {
		stype = SOCK_STREAM ;
	    } else
		rs = SR_INVALID ;
	}

	if ((rs >= 0) && (obuf == NULL)) {
	    const int	olen = MAXPATHLEN ;
	    if ((rs = uc_malloc((olen+1),&obuf)) >= 0) {
		obuf[0] = '\0' ;
		f_abuf = TRUE ;
	    }
	}

	if (rs >= 0) rs = randload(&rv) ;

/* loop trying to create a file */

	while ((rs >= 0) && (loop < MAXLOOP) && (! f_exit)) {

#if	CF_DEBUGS
	    debugprintf("opentmpx: top of while, loop=%d\n",loop) ;
#endif

	    f_exit = TRUE ;

/* put the file name together */

	    rv += loop ;
	    rs = mktmpname(obuf,rv,inname) ;
	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("opentmpx: onl=%d obuf=%s\n",rs,obuf) ;
#endif

/* go ahead and make the thing */

	    if (S_ISDIR(om)) {

	        rs = u_mkdir(obuf,operm) ;
		if (rs < 0) {
	            if ((rs == SR_EXIST) || (rs == SR_INTR))
	                f_exit = FALSE ;
	        } 

	        if (rs >= 0) {
	            rs = u_open(obuf,O_RDONLY,operm) ;
		    fd = rs ;
	        }

	    } else if (S_ISREG(om) || ((om & (~ S_IAMB)) == 0)) {
	        int	ooflags = (of | O_CREAT | O_EXCL) ;

#if	CF_DEBUGS
	        debugprintf("opentmpx: REG fname=%s\n",obuf) ;
#endif

		f = ((ooflags & O_WRONLY) || (ooflags & O_RDWR)) ;
		if (! f)
		    ooflags |= O_WRONLY ;

#if	CF_DEBUGS
	        debugprintf("opentmpx: ooflags=\\x%06X\n",ooflags) ;
	        debugprintf("opentmpx: operm=\\o%06o\n",operm) ;
#endif

	        rs = u_open(obuf,ooflags,operm) ;
	        fd = rs ;
		if (rs < 0) {
	            if ((rs == SR_EXIST) || (rs == SR_INTR)) f_exit = FALSE ;
	        }

#if	CF_DEBUGS
	        debugprintf("opentmpx: u_open() rs=%d\n",
	            rs) ;
#endif

	    } else if (S_ISFIFO(om)) {

#if	CF_DEBUGS
	        debugprintf("opentmpx: got a FIFO\n") ;
#endif

	        if ((rs = u_mknod(obuf,operm,0)) >= 0) {
	            rs = u_open(obuf,of,operm) ;
	            fd = rs ;
		} else {
	            if ((rs == SR_EXIST) || (rs == SR_INTR)) {
			f_exit = FALSE ;
		    }
	        }

	    } else if (S_ISSOCK(om)) {
	        const int	pf = PF_UNIX ;
	        if ((rs = u_socket(pf,stype,0)) >= 0) {
	            SOCKADDRESS	sa ;
		    const int	af = AF_UNIX ;
	            fd = rs ;
	            if ((rs = sockaddress_start(&sa,af,obuf,0,0)) >= 0) {
		        struct sockaddr	*sap = (struct sockaddr *) &sa ;
		        const int	sal = rs ;
	                if ((rs = u_bind(fd,sap,sal)) >= 0) {
			    rs = u_chmod(obuf,operm) ;
			    if (rs < 0) {
				u_unlink(obuf) ;
				obuf[0] = '\0' ;
			    }
			} /* end if (bind) */
	                sockaddress_finish(&sa) ;
		    } /* end if (sockaddress) */
		    if (rs < 0) {
			u_close(fd) ;
			fd = -1 ;
		    }
	        } /* end if (socket) */
		if (rs < 0) {
	            f_exit = FALSE ;
		}
	    } else {

#if	CF_DEBUGS
	        debugprintf("opentmpx: unknown mode=%08o\n",om) ;
#endif

		rs = SR_INVALID ;
	        f_exit = TRUE ;

	    } /* end if */

	    loop += 1 ;
	} /* end while */

	if ((rs >= 0) && (loop >= MAXLOOP)) {
	    rs = SR_ADDRINUSE ;
	}

	if (f_abuf) {
	    if (obuf[0] != '\0') {
		u_unlink(obuf) ;
	    }
	    uc_free(obuf) ;
	} else if (rs < 0) {
	    if (obuf != NULL) {
		obuf[0] = '\0' ;
 	    }
	}

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opentmpxer) */


static int randload(ULONG *rvp)
{
	const pid_t	pid = ugetpid() ;
	const pid_t	sid = getsid(0) ;
	const uid_t	uid = getuid() ;
	ULONG		rv = 0 ;
	ULONG		v ;

	if (rvp == NULL) return SR_FAULT ;

#ifdef	COMMENT
	v = gethostid() ;
	rv ^= (v << 32) ;
#endif

	v = sid ;
	rv += (v << 48) ;

	v = pid ;
	rv += (v << 32) ;

	v = uid ;
	rv += (v << 16) ;

	{
	    struct timeval	tod ;
	    uc_gettimeofday(&tod,NULL) ; /* cannot fail?! */
	        v = tod.tv_sec ;
	        rv += (v << 32) ;
	        rv += (v << 12) ;
	        rv += tod.tv_usec ;
	} /* end block */

#if	SYSHAS_HRTIME
	{
	    hrtime_t	ht = gethrtime() ;
	    rv += ht ;
	}
#endif /* SYSHAS_GRTIME */

	*rvp = rv ;

	return SR_OK ;
}
/* end subroutine (randload) */


static int mktmpname(char *obuf,ULONG rv,cchar *inname)
{
	const int	randlen = RANDBUFLEN ;
	int		rs ;
	char		randbuf[RANDBUFLEN+1] ;

/* load buffer w/ random HEX digits (16 bytes) from random variable (8 bytes) */

	if ((rs = cthexull(randbuf,randlen,rv)) >= 0) {
	    if ((rs = mkpath1(obuf,inname)) >= 0) {
	        int	j = rs ;
	        int	i = randlen ;
	        while (j > 0) {
		    j -= 1 ;
	            if (i > 0) {
		        if (obuf[j] == 'X') obuf[j] = randbuf[--i] ;
	            }
	        } /* end while */
	    } /* end if (mkpath) */
	} /* end if (cthexull) */

	return rs ;
}
/* end subroutine (mktmpname) */


