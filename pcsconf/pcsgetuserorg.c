/* pcsgetuserorg */

/* get the organization name (string) for a specified user-name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_ORGSYS	0		/* get from system? */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the organization name (string) for a
	specified user-name.

	Synopsis:

	int pcsgetuserorg(pr,rbuf,rlen,username)
	const char	*pr ;
	char		rbuf[] ;
	int		rlen ;
	const char	*username ;

	Arguments:

	pr		program-root
	rbuf		user supplied buffer to hold result
	rlen		length of user supplied buffer
	username	username to look up

	Returns:

	>=0		length of return organization string
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<filebuf.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<gecos.h>
#include	<localmisc.h>


/* local defines */

#define	SUBINFO		struct subinfo

#ifndef	ETCDNAME
#define	ETCDNAME	"/etc"
#endif

#undef	ORGCNAME
#define	ORGCNAME	"organization"

#ifndef	ORGLEN
#define	ORGLEN		MAXNAMELEN
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARORGANIZATION
#define	VARORGANIZATION	"ORGANIZATION"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	getuserorg(char *,int,const char *) ;
extern int	pcsgetorg(const char *,char *,int,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pcsgetuserorg(cchar *pr,char rbuf[],int rlen,cchar *un)
{
	int		rs ;

	if (pr == NULL) return SR_FAULT ;
	if ((rbuf == NULL) || (un == NULL)) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (pr[0] == '\0') return SR_INVALID ;
	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcsgetuserorg: u=%s\n",un) ;
#endif

	rs = getuserorg(rbuf,rlen,un) ;

	if (isNotPresent(rs) || (rs == 0)) {
	    rs = pcsgetorg(pr,rbuf,rlen,un) ;
	}

#if	CF_DEBUGS
	debugprintf("pcsgetuserorg: ret rs=%d\n",rs) ;
	debugprintf("pcsgetuserorg: org=>%s<\n",rbuf) ;
#endif

	return rs ;
}
/* end subroutine (pcsgetuserorg) */


