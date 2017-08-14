/* forward */

/* queue the message to be forwarded */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* run-time debugging */


/* revision history:

	= 1994-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will forward the articles to other system.

	Synopsis:

	int forward(pip,ap,afname,host)
	struct proginfo	*pip ;
	struct article	*ap ;
	char		afname[] ;
	char		host[] ;

	Arguments:

	pip		program information pointer
	ap		article pointer
	afname		?
	host		?

	Returns:

	<0		error
	>=0		OK


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<char.h>
#include	<msgheaders.h>
#include	<localmisc.h>

#include	"retpath.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#undef	LINEBUFLEN
#define	LINEBUFLEN	100

#ifndef	LINEOUTLEN
#define	LINEOUTLEN	68
#endif

#undef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + MAXHOSTNAMELEN + 100)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	bopencmd(bfile **,const char *) ;

extern int	msgheaders() ;
extern int	ng_parse() ;
extern int	matmsgstart(const char *,int) ;
extern int	matmsghead(const char *,int,char *,int *) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;


/* external variables */


/* forward references */

static int	bprintpath(bfile *,const char *,RETPATH *) ;


/* exported subroutines */


int forward(pip,ap,afname,host)
struct proginfo	*pip ;
struct article	*ap ;
char		afname[] ;
char		host[] ;
{
	struct ustat	sb ;

	bfile		pfile, *pfp = &pfile ;
	bfile		afile, *afp = &afile ;
	bfile		errfile, *efp = &errfile ;
	bfile		*fpa[3] ;

	pid_t	pid ;

	int	rs, rs2 ;
	int	len ;
	int	fd ;
	int	efd = 2 ;
	int	childstat ;
	int	f_bol, f_eol ;
	int	f_looking, f_path ;

	const char	*cp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	cmdbuf[BUFLEN + 3] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	errfname[MAXPATHLEN + 1] ;
	char	hname[LINEBUFLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: entered\n") ;
#endif

	if ((pip->prog_rslow == NULL) || (pip->prog_rslow[0] == '\0'))
		return SR_NOTFOUND ;

/* compose the command to execute */

	if ((sfbasename(pip->prog_rslow,-1,&cp) > 0) &&
	    (strcmp(cp,"rslow") == 0) && (pip->uucpnode != NULL) && 
		(strcmp(pip->uucpnode,pip->nodename) != 0)) {

	    bufprintf(cmdbuf,BUFLEN,"%s -Uk %s %s %s",
	        pip->prog_rslow,
	        pip->uucpnode,
	        host,SERVICE) ;

	} else
	    bufprintf(cmdbuf,BUFLEN,"%s %s %s",
	        pip->prog_rslow,host,SERVICE) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: program> %s\n",cmdbuf) ;
#endif

/* make it so the program writes its STDERR to a file */

	mkpath2(tmpfname,pip->tmpdname, "rbbpostXXXXXXX") ;

	rs = mktmpfile(errfname,0600,tmpfname) ;

	if (rs < 0)
	    return rs ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: errfname=%s\n",errfname) ;
#endif

	u_close(efd) ;

	rs = u_open(errfname,O_RDWR,0600) ;
	fd = rs ;

	u_unlink(errfname) ;

	if (rs < 0)
	    return rs ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: opened rs=%d\n",rs) ;
#endif

	if (fd != efd) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("forward: dupping EFD over to proper\n") ;
#endif

	    rs2 = u_dup2(fd,efd) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("forward: dupped rs2=%d\n",rs2) ;
#endif

	    u_close(fd) ;

	}


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: about to open program\n") ;
#endif

	fpa[0] = &pfile ;
	fpa[1] = NULL ;
	fpa[2] = NULL ;
	rs = bopencmd(fpa,cmdbuf) ;

	if (rs < 0) {

	    u_close(efd) ;

	    return rs ;
	}

	pid = (pid_t) rs ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: opened program pid=%d\n",pid) ;
#endif


	proglog_printf(pip,"tp=%s pid=%d aid=%s h=%s\n",
	    pip->prog_rslow,pid,ap->articleid,host) ;


	rs = bopen(afp,afname,"r",0666) ;

	if (rs < 0)
		goto badopen ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: opened article file\n") ;
#endif

	f_looking = TRUE ;
	f_path = FALSE ;
	f_bol = TRUE ;
	while ((rs = breadline(afp,linebuf,LINEBUFLEN)) > 0) {
	    len = rs ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("forward: | %t",linebuf,len) ;
#endif

	    f_eol = (linebuf[len - 1] == '\n') ;

/* watch out for the PATH header! */

	    if (f_bol) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("forward: BOL\n") ;
#endif

	        if (f_looking) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("forward: looking\n") ;
#endif

	            if (linebuf[0] == '\n') {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("forward: EOH\n") ;
#endif

	                f_path = FALSE ;
	                f_looking = FALSE ;
	                (void) bprintpath(pfp,pip->nodename,&ap->path) ;

	                bprintf(pfp,"\n") ;

	            } else if (matmsghead(linebuf,len,hname,NULL) &&
	                (strcasecmp(hname,HN_PATH) == 0)) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("forward: got a PATH\n") ;
#endif

	                f_path = TRUE ;
	                f_looking = FALSE ;
	                (void) bprintpath(pfp,pip->nodename,&ap->path) ;

	            } else {

	                rs = bwrite(pfp,linebuf,len) ;

	            }

	        }  else if (f_path) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("forward: inside PATH\n") ;
