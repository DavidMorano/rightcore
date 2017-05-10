/*
 * timed  - Time server
 *
 *	Returns with time of day
 *	in seconds since Jan 1, 1900.  This is 
 *	86400(365*70 + 17) more than time
 *	since Jan 1, 1970, which is what get/settimeofday
 *	uses.  Called from inetd, so uses fd 0 as socket.
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>




#define	TOFFSET	((unsigned int) 86400* (unsigned int) (365*70 + 17))





int main()
{
	struct timeval timestruct ;

	unsigned int	unixtime ;
	unsigned int	nettime ;

	int	fromlen ;
	int	rs ;



	if (gettimeofday(&timestruct,NULL) == -1)
	    _exit(1) ;

	unixtime = timestruct.tv_sec + TOFFSET ;

	nettime = htonl(unixtime) ;

	if (write(0, (char *) &nettime, sizeof (int)) != sizeof (int)) {

/* maybe a UDP socket */

	    struct sockaddr from ;

	    struct fd_set	fds ;

	char	netbuf[40] ;


		FD_ZERO(&fds) ;

		FD_SET(0,&fds) ;

	    timestruct.tv_sec = timestruct.tv_usec = 0 ;
	    rs = select(2, &fds, NULL,NULL, &timestruct) ;

		if ((rs > 0) && FD_ISSET(1,&fds)) {

	        fromlen = sizeof(struct sockaddr) ;
	        if (recvfrom(0, netbuf, 40, 0,
	            &from, &fromlen) >= 0) {

	            sendto(0, (char *) &nettime, 
	                sizeof(int), 0, &from, fromlen) ;

	        }
	    }
	}

	if (close(0) == -1)
	    _exit(1) ;

}
/* end subroutine (main) */



