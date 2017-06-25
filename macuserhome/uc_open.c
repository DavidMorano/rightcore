/* uc_open */

/* interface component for UNIX® library-3c */
/* higher-level "open" /w timeout */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Filename formats:

	UNIX® domain sockets have the format:
		filepath

	where:
		filepath

	is just a regular UNIX® file path to the socket file.

	File-systems that are supported internally (no external
	shared-memory object needed) are:

	proto
	prog
	pass
	shm
	u

	Protocols (when using the 'proto' filesystem above) have the format:

		/proto/<protoname>/<af>/<host>/<service>

	where:
		proto		constant name 'proto'
		<protoname>	protocol name
					tcp
					tcpmux[:port]
					tcpnls[:port]
					udp
					uss
					ussmux[:svc]
					usd
		<af>		address family
					inet
					inet4
					inet6
		<host>		hostname of remote host to contact
		<service>	service to contact


	Examples:

	/something/unix/domain/socket

	/proto/tcp/inet/rca/daytime
	/proto/udp/inet/rca/daytime
	/proto/udp/inet6/rca/daytime
	/proto/uss/unix/path
	/proto/usd/unix/path

	/proto/inet/tcp/rca/daytime
	/proto/inet/udp/rca/daytime
	/proto/inet6/udp/rca/daytime


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	OURSTAT		struct ustat
#define	FUNCSTAT	u_stat
#define	FUNCFSTAT	u_fstat
#define	FUNCLSTAT	u_lstat
#define	FUNCOPEN	u_open

#ifndef	MAXSYMLINKS
#define	MAXSYMLINKS	20		/* defined by the OS */
#endif

#define	NPOLLS		2
#define	POLLMULT	1000

#undef	CH_EXPAND
#define	CH_EXPAND	'%'

#define	UCOPENINFO	struct ucopeninfo


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	mkvarpath(char *,const char *,int) ;
extern int	haslc(const char *,int) ;
extern int	hasvarpathprefix(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_open(cchar *fname,int oflags,mode_t operms)
{
	int		opts = 0 ;

#if	CF_DEBUGS
	debugprintf("uc_open: fname=%s\n",fname) ;
#endif

	return uc_openex(fname,oflags,operms,-1,opts) ;
}
/* end subroutine (uc_open) */


int uc_opene(cchar *fname,int oflags,mode_t operms,int timeout)
{
	int		opts = 0 ;

#if	CF_DEBUGS
	debugprintf("uc_opene: fname=%s to=%d\n",fname,timeout) ;
#endif

	return uc_openex(fname,oflags,operms,timeout,opts) ;
}
/* end subroutine (uc_opene) */


int uc_openenv(cchar *fname,int oflags,mode_t operms,cchar **ev,int timeout)
{
	UCOPENINFO	oi ;
	int		rs ;
	int		opts = 0 ;

	memset(&oi,0,sizeof(struct ucopeninfo)) ;
	oi.fname = fname ;
	oi.oflags = oflags ;
	oi.operms = operms ;
	oi.to = timeout ;
	oi.opts = opts ;
	oi.envv = ev ;

	rs = uc_openinfo(&oi) ;

	return rs ;
}
/* end subroutine (uc_openenv) */