#endif

	            if (! CHAR_ISWHITE(linebuf[0])) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("forward: no white space\n") ;
#endif

	                f_path = FALSE ;
	                rs = bwrite(pfp,linebuf,len) ;

	            }

	        } else {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("forward: 1 writing> %w",linebuf,len) ;
#endif

	            rs = bwrite(pfp,linebuf,len) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("forward: 1 bwrite rs=%d\n",rs) ;
#endif

	        }

	    } else {

	        if (! f_path) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("forward: 2 writing> %w",linebuf,len) ;
#endif

	            rs = bwrite(pfp,linebuf,len) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("forward: 2 bwrite rs=%d\n",rs) ;
#endif

	        }

	    }

	    f_bol = f_eol ;
	} /* end while (writing article file out) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: closing program input\n") ;
#endif

	bclose(pfp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: about to close article file\n") ;
#endif

	bclose(afp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: about to wait for child\n") ;
#endif

/* wait for the transport command to come back */

	rs = u_waitpid(pid,&childstat,0) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("forward: back from wait rs=%d\n",rs) ;
#endif

	if ((u_fstat(efd,&sb) >= 0) && (sb.st_size > 0) &&
	    (bopen(efp,(char *) efd,"rd",0666) >= 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("forward: got some STDERR size=%d\n",
			sb.st_size) ;
#endif

	    proglog_printf(pip,"forwarding error aid=%s es=%d\n",
	        ap->articleid,childstat) ;

	    bseek(efp,0L,SEEK_SET) ;

	    while ((len = breadline(efp,linebuf,LINEBUFLEN)) > 0) {
	        proglog_printf(pip," | %w",linebuf,len) ;
	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("forward: done loggin STDERR \n") ;
#endif

	} /* end if (errors from spawned program) */

	bclose(efp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("forward: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* could not open the article file */
badopen:
	bclose(pfp) ;

	u_close(efd) ;

	return rs ;
}
/* end subroutine (forward) */


/* local subroutines */


static int bprintpath(pfp,nodename,plp)
bfile		*pfp ;
const char	nodename[] ;
RETPATH		*plp ;
{
	int	rs = SR_OK ;
	int	i ;
	int	cl, ll ;
	int	bl = 0 ;

	const char	*cp ;


	ll = 0 ;
	if (rs >= 0) {
	    rs = bprintf(pfp,"%s: %s",HN_PATH,nodename) ;
	    bl += rs ;
	    ll += rs ;
	}

	if (rs >= 0) {
	for (i = 0 ; retpath_get(plp,i,&cp) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;

	    cl = strlen(cp) ;

	    if ((ll + cl + 1) >= LINEOUTLEN) {

		rs = bwrite(pfp,"\n\t",2) ;
		bl += rs ;

		ll = 1 ;

	    } /* end if */

	    if (rs >= 0) {
	        rs = bprintf(pfp,"!%s",cp) ;
	        bl += rs ;
	        ll += rs ;
	    }

	    if (rs < 0) break ;
	} /* end for */
	} /* end if */

	if (rs >= 0) {
	   rs = bprintf(pfp,"\n") ;
	   bl += rs ;
	}

ret0:
	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (bprintpath) */



