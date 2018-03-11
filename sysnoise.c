/* sysnoise */

/* retrieve process ID related noise from the system */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_HRTIME	1		/* use high resolution time */
#define	CF_PROCFS	0		/* use 'proc(4)' */
#define	CF_DEVRANDOM	1		/* use '/dev/urandom' */
#define	CF_VMSTAT	0		/* use 'vmstat -s'? */

#if	defined(VMSTATNOISE) && (VMSTATNOISE > 0)
#undef	CF_VMSTAT
#define	CF_VMSTAT	1
#endif


/* revision history:

	= 2002-07-13, David A­D­ Morano
        I first made this up to get some addition noise from a UNIX® system that
        is fairly portable (a big problem).

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine acquires the process noise in the system. It basically
        just looks at the process IDs that exist at the moment of the call and
        returns a random sequence of data bases on that.

	Synopsis:

	int sysnoise(a,alen)
	uchar	a[] ;
	int	alen ;

	Arguments:

	a	buffer receive the noise
	alen	length (in bytes) of buffer supplied by caller

	Returns:

	>=0	bytes of noise returned
	<0	error


	Important Note:

        There is great danger in this subroutine! What if the sub-process starts
        up OK but then never writes anything into the pipe AND it also just
        hangs there indefinitely? Well, you might say "how can that happen?"
        "Even if it doesn't write anything, won't the sub-process eventually
        exit and thereby close its end of the pipe?" Well, my response, is, it
        has happened! Remember that we are trying to run 'vmstat -s'! Who knows
        what weirdo locks it has to acquire that might hang rather indefinitely?
        That is exactly what was observed, it was hanging for days on some
        system-lock of some sort!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<sys/time.h>		/* for |gethrtime()| */
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<fsdir.h>
#include	<sha1.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	PROCDNAME	"/proc"
#define	DEVFNAME	"/dev/null"
#define	DEVRANDOM	"/dev/urandom"

#define	NNENTS		1000
#define	DIGESTLEN	20

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	PROG_VMSTAT	"/usr/bin/vmstat"
#define	PROGNAME_VMSTAT	"vmstat"

#if	(! defined(SYSHAS_HRTIME)) || (SYSHAS_HRTIME == 0)
#undef	CF_HRTIME
#define	CF_HRTIME	0
#endif

#define	SWAPB(v)	(((v) << 8) | (((v) >> 8) & 255))

#define	RANDBUF		struct randbuf


/* external subroutines */

