/* uc_readlinetimed */

/* read a line from a file descriptor but time it */


#define	F_DEBUGS	0


/* revision history :

	= 86/03/26, David A­D­ Morano

	This was first written to give a little bit to UNIX what we
	have in our own circuit pack OSes !  Note that this subroutine
	depends on another little one ('uc_read()') that is used to
	provide an underlying extended 'read(2)' like capability.


*/


/******************************************************************************

	Get a line code amount of data (data ending in an NL) and
	time it also so that we can abort if it times out.


	Synopsis :

	int uc_readlinetimed(fd,buf,buflen,timeout)
	int	fd ;
	char	buf[] ;
	int	buflen ;
	int	timeout ;


	Arguments :

	fd		file descriptor
	buf		user buffer to receive daa
	buflen		maximum amount of data the user wants
	timeout		time in seconds to wait


	Returns :

	<0		error
	>=0		amount of data returned



******************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>



/* external subroutines */

extern int	uc_reade(int,char *,int,int,int) ;






int uc_readlinetimed(fd,buf,buflen,timeout)
int	fd ;
char	buf[] ;
int	buflen ;
int	timeout ;
{
	int	rs ;
	int	len = 0 ;
	int	rlen = 0 ;


#if	F_DEBUGS
	eprintf("uc_readlinetimed: FD=%d buflen=%d\n",fd,buflen) ;
#endif

#ifdef	COMMENT
	if (timeout < 0)
	    timeout = 0x7FFFFFFF ;
#endif

#if	F_DEBUGS
	    if ((rs = u_fcntl(fd,F_GETFL,0)) < 0) {

		eprintf("uc_readlinetimed: FD not open ? rs=%d\n",rs) ;

		return rs ;
	}

	eprintf("uc_readlinetimed: flags=%08x\n",rs & O_ACCMODE) ;
#endif /* F_DEBUGS */

	while (rlen < buflen) {

	    rs = uc_reade(fd,buf + rlen,1,0,timeout) ;

	    if (rs <= 0)
		break ;

	    len = rs ;
	    rlen += len ;
	    if (buf[rlen - 1] == '\n') 
		break ;

	} /* end while */

#if	F_DEBUGS
	eprintf("uc_readlinetimed: after loop len=%d\n",len) ;
#endif

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (uc_readlinetimed) */



