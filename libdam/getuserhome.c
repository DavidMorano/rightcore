/* getuserhome */

/* get the home directory of the specified user */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We carry out some searches in order to find the home directory for a
	specified user (given by username).

	Synopsis:

	int getuserhome(rbuf,rlen,username)
	char		rbuf[] ;
	int		rlen ;
	const char	username[] ;

	Arguments:

	rbuf		buffer to hold result
	rlen		length of supplied result buffer
	username	username to check

	Returns:

	>=0		OK
	<0		some error


	Implementation note:

	Is "home-searching" faster than just asking the system for the home
	directory of a user?  It probably is not!  In reality, it is a matter
	of interacting with the Automount server probably much more than what
	would have happened if we just first interacted with the
	Name-Server-Cache and then the Automount server once after that.

	Note that we NEVER 'stat(2)' a file directly within any of the possible
	base directories for home-directories without first having a pretty
	good idea that it is actually there.  This is done to avoid those
	stupid SYSLOG entries saying that a file could not be found within an
	indirect auto-mount point -- what the base directories usually are
	now-a-days!

	On second thought, avoiding asking the "system" may be a good course of
	action since even though we are going through the name-server cache, it
	more often than not has to go out to some weirdo-only-knows network NIS+
	(whatever) server to get the PASSWD information.  This is not unusual
	in a large organization.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<fsdir.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct subinfo_flags {
	uint		pw:1 ;
} ;

struct subinfo {
	const char	*un ;
	SUBINFO_FL	init ;
	struct passwd	pw ;
	uid_t		uid ;
	int		pwlen ;
	char		*pwbuf ;
} ;


/* forward references */

int		getuserhome(char *,int,const char *) ;