extern int	mkfdfname(char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecull(const char *,int,ULONG *) ;


/* local structures */

struct randbuf {
	ushort		*buf ;
	int		n, c, i, maxi ;
} ;


/* forward references */

static int	randbuf_start(RANDBUF *,ushort *,int) ;
static int	randbuf_len(RANDBUF *) ;
static int	randbuf_addui(RANDBUF *,uint) ;
static int	randbuf_addull(RANDBUF *,ULONG) ;
static int	randbuf_incr(RANDBUF *) ;
static int	randbuf_finish(RANDBUF *) ;


/* local variables */


/* exported subroutines */


int sysnoise(uchar *a,int alen)
{
	RANDBUF		rb ;
	int		rs, rs1 ;
	int		len = 0 ;
	ushort		buf[NNENTS + 1], *uswp ;

	if (a == NULL) return SR_FAULT ;

	if ((rs = randbuf_start(&rb,buf,NNENTS)) >= 0) {
	    TIMEVAL	tv ;
	    RANDBUF	rb ;
	    hrtime_t	hrt ;
	    ULONG	ulw ;
	    int		size ;
	    int		i ;

/* get system time for the first few entries */

	    uc_gettimeofday(&tv,NULL) ;

	    randbuf_addui(&rb,tv.tv_sec) ;

	    randbuf_addui(&rb,tv.tv_usec) ;

/* get high-resolution time if it is available */

#if	CF_HRTIME
	    hrt = gethrtime() ;

	    uswp = (ushort *) &hrt ;
	    for (i = 0 ; i < (sizeof(hrtime_t) / sizeof(ushort)) ; i += 1) {
	        randbuf_addui(&rb,uswp[i]) ;
	    }

	    hrt = gethrvtime() ;

	    uswp = (ushort *) &hrt ;
	    for (i = 0 ; i < (sizeof(hrtime_t) / sizeof(ushort)) ; i += 1) {
	        randbuf_addui(&rb,uswp[i]) ;
	    }
#endif /* CF_HRTIME */

/* get the process IDs */

#if	CF_PROCFS
	    {
	        fsdir		pdir ;
	        fsdir_ent	ds ;

	        if (fsdir_open(&pdir,PROCDNAME) >= 0) {
	            int		dlen ;
	            int		v ;
	            const char	*dnp ;

	            i = 0 ;
	            while ((dlen = fsdir_read(&pdir,&ds)) > 0) {

	                dnp = ds.name ;
	                if (*dnp == '.') {
	                    if (dnp[1] == '\0') continue ;
	                    if ((dnp[1] == '.') && (dnp[2] == '\0')) continue ;
	                }

	                if ((rs1 = cfdeci(dnp,dlen,&v)) >= 0) {
	                    if ((i++ & 1) == 1) v = SWAPB(v) ;
	                    randbuf_addui(&rb,v) ;
	                }

	            } /* end while (reading entries) */

	            fsdir_close(&pdir) ;
	        } /* end if (had a PROC directory) */

	    } /* end block */
#endif /* CF_PROCFS */

/* get some random butt from the system directly */

#if	CF_DEVRANDOM
	    if ((rs1 = u_open(DEVRANDOM,O_RDONLY,0666)) >= 0) {
	        int	fd = rs1 ;

	        if ((len = u_read(fd,&ulw,sizeof(ULONG))) > 0) {
	            randbuf_addull(&rb,ulw) ;
		}

	        u_close(fd) ;
	    } /* end if (had DEVRANDOM) */
#endif /* CF_DEVRANDOM */

/* get the VM statistics */

#if	CF_VMSTAT
	    {
	        bfile	outfile ;
	        pid_t	pid ;
	        int	pfds[2] ;

	        if ((rs = u_pipe(pfds)) >= 0) {

	            if ((rs = uc_fork()) == 0) {
	                int		afd ;
	                char		*argv[4] ;
	                const char	*devfname = DEVFNAME ;

	                u_close(pfds[0]) ;

	                afd = uc_moveup(pfds[1],3) ;

	                for (i = 0 ; i < 3 ; i += 1)
	                    u_close(i) ;

	                u_open(devfname,O_RDONLY,0666) ;

	                u_dup(afd) ;

	                u_open(devfname,O_WRONLY,0666) ;

	                u_close(afd) ;

	                argv[0] = PROGNAME_VMSTAT ;
	                argv[1] = "-s" ;
	                argv[2] = NULL ;

	                u_execvp(PROG_VMSTAT,argv) ;

	                uc_exit(EX_NOEXEC) ;
	            } else if (rs > 0) {
	                mode_t		om = 0666 ;
			const int	llen = LINEBUFLEN ;
	                int		childstat ;
	                int		cl ;
	                const char	*cp ;
			char		fbuf[MAXNAMELEN+1] ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                pid = (pid_t) rs ;
	                u_close(pfds[1]) ;

#if	CF_DEBUGS
	                debugprintf("sysnoise: child pid=%d c=%d\n",pid,rb.c) ;
#endif

			mkfdfname(fbuf,pfds[0]) ;
	                if ((rs = bopen(&outfile,fbuf,"r",om)) >= 0) {

	                    u_close(pfds[0]) ;
	                    pfds[0] = -1 ;

	                    while (rs >= 0) {
	                        rs = breadline(&outfile,lbuf,llen) ;
	                        len = rs ;
	                        if (rs <= 0) break ;

	                        if (lbuf[len - 1] == '\n') len -= 1 ;
				lbuf[len] = '\0' ;

#if	CF_DEBUGS
	                        debugprintf("sysnoise: line=>%t<\n",
	                            lbuf,MIN(len,50)) ;
#endif

	                        if ((cl = nextfield(lbuf,len,&cp)) > 0) {

	                            rs = cfdecull(cp,cl,&ulw) ;

#if	CF_DEBUGS
	                            debugprintf("sysnoise: vmstat "
	                                "rs=%d v=%016llx\n",
	                                rs,ulw) ;
#endif

	                            if (rs >= 0)
	                                randbuf_addull(&rb,ulw) ;

#if	CF_DEBUGS
	                            debugprintf("sysnoise: c=%d maxi=%d\n",
	                                rb.c,rb.maxi) ;
#endif

	                        } /* end if (got a field) */

	                    } /* end while (reading lines) */

	                    bclose(&outfile) ;
	                } /* end if (output available) */

	                u_waitpid(pid,&childstat,0) ;
	            } /* end if (parent) */

	            if (pfds[0] >= 0) {
	                u_close(pfds[0]) ;
	            }

	        } /* end if (opened pipes) */

	    } /* end block (VM statitics) */
#endif /* CF_VMSTAT */

/* now mix everything up */

	    if (rs >= 0) {
	        if ((rs = randbuf_len(&rb)) >= 0) {
	            SHA1	d ;
		    int		m = rs ;

	            if ((rs = sha1_start(&d)) >= 0) {

	                size = m * sizeof(ushort) ;
	                sha1_update(&d,(char *) buf,size) ;

	                if (alen >= DIGESTLEN) {
	                    sha1_digest(&d,(uchar *) a) ;
	                    len = DIGESTLEN ;
	                } else {
	                    uchar	aa[DIGESTLEN + 1] ;
	                    sha1_digest(&d,aa) ;
	                    memcpy(a,aa,alen) ;
	                    len = alen ;
	                } /* end if */

	                sha1_finish(&d) ;
	            } /* end if (sha1) */

	        } /* end if (randbuf) */
	    } /* end if (ok) */

	    randbuf_finish(&rb) ;
	} /* end if (randbuf) */

#if	CF_DEBUGS
	debugprintf("sysnoise: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs > 0) ? len : rs ;
}
/* end subroutine (sysnoise) */


/* local subroutines */


static int randbuf_start(RANDBUF *op,ushort buf[],int n)
{

	memset(op,0,sizeof(RANDBUF)) ;
	op->buf = buf ;
	op->n = n ;
	return 0 ;
}
/* end subroutine (randbuf_start) */


static int randbuf_len(RANDBUF *op)
{

	return op->maxi ;
}
/* end subroutine (randbuf_len) */


static int randbuf_addui(RANDBUF *op,uint uv)
{

	op->buf[op->i] ^= uv ;
	randbuf_incr(op) ;

	return op->c ;
}
/* end subroutine (randbuf_addui) */


static int randbuf_addull(RANDBUF *op,ULONG ulw)
{
	int		i ;
	ushort		usw ;

	for (i = 0 ; i < 4 ; i += 1) {
	    usw = (ushort) (ulw & 0xFFFF) ;
	    ulw = ulw >> 16 ;
	    op->buf[op->i] ^= usw ;
	    randbuf_incr(op) ;
	} /* end for */

	return op->c ;
}
/* end subroutine (randbuf_addull) */


static int randbuf_finish(RANDBUF *op)
{
	int		rs ;
	int		size ;

	rs = op->c ;

	size = op->maxi * sizeof(ushort) ;
	memset(op->buf,0,size) ;

#ifdef	COMMENT
	memset(op,0,sizeof(RANDBUF)) ;
#endif

	return rs ;
}
/* end subroutine (randbuf_finish) */


static int randbuf_incr(RANDBUF *op)
{

	op->i += 1 ;
	if (op->i > op->maxi)
	    op->maxi = op->i ;

	if (op->i >= op->n)
	    op->i = 0 ;

	op->c += 1 ;
	return op->i ;
}
/* end subroutine (randbuf_incr) */


