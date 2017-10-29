/* notify */

/* give a user the notice */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1999-01-10, David Morano

	This subroutine was originally written.  It was prompted by the
	failure of other terminal message programs when they stopped
	working on Solaris® 8!


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will find the latest accessed terminal for
	the given user and then send the specified notification to
	that terminal.

	Synopsis:

	int notify(pip,tp,un,mbuf,mlen,up,ul)
	struct proginfo	*pip ;
	TMPX		*tp ;
	int		un ;
	char		mbuf[] ;
	int		mlen ;
	const char	up[] ;
	int		ul ;

	Arguments:

	- pip		program information
	- tp		TMPX pointer
	- un 		user number (internal)
	- mbuf		user buffer to receive name of controlling terminal
	- mlen		length of user supplied buffer
	- up		username to notify
	- ul		username legnth

	Returns:

	>=	length of name of controlling terminal
	<0	error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<tmpx.h>
#include	<vecstr.h>
#include	<sbuf.h>
#include	<ascii.h>
#include	<getxusername.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	USERNAMELEN
#define	USERNAMELEN	100
#endif

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#define	O_TERM		(O_WRONLY | O_NOCTTY | O_NDELAY)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	tmpx_getuserterms(TMPX *,VECSTR *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* local structures */


/* forward references */

static int noteone(struct proginfo *,const char *,const char *,int) ;
static int writenotice(struct proginfo *,int,const char *,int) ;


/* local variables */

static const char	eol[] = "\033[K\r\n" ;


/* exported subroutines */