static int	subinfo_start(SUBINFO *,const char *) ;
static int	subinfo_getpw(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	subinfo_getvar(SUBINFO *,char *,int) ;
static int	subinfo_getdirsearch(SUBINFO *,char *,int) ;
static int	subinfo_getsysdb(SUBINFO *,char *,int) ;

static int	dirsearch(const char *,const char *) ;


/* local variables */

static const char	*homednames[] = {
	"/home",
	"/usr/add-on",
	"/sysadm",
	NULL
} ;

static int	(*gethomes[])(SUBINFO *,char *,int) = {
	subinfo_getvar,
	subinfo_getdirsearch,
	subinfo_getsysdb,
	NULL
} ;


/* exported subroutines */


int getuserhome(char *rbuf,int rlen,cchar *un)
{
	SUBINFO		si ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("getuserhome: ent un=%s\n",un) ;
#endif

	if (rbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;
	if ((rs = subinfo_start(&si,un)) >= 0) {
	    int	i ;
	    for (i = 0 ; gethomes[i] != NULL ; i += 1) {
	        rs = (*gethomes[i])(&si,rbuf,rlen) ;
	        rl = rs ;
		if (rs != 0) break ;
	    } /* end for */
#if	CF_DEBUGS
	    debugprintf("getuserhome: for-out rs=%d i=%u\n",rs,i) ;
#endif
	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

	if ((rs >= 0) && (rl == 0)) rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("getuserhome: ret rs=%d len=%u\n",rs,rl) ;
	if (rs >= 0)
	    debugprintf("getuserhome: home=>%t<\n",rbuf,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (getuserhome) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *un)
{
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->un = un ;
	sip->uid = -1 ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    sip->pwbuf = pwbuf ;
	    sip->pwlen = pwlen ;
	}

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip == NULL) return SR_FAULT ;

	if (sip->pwbuf != NULL) {
	    rs1 = uc_free(sip->pwbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->pwbuf = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_getpw(SUBINFO *sip)
{
	int		rs = SR_OK ;
	const char	*un = sip->un ;

#if	CF_DEBUGS
	debugprintf("getuserhome/subinfo_getpw: init.pw=%u\n",sip->init.pw) ;
	debugprintf("getuserhome/subinfo_getpw: un=%s\n",un) ;
#endif

	if (! sip->init.pw) {
	    const int	pwlen = sip->pwlen ;
	    sip->init.pw = TRUE ;
	    if (un[0] != '-') {
	        if (hasalldig(un,-1)) {
	            uint	uv ;
	            if ((rs = cfdecui(un,-1,&uv)) >= 0) {
	                const uid_t	uid = uv ;
	                rs = getpwusername(&sip->pw,sip->pwbuf,pwlen,uid) ;
	            }
	        } else {
	            rs = GETPW_NAME(&sip->pw,sip->pwbuf,pwlen,un) ;
	        }
	    } else {
	        rs = getpwusername(&sip->pw,sip->pwbuf,pwlen,-1) ;
	    }
	    if (rs >= 0) {
	        sip->uid = sip->pw.pw_uid ;
	    }
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (subinfo_getpw) */


static int subinfo_getvar(SUBINFO *sip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	const char	*un = sip->un ;

	if (un[0] == '-') un = getenv(VARUSERNAME) ;
	if ((un != NULL) && (un[0] != '\0')) {
	    const char	*hd = getenv(VARHOME) ;
	    if ((hd != NULL) && (hd[0] != '\0')) {
	        const char	*bp ;
	        int		bl ;
	        if ((bl = sfbasename(hd,-1,&bp)) > 0) {
	            if (strwcmp(un,bp,bl) == 0) {
	                rs = mknpath1(rbuf,rlen,hd) ;
	                len = rs ;
	            }
	        } /* end if (sfbasename) */
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_getvar) */


static int subinfo_getdirsearch(SUBINFO *sip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		len = 0 ;
	const char	*un = sip->un ;

	if (un[0] != '-') {
	    struct ustat	sb ;
	    int			i ;
	    const char		*hdn ;
	    for (i = 0 ; homednames[i] != NULL ; i += 1) {
	        hdn = homednames[i] ;
	        if ((rs = uc_stat(hdn,&sb)) >= 0) {
		    if (S_ISDIR(sb.st_mode)) {
	                if ((rs = dirsearch(hdn,un)) > 0) {
	                    if ((rs = mknpath2(rbuf,rlen,hdn,un)) >= 0) {
	                        if ((rs = uc_stat(rbuf,&sb)) >= 0) {
	                            if (S_ISDIR(sb.st_mode)) {
	                        	len = rs ;
				    }
				} else if (isNotPresent(rs)) {
				    rs = SR_OK ;
	                        }
	                    } /* end if (mkpath) */
		        } /* end if (dirsearch) */
		    } /* end if (isdir) */
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		} /* end if (stat) */
	        if ((rs < 0) || (len > 0)) break ;
	    } /* end for (looping over login-directory root directories) */
	} /* end if (specified) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_getdirsearch) */


static int subinfo_getsysdb(SUBINFO *sip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("getuserhome/gethome_sysdb: un=%s\n",sip->un) ;
#endif

	if (! sip->init.pw) {
	    rs = subinfo_getpw(sip) ;
	}

	if (rs >= 0) {
	    rs = mknpath1(rbuf,rlen,sip->pw.pw_dir) ;
	    len = rs ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_getsysdb) */


/* see if the user is in this directory */
static int dirsearch(cchar *basedname,cchar *un)
{
	FSDIR		dir ;
	FSDIR_ENT	ds ;
	int		rs ;
	int		rs1 ;
	int		f_found = FALSE ;

	if ((rs = fsdir_open(&dir,basedname)) >= 0) {
	    const char	*fnp ;
	    while ((rs = fsdir_read(&dir,&ds)) > 0) {
	        fnp = ds.name ;

	        if (fnp[0] != '.') {
	            f_found = (fnp[0] == un[0]) && (strcmp(fnp,un) == 0) ;
	            if (f_found) break ;
	        }

	    } /* end while */
	    rs1 = fsdir_close(&dir) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (dirsearch) */


