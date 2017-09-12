/* progreport */

/* subroutines to report (pretty much) a fatal program of some sort */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 2012-0619, David A­D­ Morano
	This program was originally written.

	= 2017-09-11, David A­D­ Morano
        Taken out of existing code to try to make more useful for a wider range
        of programs.

*/

/* Copyright © 2012,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These are support subroutines for making a report (to a file) of some
	of the information related to how the calling program was invoked.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"progreport.h"


/* local defines */


/* external subroutines */

extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,void *,int) ;
extern int	hasprintbad(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthex(cchar *,int,cchar *,int) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* forward references */

static int	mkreportfile(PROGINFO *,char *,cchar *) ;
static int	mkreportout(PROGINFO *,cchar *,cchar *,int,cchar **,int) ;
static int	mktmpreportdir(char *,cchar *,cchar *,mode_t) ;


/* exported subroutines */


int mkreport(PROGINFO *pip,int argc,cchar **argv,int rv)
{
	const int	ulen = USERNAMELEN ;
	int		rs ;
	char		ubuf[USERNAMELEN+1] ;

	if (pip->daytime == 0) pip->daytime = time(NULL) ;

	if ((rs = getusername(ubuf,ulen,-1)) >= 0) {
	    const mode_t	dm = 0777 ;
	    cchar		*dname = pip->progname ;
	    cchar		*oun = pip->username ;
	    char		rbuf[MAXPATHLEN+1] ;
	    pip->username = ubuf ;
	    if ((rs = mktmpreportdir(rbuf,ubuf,dname,dm)) >= 0) {
	        char	fbuf[MAXPATHLEN+1] ;
	        if ((rs = mkreportfile(pip,fbuf,rbuf)) >= 0) {
	            const int	nlen = NODENAMELEN ;
	            char	nbuf[NODENAMELEN+1] ;
	            if (pip->pid == 0) pip->pid = getpid() ;
	            if ((rs = getnodename(nbuf,nlen)) >= 0) {
	                const int	llen = LOGIDLEN ;
	                const int	v = pip->pid ;
	                cchar		*onn = pip->nodename ;
	                char		lbuf[LOGIDLEN+1] ;
	                pip->nodename = nbuf ;
	                if ((rs = mklogid(lbuf,llen,nbuf,rs,v)) >= 0) {
	                    {
	                        rs = mkreportout(pip,fbuf,lbuf,argc,argv,rv) ;
	                    }
	                } /* end if (mklogid) */
	                pip->nodename = onn ;
	            } /* end if (getnodename) */
	        } /* end if (mkreportfile) */
	    } /* end if (mktmpuserdir) */
	    pip->username = oun ;
	} /* end if (getusername) */

	return rs ;
}
/* end subroutine (mkreport) */


/* local subroutines */


static int mkreportfile(PROGINFO *pip,char *fbuf,cchar *rbuf)
{
	TMTIME		mt ;
	const time_t	dt = pip->daytime ;
	int		rs ;

	if ((rs = tmtime_localtime(&mt,dt)) >= 0) {
	    const int	tlen = TIMEBUFLEN ;
	    cchar	*fmt = "r%y%m%d%H%M%S" ;
	    char	tbuf[TIMEBUFLEN+1] ;
	    if ((rs = sntmtime(tbuf,tlen,&mt,fmt)) >= 0) {
	        rs = mkpath2(fbuf,rbuf,tbuf) ;
	    } /* end if (sntmtime) */
	} /* end if (localtime) */

	return rs ;
}
/* end subroutine (mkreportfile) */


static int mkreportout(PROGINFO *pip,cchar *fbuf,cchar *id,
	int ac,cchar **av,int rv)
{
	bfile		rfile, *rfp = &rfile ;
	const time_t	dt = pip->daytime ;
	int		rs ;
	cchar		*fmt ;
	char		tbuf[TIMEBUFLEN+1] ;
	timestr_logz(dt,tbuf) ;
	if ((rs = bopen(rfp,fbuf,"wct",0666)) >= 0) {
	    const int	al = DISARGLEN ;
	    int		v = pip->pid ;
	    int		i ;

	    fmt = "%-15s %s failure report (%d)\n" ;
	    bprintf(rfp,fmt,id,tbuf,rv) ;

	    fmt = "%-15s node=%s\n" ;
	    bprintf(rfp,fmt,id,pip->nodename) ;

	    fmt = "%-15s user=%s\n" ;
	    bprintf(rfp,fmt,id,pip->username) ;

	    fmt = "%-15s pid=%u\n" ;
	    bprintf(rfp,fmt,id,v) ;

	    fmt = "%-15s argc=%u args¬\n" ;
	    bprintf(rfp,fmt,id,ac) ;

	    fmt = "%-15s a%02u=>%t<\n" ;
	    for (i = 0 ; (i < ac) && (av[i] != NULL) ; i += 1) {
	        cchar	*ap = av[i] ;
	        rs = bprintf(rfp,fmt,id,i,ap,al) ;
	        if (rs < 0) break ;
	    } /* end if */

	    fmt = "%-15s done\n" ;
	    bprintf(rfp,fmt,id) ;

	    bclose(rfp) ;
	} /* end if (file) */
	return rs ;
}
/* end subroutine (mkreportout) */


/* ARGSUSED */
static int mktmpreportdir(char *rbuf,cchar *ubuf,cchar *dname,mode_t m)
{
	cchar		*rdname = REPORTDNAME ;
	int		rs ;
	int		rl = 0 ;
	if ((rs = mkdirs(rdname,m)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = uc_stat(rdname,&sb)) >= 0) {
	        const mode_t	dm = (m|S_ISVTX) ;
		const uid_t	uid = getuid() ;
		const uid_t	u = sb.st_uid ;
		if (u == uid) {
		    if ((rs = uc_minmod(rdname,dm)) >= 0) {
			cchar	*adm = ADMINUSER ;
			if ((rs = getuid_user(adm,-1)) >= 0) {
			    const uid_t	uid_admin = rs ;
			    rs = uc_chown(rdname,uid_admin,-1) ;
			} else if (isNotPresent(rs)) {
			    rs = SR_OK ;
			}
		    }
		}
		if (rs >= 0) {
		    if ((rs = mkpath2(rbuf,rdname,dname)) >= 0) {
	    	        rl = rs ;
	    	        if ((rs = mkdirs(rbuf,m)) >= 0) {
	        	    rs = uc_minmod(rbuf,dm) ;
	    	        }
		    }
		}
	    } /* end if (stat) */
	} /* end if (mkdirs) */
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mktmpreportdir) */


