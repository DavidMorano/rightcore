/* ns */


#ifndef	NS_INCLUDE
#define	NS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vecobj.h>
#include	<vecstr.h>



#define	NS_ARGS		struct ns_args
#define	NS		struct ns_head



struct ns_args {
	char	*pr ;
	int	timeout ;		/* connection timeout */
	int	options ;
} ;

struct ns_head {
	unsigned long	magic ;
	void	*addr ;			/* remote address */
	void	*con ;			/* connection handle */
	int	itype ;
	int	fd ;
	int	addrlen ;
} ;



#if	(! defined(NS_MASTER)) || (NS_MASTER == 0)

extern int	ns_open(NS *,NS_ARGS *,
			const char *,const char *) ;
extern int	ns_read(NS *,char *,int,int) ;
extern int	ns_write(NS *,const char *,int) ;
extern int	ns_poll(NS *,int) ;
extern int	ns_close(NS *) ;
extern int	ns_localname(NS *,char *,int) ;
extern int	ns_peername(NS *,char *,int) ;

#endif /* NS_MASTER */

#endif /* NS_INCLUDE */


