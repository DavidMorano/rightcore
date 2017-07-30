/* getprogpath */

/* get the path to a program that is used within the PCS system */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to find the full pathname to a file.

	Synopsis:

	int getprogpath(idp,plp,rbuf,pp,pl)
	IDS		*idp ;
	vecstr		*plp ;
	char		rbuf[] ;
	const char	pp[] ;
	int		pl ;

	Arguments:

	idp		pointer to credential IDs
	plp		pointer to path list (vecstr)
	rbuf		returned file path if not the same as input
	pp		program to find
	pl		length of 'pp' string

	Returns:

	>0		found the program path and this is the length
	==0		program was found w/o a path prefix
	<0		program was not found


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#define	NDF	"getprogpath.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	xfile(IDS *,const char *) ;
extern int	getpwd(char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugprintf(const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getprogpath(IDS *idp,vecstr *plp,char *rbuf,cchar *pnp,int pnl)
{
	int		rs = SR_OK ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("getprogpath: ent name=%t\n",pnp,pnl) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"getprogpath: ent name=%t\n",pnp,pnl) ;
#endif

	if ((idp == NULL) || (plp == NULL)) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (pnp == NULL) return SR_FAULT ;

	if ((pnp[0] == '\0') || (pnl == 0)) return SR_INVALID ;

	if (pnl < 0) pnl = strlen(pnp) ;

	while ((pnl > 0) && (pnp[pnl-1] == '/')) {
	    pnl -=1 ;
	}

	rbuf[0] = '\0' ;
	if (strnchr(pnp,pnl,'/') == NULL) {
	    int		rs1 ;
	    int		i ;
	    int		f = FALSE ;
	    const char	*pp ;
	    char	pwd[MAXPATHLEN + 1] = { 0 } ;

	    for (i = 0 ; vecstr_get(plp,i,&pp) >= 0 ; i += 1) {
	        if (pp != NULL) {

#if	CF_DEBUGS
		debugprintf("getprogpath: p=>%s<\n",pp) ;
#endif
#if	CF_DEBUGN
		nprintf(NDF,"getprogpath: p=>%s<\n",pp) ;
#endif

	        if (pp[0] == '\0') {
	            if (pwd[0] == '\0') rs = getpwd(pwd,MAXPATHLEN) ;
		    if (rs >= 0) {
	                rs1 = mkpath2w(rbuf,pwd,pnp,pnl) ;
	        	rl = rs1 ;
		    }
	        } else {
	            rs1 = mkpath2w(rbuf,pp,pnp,pnl) ;
	            rl = rs1 ;
		}

#if	CF_DEBUGS
	debugprintf("getprogpath: mid rs=%d\n",rs) ;
	debugprintf("getprogpath: rbuf=%s\n",rbuf) ;
#endif

	        if ((rs >= 0) && (rs1 > 0)) {

	            if ((rs = xfile(idp,rbuf)) >= 0) {
			f = TRUE ;
		    } else if (isNotPresent(rs)) {
			rl = 0 ;
			rs = SR_OK ;
		    }

	        } /* end if */

		}
		if (f) break ;
		if (rs < 0) break ;
	    } /* end for */

	    if ((rs >= 0) && (!f)) rs = SR_NOENT ;

	} else {

	    if ((rs = mkpath1w(rbuf,pnp,pnl)) >= 0) {
		rl = rs ;
	        rs = xfile(idp,rbuf) ;
	    }

	} /* end if */

	if (rs < 0)
	    rbuf[0] = '\0' ;

#if	CF_DEBUGN
	nprintf(NDF,"getprogpath: ret rs=%d rl=%d\n",rs,rl) ;
	nprintf(NDF,"getprogpath: ret rbuf=%s\n",rbuf) ;
#endif

#if	CF_DEBUGS
	debugprintf("getprogpath: ret rs=%d rl=%d\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (getprogpath) */


