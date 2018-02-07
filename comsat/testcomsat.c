/* testcomsat */

/* test seding COMAST messages */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        This code module was completely rewritten to replace the previous
        mail-delivery program for PCS, written around 1990 or so and which also
        served for a time to filter environment for the SENDMAIL daemon.

	= 2004-02-17, David A­D­ Morano
        This was modified to add the MSGID object. That is a database that
        stores message IDs. We used it to eliminate duplicate mail deliveries
        which as of late are coming from several popular sources!

*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This program tests sending (fake) COMSAT messages.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<syslog.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<sbuf.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	CSBUFLEN	MAX((2 * MAXHOSTNAMELEN),MSGBUFLEN)

#define	PROTONAME	"udp"
#define	LOCALHOST	"localhost"

#ifndef	PORTSPEC_COMSAT
#define	PORTSPEC_COMSAT		"biff"
#endif

#ifndef	IPPORT_BIFFUDP
#define	IPPORT_BIFFUDP	512
#endif


/* typedefs */

typedef struct sockaddr		sockaddr_t ;


/* external subroutines */

extern int	snscs(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	getourhe(cchar *,cchar *,struct hostent *,char *,int) ;
extern int	issamehostname(const char *,const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* local typedefs */


/* forward references */

static int	comsat(int,cchar **,int) ;
static int	mkcsmsg(char *,int,const char *,int,uint) ;


/* local variables */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	int		rs ;
	int		port ;
	int		ex = EX_OK ;

#if	CF_DEBUGS || CF_DEBUG
	{
	    const char	*vardf = "TESTCOMSAT_DEBUGFILE" ;
	    const char	*cp ;
	    if ((cp = getenv(vardf)) != NULL) {
	        debugopen(cp) ;
	        debugprintf("main: starting\n") ;
	    }
	}
#endif /* CF_DEBUGS */

/* get some information about COMSAT */

	if ((rs = getportnum(PROTONAME,PORTSPEC_COMSAT)) >= 0) {
	    port = rs ;
	} else if (isNotPresent(rs)) {
	    port = IPPORT_BIFFUDP ;
	    rs = SR_OK ;
	}

	if (rs >= 0) {
	    rs = comsat(argc,argv,port) ;
	}

#if	CF_DEBUGS
	debugprintf("main: ret\n") ;
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int comsat(int argc,cchar **argv,int port)
{
	SOCKADDRESS	sa ;
	const int	af = AF_INET ;
	const int	proto = IPPROTO_UDP ;
	int		rs ;
	int		n = 0 ;
	int		ul = -1 ;
	const char	*up = "pcs" ;

	if ((rs = u_socket(PF_INET,SOCK_DGRAM,proto)) >= 0) {
	    struct hostent	he, *hep = &he ;
	    const int		helen = getbufsize(getbufsize_he) ;
	    const int		fd = rs ;
	    char		*hebuf ;
	    if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
		int	ai ;
		for (ai = 1 ; argv[ai] != NULL ; ai += 1) {
	    	    cchar	*hn = argv[ai] ;
	            if ((rs = getourhe(hn,NULL,hep,hebuf,helen)) >= 0) {
	        	if (hep->h_addrtype == af) {
		    	    const void	*ha = (const void *) hep->h_addr ;
		    	    const int	ps = port ;
	            	    uint	off = 0 ;

#if	CF_DEBUGS
		    {
	                in_addr_t	ia ;
	                memcpy(&ia,hep->h_addr,sizeof(int)) ;
	                debugprintf("comsat: got INET address=\\x%08x\n",
	                    ntohl(ia)) ;
	            }
#endif /* CF_DEBUGS */

	            	    if ((rs = sockaddress_start(&sa,af,ha,ps,0)) >= 0) {
			        sockaddr_t	*sap = (sockaddr_t *) &sa ;
				const int	sal = rs ;
				const int	mlen = CSBUFLEN ;
				char		mbuf[CSBUFLEN+1] ;

	                        if ((rs = mkcsmsg(mbuf,mlen,up,ul,off)) > 1) {
				    const int	ml = rs ;
				    const int	sflags = 0 ;

	                            rs = u_sendto(fd,mbuf,ml,sflags,sap,sal) ;

#if	CF_DEBUGS
	                            debugprintf("comsat: u_sendto() rs=%d\n",
	                                rs) ;
#endif

	                            n += 1 ;

	                        } /* end if (mkcsmsg) */

	                        sockaddress_finish(&sa) ;
	                    } /* end if (sockaddress) */

	                } /* end if (got an host address) */
		    } /* end if (getourhe) */
	        } /* end for (looping through mail-hosts) */
	        uc_free(hebuf) ;
	    } /* end if (memory-allocation) */
	    u_close(fd) ;
	} /* end if (socket) */

	return rs ;
}
/* end subroutine (comsat) */


/* make (marshall) the COMSAT message itself */
static int mkcsmsg(char *rbuf,int rlen,cchar *up,int ul,uint val)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {

	    sbuf_strw(&b,up,ul) ;

	    sbuf_char(&b,'@') ;

	    sbuf_deci(&b,val) ;

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mkcsmsg) */