int notify(pip,txp,un,mbuf,mlen,up,ul)
struct proginfo	*pip ;
TMPX		*txp ;
int		un ;
const char	mbuf[] ;
int		mlen ;
const char	up[] ;
int		ul ;
{
	VECSTR	uterms ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	numterms ;
	int	cl ;
	int	n = 0 ;

	const char	*tp ;
	const char	*cp ;

	char	username[USERNAMELEN + 1] ;
	char	termfname[MAXPATHLEN+1] ;


	if (mbuf == NULL) return SR_FAULT ;
	if (userspec == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("notify: ent us=%t\n",up,ul) ;
	    debugprintf("notify: mlen=%d mbuf=%t\n",
	        mlen,mbuf,strlinelen(mbuf,mlen,45)) ;
	}
#endif

	if (userspec[0] == '\0') return SR_INVALID ;

/* parse the username specification into the real username and count */

	if ((tp = strnchr(up,ul,'=')) != NULL) {

	    cl = sfshrink(up,(tp-up),&cp) ;

	    rs1 = cfdeci((tp+1),-1,&numterms) ;
	    if (rs1 < 0)
	        numterms = 0 ;

	} else {

	    numterms = pip->numterms ;
	    cl = sfshrink(userspec,-1,&cp) ;

	} /* end if */
	if (numterms < 0) numterms = 0 ;

	username[0] = '\0' ;
	if (cl == 0)
	    return SR_INVALID ;

	if (cp[0] == '-') {
	    rs = getusername(username,USERNAMELEN,-1) ;
	} else
	    strwcpy(username,cp,MIN(cl,USERNAMELEN)) ;

	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("notify: username=%s numterms=%d\n",username,numterms) ;
#endif

/* get the terminals that this user is logged in on */

	if ((rs = vecstr_start(&uterms,10,0)) >= 0) {

/* get the terminals for this user, if any */

	    if ((rs = tmpx_getuserterms(txp,&uterms,username)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("notify: tmpx_getuserterms() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            for (i = 0 ; vecstr_get(&uterms,i,&cp) >= 0 ; i += 1) {
	                if (cp != NULL)
	                debugprintf("notify: line=%s\n",cp) ;
	            }
	        }
#endif /* CF_DEBUG */

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"recip=%s",username) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("notify: top-of-loop\n") ;
#endif

	        n = 0 ;
	        for (i = 0 ; vecstr_get(&uterms,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("notify: termdev=%s\n",cp) ;
#endif

	                rs = noteone(pip,cp,mbuf,mlen) ;
			n += ((rs > 0) ? 1 : 0) ;

	                if (pip->open.logprog)
	                    logfile_printf(&pip->lh,"  line=%s (%d)",cp,rs) ;

	                if ((numterms > 0) && (n >= numterms))
	                    break ;

	            if (rs < 0) break ;
	        } /* end for (writing them) */

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"  sent=%u",n) ;

	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("notify: A tmpx_getuserterms() rs=%d\n",rs) ;
#endif

	    vecstr_finish(&uterms) ;
	} /* end if (vecstr) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("notify: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (notify) */


/* local subroutines */


static int noteone(pip,termfname,mbuf,mlen)
struct proginfo	*pip ;
const char	termfname[] ;
const char	mbuf[] ;
int		mlen ;
{
	struct ustat	sb ;

	const int	oflags = (O_WRONLY | O_NOCTTY) ;

	int	rs = SR_OK ;
	int	n = 0 ;


	if ((rs = uc_open(termfname,oflags,0666)) >= 0) {
	    const int	fd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("notify: got one termdev=%s fd=%d\n",
	            termfname,fd) ;
#endif

	    if ((rs = u_fstat(fd,&sb)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("notify: IXUSR=\\o%08o\n",
	                (sb.st_mode & S_IXUSR)) ;
#endif

	        if ((! pip->f.biffonly) || (sb.st_mode & S_IXUSR)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("notify: writenotice()\n") ;
#endif

	            if ((rs = writenotice(pip,fd,mbuf,mlen)) >= 0) {

	                n += 1 ;
	                if ((pip->verboselevel >= 2) && pip->f.outfile) {
	                    bprintf(pip->ofp,"%s\n",termfname) ;
	                }

	            } /* end if (successful write) */

	        } /* end if (go-ahead) */

	    } /* end if (stat) */

	    u_close(fd) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (noteone) */


static int writenotice(pip,fd_termdev,tbuf,tlen)
struct proginfo	*pip ;
int		fd_termdev ;
const char	tbuf[] ;
int		tlen ;
{
	SBUF	out ;

	int	rs ;
	int	rs1 ;
	int	sl, cl ;
	int	wlen = 0 ;

	const char	*tp, *sp, *cp ;

	char	buf[BUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("writenotice: fd_termdev=%d tlen=%d \n",
	        fd_termdev,tlen) ;
	    debugprintf("writenotice: tlen=%d tbuf=%t\n",
	        tlen,tbuf,strlinelen(tbuf,tlen,45)) ;
	}
#endif

	if ((rs = sbuf_start(&out,buf,BUFLEN)) >= 0) {

/* form the notice to write out */

	    if (pip->f.ringbell)
	        sbuf_char(&out,CH_BELL) ;

	    sbuf_char(&out,'\r') ;

	    sp = tbuf ;
	    sl = tlen ;
	    while ((tp = strnchr(sp,sl,'\n')) != NULL) {

	        cp = sp ;
	        cl = (tp - sp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("writenotice: line=>%t<\n",cp,cl) ;
#endif

	        sbuf_strw(&out,cp,cl) ;

	        rs = sbuf_strw(&out,(char *) eol,5) ;

	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;

	    } /* end while */

	    if ((rs >= 0) && (sl > 0)) {

	        cp = sp ;
	        cl = sl ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("writenotice: line=>%t<\n",cp,cl) ;
#endif

	        sbuf_strw(&out,cp,cl) ;

	        sbuf_strw(&out,(char *) eol,5) ;

	    } /* end if (some residue remaining) */

	    wlen = sbuf_getlen(&out) ;
	    if (rs >= 0) rs = wlen ;

/* write the notice out */

	    if ((rs >= 0) && (wlen > 0))
	        rs = u_write(fd_termdev,buf,wlen) ;

	    rs1 = sbuf_finish(&out) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (writenotice) */



