/* uc_open64 */

/* higher-level "open" */


#define	CF_DEBUGS	0


/******************************************************************************

	Filename formats:

	UNIX® somain sockets have the format:
		/filepath

	where:
		filepath

	is just a regular UNIX® file path to the socket file.

	All other protocols have the format:
		/proto/protofamily/protoname/host/service

	where:
		proto		constant name 'proto'
		protofamily	protocol family
					inet
					inet6
		protoname	protocol name
					tcp
					udp
		host		hostname of remote host to contact
		service		service within the specified 
					daytime


	Examples:

	/something/unix/domain/socket

	/proto/inet/tcp/rca/daytime

	/proto/inet/udp/rca/daytime

	/proto/inet6/udp/rca/daytime


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#undef	MAXLINKS
#define	MAXLINKS	20


/* external subroutines */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_open64(fname,oflags,perm)
const char	fname[] ;
int	oflags ;
int	perm ;
{
	int	rs ;
	int	c_links = 0 ;

	char	buf[MAXPATHLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

top:

#if	CF_DEBUGS
	debugprintf("uc_open: fname=%s\n",fname) ;
#endif

	if ((fname[0] == '/') &&
	    (strncmp(fname,"/proto/",7) == 0)) {

#if	CF_DEBUGS
	    debugprintf("uc_open: openproto() \n") ;
#endif

	    rs = uc_openproto(fname,oflags,0) ;

#if	CF_DEBUGS
	    debugprintf("uc_open: openproto() rs=%d\n",rs) ;
#endif

	} else {

	    rs = u_open(fname,oflags,perm) ;

#if	CF_DEBUGS
	    debugprintf("uc_open: u_open() rs=%d\n",rs) ;
#endif

	    if (rs == SR_OPNOTSUPP) {

	        struct stat64	sb ;

	        int	rs1 ;


	        rs1 = u_stat64(fname,&sb) ;

	        if ((rs1 >= 0) && S_ISSOCK(sb.st_mode)) {

	            rs = uc_opensocket(fname,oflags,0) ;

#if	CF_DEBUGS
	            debugprintf("uc_open: opensocket() rs=%d\n",rs) ;
#endif

	        }

	    } else if (rs == SR_NOENT) {

	        struct stat64	sb ;


	        rs = u_lstat64(fname,&sb) ;

#if	CF_DEBUGS
	        debugprintf("uc_open: u_lstat() rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && S_ISLNK(sb.st_mode)) {

	            rs = u_readlink(fname,buf,MAXPATHLEN) ;

	            if (rs >= 0) {

	                buf[rs] = '\0' ;
	                fname = (const char *) buf ;
	                c_links += 1 ;
	                if (c_links < MAXLINKS)
	                    goto top ;

	                rs = SR_MLINK ;
	            }
	        }

	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("uc_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_open64) */


