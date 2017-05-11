/* tmpmailboxes */

/* get the TMP-MAIL (mailboxes) directory name */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	FInd or create the TMP-MAIL (mailboxes) directory.

	Synopsis:

	int tmpmailboxes(char *rbuf,int rlen)

	Arguments:

	rbuf		buffer to hold result
	rlen		length of supplied result buffer

	Returns:

	>=0		length of result string (in result buffer)
	<0		some error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>


/* local defines */

#define	PRNAME		"PCS" /* Personal Communication Services */

#ifndef	VARTMPDNAME
#define	VARTMPDNAME 	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	TMPMAILBOXES
#define	TMPMAILBOXES	"/var/tmp/mailboxes"
#endif

#ifndef	PC_CHOWNRES
#define	PC_CHOWNRES	_PC_CHOWN_RESTRICTED
#endif


/* external subroutines */

extern int	sfsubstance(const char *,int,const char **) ;
extern int	matkeystr(const char **,char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getdomainname(char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	mkpr(char *,int,cchar *,cchar *) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	deftmpdir(char *,int) ;
static int	chownpcs(cchar *) ;


/* local variables */


/* exported subroutines */


int tmpmailboxes(char *rbuf,int rlen)
{
	USTAT		sb ;
	int		rs ;
	int		rl = 0 ;
	cchar		*tmpmb = TMPMAILBOXES ;

#if	CF_DEBUGS
	debugprintf("tmpmailboxes: ent\n") ;
#endif

	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if ((rs = uc_stat(tmpmb,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
		const int	am = (R_OK|W_OK|X_OK) ;
		if ((rs = perm(tmpmb,-1,-1,NULL,am)) >= 0) {
	            rs = sncpy1(rbuf,rlen,tmpmb) ;
		    rl = rs ;
		} else if (isNotAccess(rs)) {
	            rs = deftmpdir(rbuf,rlen) ;
		    rl = rs ;
		}
	    } else {
	        rs = deftmpdir(rbuf,rlen) ;
		rl = rs ;
	    }
	} else if (isNotPresent(rs)) {
	    const mode_t	dm = (0777|S_ISVTX) ;
	    if ((rs = uc_mkdir(tmpmb,dm)) >= 0) {
		if ((rs = uc_minmod(tmpmb,dm)) >= 0) {
		    const int	n = PC_CHOWNRES ; /* chown-restricted */
		    rl = strlen(tmpmb) ;
	            if ((rs = uc_pathconf(tmpmb,n,NULL)) == 0) {
			if ((rs = chownpcs(tmpmb)) >= 0) {
			    rs = SR_OK ;
			} else if (isNotPresent(rs)) {
			    rs = SR_OK ;
			}
		    }
		} else if (isNotAccess(rs)) {
	            rs = deftmpdir(rbuf,rlen) ;
	            rl = rs ;
		}
	    } else if (isNotPresent(rs)) {
	        rs = deftmpdir(rbuf,rlen) ;
	        rl = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("tmpmailboxes: ret rs=%d rl=%u\n",rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (tmpmailboxes) */


/* local subroutine */


static int deftmpdir(char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	cchar		*tmpdir = TMPDNAME ;
	cchar		*cp = getenv(VARTMPDNAME) ;
	if (cp != NULL) {
	    USTAT	sb ;
	    if ((rs = uc_stat(cp,&sb)) >= 0) {
		if (S_ISDIR(sb.st_mode)) {
		    const int	am = (R_OK|W_OK|X_OK) ;
		    if ((rs = perm(cp,-1,-1,NULL,am)) >= 0) {
	                rs = sncpy1(rbuf,rlen,cp) ;
		    } else if (isNotAccess(rs)) {
	                rs = sncpy1(rbuf,rlen,tmpdir) ;
		    }
		} else {
	            rs = sncpy1(rbuf,rlen,tmpdir) ;
		}
	    } else if (isNotPresent(rs)) {
	        rs = sncpy1(rbuf,rlen,tmpdir) ;
	    }
	} else {
	    rs = sncpy1(rbuf,rlen,tmpdir) ;
	}
	return rs ;
}
/* end subroutine (deftmpdir) */


static int chownpcs(cchar *dname)
{
	int		rs ;
	char		domain[MAXHOSTNAMELEN+1] ;
#if	CF_DEBUGS
	debugprintf("tmpmailboxes/chownpcs: ent\n") ;
#endif
	if ((rs = getnodedomain(NULL,domain)) >= 0) {
	    const int	prlen = MAXPATHLEN ;
	    cchar	*prname = PRNAME ;
	    char	prbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpr(prbuf,prlen,prname,dname)) >= 0) {
	 	USTAT	sb ;
		if ((rs = uc_stat(prbuf,&sb)) >= 0) {
		    {
		        const uid_t	uid_pcs = sb.st_uid ;
		        const gid_t	gid_pcs = sb.st_gid ;
		        rs = uc_chown(dname,uid_pcs,gid_pcs) ;
		    }
		} /* end if (uc_stat) */
	    } /* end if (mkpr) */
	} /* end if (getnodedomain) */
#if	CF_DEBUGS
	debugprintf("tmpmailboxes/chownpcs: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (chownpcs) */


