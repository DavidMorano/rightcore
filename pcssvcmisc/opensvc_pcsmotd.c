/* opensvc_pcsmotd */

/* LOCAL facility open-service (MOTD) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_pcsmotd(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getxusername.h>
#include	<motd.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_pcsmotd.h"
#include	"defs.h"


/* local defines */

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#ifndef	VARDOMAIN
#define	VARDOMAIN	"DOMAIN"
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#define	MOTDADMIN_PCS	"pcs"

#define	NDF		"opensvc_pcsmotd.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	openshmtmp(char *,int,mode_t) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opensvc_pcsmotd(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	MOTD_ID		mid ;
	gid_t		gid = getgid() ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		ll = -1 ;
	int		cols = COLUMNS ;

	const char	*varprlocal = VARPRLOCAL ;
	const char	*prlocal = NULL ;
	const char	*un = NULL ;
	const char	*groupname = NULL ;
	const char	*admin = NULL ;
	const char	*admins[3] ;

	char		ubuf[USERNAMELEN+1] ;
	char		gbuf[GROUPNAMELEN+1] ;
	char		prdname[MAXPATHLEN+1] ;

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: ent pr=%s\n",pr) ;
	nprintf(NDF,"opensvc_pcsmotd: prn=%s\n",prn) ;
#endif

/* find the number of allowable columns (for line-length limiting) */

#ifdef	COMMENT
	{
	    const char	*varcols = getourenv(envv,VARCOLUMNS) ;
	    if (varcols != NULL) {
	        int	rs1 ;
	        int	v ;
	        rs1 = cfdeci(varcols,-1,&v) ;
	        if (rs1 >= 0) cols = v ;
	    }
	}
#endif /* COMMENT */

/* find any administrator name given to us */

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc >= 2) && (argv[1] != '\0')) {
		admin = argv[1] ;
	    }
	}

/* default user as necessary */

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: 0 admin=%s\n",admin) ;
#endif

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = "-" ;
	if (rs < 0) goto ret0 ;

/* get the program-root for LOCAL */

	{
	    const char	*dnp = getourenv(envv,VARDOMAIN) ;
	    char	nn[NODENAMELEN+1] ;
	    char	dn[MAXHOSTNAMELEN+1] ;

	    if ((dnp == NULL) || (dnp[0] == '\0')) { /* user doesn't have */
		dnp = dn ;
		rs = getnodedomain(nn,dn) ;
	    }
	    if (rs >= 0) {
		prlocal = prdname ;
	        rs = mkpr(prdname,MAXHOSTNAMELEN,varprlocal,dnp) ;
	    }
	}
	if (rs < 0) goto ret0 ;

/* what administrators do we want? */

	if ((admin == NULL) || (admin[0] == '\0')) admin = MOTDADMIN_PCS ;
	{
	    int	i = 0 ;
	    admins[i++] = admin ;
	    admins[i] = NULL ;
	}

/* get our identification */

	if (groupname == NULL) {
	    rs = getgroupname(gbuf,GROUPNAMELEN,gid) ;
	    groupname = gbuf ;
	}
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: mid2 un=%s\n",un) ;
	nprintf(NDF,"opensvc_pcsmotd: mid2 gn=%s\n",groupname) ;
#endif

/* fill in our identification for MOTD processing */

	memset(&mid,0,sizeof(MOTD_ID)) ;
	mid.username = un ;
	mid.groupname = groupname ;
	mid.uid = getuid() ;
	mid.gid = gid ;

/* write it out */

	if ((rs = openshmtmp(NULL,0,0660)) >= 0) {
	    MOTD	m ;
	    fd = rs ;

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: motd_open()\n") ;
#endif

	    if ((rs = motd_open(&m,prlocal)) >= 0) {

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: mid4 rs=%d\n",rs) ;
#endif

	        rs = motd_processid(&m,&mid,admins,fd) ;

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: motd_processid() rs=%d\n",rs) ;
#endif

		rs1 = motd_close(&m) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (motd) */

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: motd-out rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
		u_rewind(fd) ;
	    } else {
	        u_close(fd) ;
	    }
	} /* end if (tmp-file) */

ret0:

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsmotd: ret rs=%d fd=%d\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_pcsmotd) */


/* local subroutines */


