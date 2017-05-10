/* handle */

/* handle this connection of messages */
/* last modified %G% version %I% */


#define	CF_DEBUG	1		/* run-time debug print-outs */


/* revision history:

	= 1986-07-10, David A­D­ Morano

	This program was originally written.


	= 1998-07-01, David A­D­ Morano

	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.


*/


/**************************************************************************

	This subroutine will handle a connection that may contain one
	or more messages in it to be processed.  Incoming messages are
	collected from the input in three phases.  The phases are in
	the format:

	Phase 0
	- version

	Phase 1
	- originating or transport host

	Phase 2
	- job ID
	- envelope "from"
	- message options
	- length of message
	- recipients (as many as there are, one per line)
	- blank line marking the end-of-header

	Phase 3
	- message part (length given by value above)

	Phases 1 through 3 can be repeated (as a unit) as necessary until
	the input is empty.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<pcsconf.h>
#include	<bfile.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"address.h"


/* local defines */

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getfiledirs(char *,const char *,const char *,char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern int	addressparse(), addressjoin() ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_edate(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int handle(pip,s,elp)
struct proginfo	*pip ;
int		s ;
vecstr		*elp ;
{
	bfile	infile, *ifp = &infile ;

	offset_t	fsize ;

	time_t	daytime = 0 ;

	int	rs = SR_OK ;
	int	len ;
	int	cl, sl, pn ;
	int	ofd, efd ;
	int	version ;
	int	magiclen ;
	int	f, f_seekable ;

	char	buf[BUFLEN + 1] ;
	char	tmpfnamebuf[MAXPATHLEN + 1] ;
	char	ofnamebuf[MAXPATHLEN + 1], efnamebuf[MAXPATHLEN + 1] ;
	char	th[MAXHOSTNAMELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: entered, s=%d\n",s) ;
#endif

	magiclen = strlen(FILEMAGIC) ;

	ofnamebuf[0] = '\0' ;
	efnamebuf[0] = '\0' ;

/* create a quick and dirty log ID */

	pip->pid = getpid() ;

	snddd(buf,BUFLEN, pip->ppid,pip->pid) ;

	logfile_setid(&pip->lh,buf) ;

/* make log entry */

	daytime = time(NULL) ;

	logfile_printf(&pip->lh,"%s request pid=%d\n",
	    timestr_logz(daytime,timebuf),pip->pid) ;

	if ((pip->workdname != NULL) && (pip->workdname[0] != '\0'))
		u_chdir(pip->workdname) ;

/* can we get a peername from this socket, if it IS a socket ? */

#ifdef	COMMENT
	if (isasocket(s)) {

	    char	inethost[MAXHOSTNAMELEN + 1] ;


	    rs = u_getpeername(s,....) ;


	    logfile_printf(&pip->lh,"connection=%s\n", inethost) ;

	} /* end if (internet connection) */
#endif /* COMMENT */

/* can we execute this service daemon ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: starting X check\n") ;
#endif

	f = (strchr(pip->prog_sendmail,'/') != NULL) ;

	if ((f && ((rs = u_access(pip->prog_sendmail,X_OK)) < 0)) ||
	    ((! f) && 
	    ((rs = getfiledirs(NULL,pip->prog_sendmail,"x",NULL)) <= 0))) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("handle: could not execute\n") ;
#endif

	    if (rs == 0) 
		rs = SR_NOEXIST ;

	    logfile_printf(&pip->lh,
		"cannot find or execute service daemon (%d)\n",rs) ;

	    return rs ;

	} /* end if (checking executability of SENDMAIL) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: past X check\n") ;
#endif


/* output file */

	mkpath2(tmpfnamebuf, pip->tmpdname, "rm_ofXXXXXXXXX") ;

	if ((rs = mktmpfile(ofnamebuf, 0600, tmpfnamebuf)) < 0) {

	    logfile_printf(&pip->lh,
	        "could not open an output file, rs=%d\n",rs) ;

	    goto bad1 ;
	}

/* error file */

	mkpath2(tmpfnamebuf, pip->tmpdname, "rm_efXXXXXXXXX") ;

	if ((rs = mktmpfile( efnamebuf, 0600, tmpfnamebuf)) < 0) {

	    logfile_printf(&pip->lh,
	        "could not open an error file, rs=%d\n",rs) ;

	    goto bad2 ;
	}

/* open them both */

	rs = u_open(ofnamebuf,O_FLAGS,0666) ;
	ofd = rs ;
	if (rs < 0)
	    goto bad3 ;

	rs = u_open(efnamebuf,O_FLAGS,0666) ;
	efd = rs ;
	if (rs < 0)
	    goto bad4 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: ofd=%d efd=%d\n",ofd,efd) ;
#endif

	if (ofnamebuf[0] != '\0') {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: unlinking output file\n") ;
#endif

	    if (u_unlink(ofnamebuf) >= 0)
	        ofnamebuf[0] = '\0' ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: checking error file\n") ;
#endif

	if (efnamebuf[0] != '\0') {

	    if (u_unlink(efnamebuf) >= 0)
	        efnamebuf[0] = '\0' ;

	}

/* open the input channel with a line coded interpreter */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: about to open input\n") ;
#endif

	th[0] = '\0' ;
	rs = bopen(ifp,(char *) s,"dr",0666) ;
	if (rs < 0)
		goto ret4 ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("handle: opened the input, rs=%d\n",rs) ;
#endif

	    fsize = -1 ;
	    if (f_seekable = (bseek(ifp,0L,SEEK_CUR) >= 0)) {

	        struct ustat	sb ;


	        bcontrol(ifp,BC_STAT,&sb) ;

	        fsize = sb.st_size ;

	    }

/* pop off the version */

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("handle: checking file magic and file version\n") ;
#endif

		version = -1 ;
	    if ((rs = breadline(ifp,buf,BUFLEN)) > 0) {

		len = rs ;
		if (strncmp(buf,FILEMAGIC,magiclen) != 0)
			goto badmagic ;

		if (buf[len - 1] == '\n') 
			len -= 1 ;

		buf[len] = '\0' ;
		cp = buf ;
		while (*cp && (! CHAR_ISWHITE(*cp))) 
			cp += 1 ;

	        if (cfdeci(cp,-1,&version) < 0)
	            version = -1 ;

	    } /* end if (getting version) */

		if (version > FILEVERSION)
			goto badversion ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: version=%d\n",version) ;
#endif


/* pop them off */

	    pn = 0 ;
	    while ((rs = breadline(ifp,buf,BUFLEN)) > 0) {

		len = rs ;

/* parse out what host this group came from */

	        cl = sfshrink(buf,len,&cp) ;

		if (cl > 0) {

	            cp[cl] = '\0' ;
	            if (strcmp(th,cp) != 0)
	                logfile_printf(&pip->lh, "host=%s\n",cp) ;

	            strwcpy(th,cp,cl) ;

	        } /* end if (transport host) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: about to 'process'\n") ;
#endif

	        rs = process(pip,
			version,ifp,elp,pn,th,f_seekable,fsize,ofd,efd) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: process() rs=%d\n",rs) ;
#endif

	        if (rs < 0) 
			break ;

	        pn += 1 ;

	    } /* end while (looping through messages) */

ret5:
	    bclose(ifp) ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("handle: done, rs=%d\n",rs) ;
#endif


done:
ret4:
	u_close(efd) ;

bad4:
ret3:
	u_close(ofd) ;

bad3:
ret2:
	if (efnamebuf[0] != '\0')
	    u_unlink(efnamebuf) ;

bad2:
ret1:
	if (ofnamebuf[0] != '\0')
	    u_unlink(ofnamebuf) ;

bad1:
ret0:
	return rs ;

/* bad stuff */
badmagic:
	                logfile_printf(&pip->lh, "bad magic=>%W<\n",
				buf,strnlen(buf,14)) ;

	goto ret5 ;

badversion:
	                logfile_printf(&pip->lh, "bad version=%d\n",
				version) ;

	goto ret5 ;

}
/* end subroutine (handle) */



