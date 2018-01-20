/* progcmd */

/* handle IPC-related things */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was adopted from the DWD program. I may not have changed
        all of the comments correctly though!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines handle some IPC related functions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<listenspec.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#if	CF_DEBUGS || CF_DEBUG
#include	<debug.h>
#endif

#include	"config.h"
#include	"defs.h"
#include	"muximsg.h"


/* local defines */

#ifndef	IPCDIRMODE
#define	IPCDIRMODE	0777
#endif

#define	IPCBUFLEN	MSGBUFLEN

#define	NIOVECS		1

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	isBadSend(int) ;
extern int	isBadRecv(int) ;
extern int	isBadMsg(int) ;

extern int	progjobdir(PROGINFO *,char *) ;
extern int	progreqfile(PROGINFO *) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	progcmder(PROGINFO *,ARGINFO *,bfile *,
			int,SOCKADDRESS *,int) ;
static int	progcmder_status(PROGINFO *,ARGINFO *,bfile *,
			int,SOCKADDRESS *,int) ;
static int	progcmder_listeners(PROGINFO *,ARGINFO *,bfile *,
			int,SOCKADDRESS *,int) ;
static int	progcmder_mark(PROGINFO *,ARGINFO *,bfile *,
			int,SOCKADDRESS *,int) ;
static int	progcmder_help(PROGINFO *,ARGINFO *,bfile *,
			int,SOCKADDRESS *,int) ;


/* local variables */

static cchar	*cmds[] = {
	"help",
	"status",
	"listeners",
	"mark",
	NULL
} ;

enum cmds {
	cmd_help,
	cmd_status,
	cmd_listeners,
	cmd_mark,
	cmd_overlast
} ;


/* exported subroutines */


int progcmd(PROGINFO *pip,ARGINFO *aip)
{
	int		rs ;
	int		rs1 ;

	if ((rs = keyopt_count(&pip->cmds)) > 0) {
	    bfile	ofile, *ofp = &ofile ;
	    cchar	*fn = pip->outfname ;

	    if ((fn == NULL) || (fn[0] == '\0'))
	        fn = STDOUTFNAME ;

	    if ((rs = bopen(ofp,fn,"wct",0666)) >= 0) {

/* create (or divine) the "request" filename as needed */

	        if ((rs = progreqfile(pip)) >= 0) {
	            char	ourdname[MAXPATHLEN + 1] ;
	            char	template[MAXPATHLEN + 1] ;
	            char	fname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progcmd/progcmd: req=%s\n",
	                    pip->reqfname) ;
#endif

/* ensure our directory for our socket */

	            rs = progjobdir(pip,ourdname) ;

	            if (rs >= 0)
	                rs = mkpath2(template,ourdname,"callXXXXXXXXXX") ;

	            if (rs >= 0) {
	                const mode_t	om = 0666 ;
	                const int	of = (O_RDWR | O_CREAT) ;

/* create our socket */

	                if ((rs = opentmpusd(template,of,om,fname)) >= 0) {
	                    SOCKADDRESS	sa ;
	                    const int	af = AF_UNIX ;
	                    int		fd = rs ;
	                    const char	*rf = pip->reqfname ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("progcmd/progcmd: srcaddr=%s\n",
	                            fname) ;
#endif

/* create the socket-address to the server request socket */

	                    if ((rs = sockaddress_start(&sa,af,rf,0,0)) >= 0) {
	                        const int	sal = rs ;

	                        rs = progcmder(pip,aip,ofp,fd,&sa,sal) ;

	                        sockaddress_finish(&sa) ;
	                    } /* end if (sockaddress) */

	                    u_close(fd) ;
	                } /* end if (file) */

	                if (fname[0] != '\0') {
	                    u_unlink(fname) ;
	                    fname[0] = '\0' ;
	                }

	            } /* end if (ok) */

	        } /* end if (progreqfile) */

	        rs1 = bclose(ofp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (file) */

	} /* end if (positive) */

	return rs ;
}
/* end subroutine (progcmd) */


int progcmdname(PROGINFO *pip,int idx,cchar **rpp)
{
	const int	nidx = nelements(cmds) ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if ((idx >= 0) && (idx < nidx)) {
	    len = (cmds[idx] != NULL) ? strlen(cmds[idx]) : 0 ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? cmds[idx] : NULL ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progcmdname) */


/* local subroutines */


static int progcmder(pip,aip,ofp,fd,sap,sal)
PROGINFO	*pip ;
ARGINFO		*aip ;
bfile		*ofp ;
int		fd ;
SOCKADDRESS	*sap ;
int		sal ;
{
	KEYOPT_CUR	kcur ;
	int		rs ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((rs = keyopt_curbegin(&pip->cmds,&kcur)) >= 0) {
	    int		ci ;
	    int		kl ;
	    cchar	*kp ;

	    while (rs >= 0) {
	        kl = keyopt_enumkeys(&pip->cmds,&kcur,&kp) ;
	        if (kl == SR_NOTFOUND) break ;
	        if (kl == 0) continue ;

	        rs = kl ;
	        if (rs >= 0) {
	            if ((ci = matostr(cmds,1,kp,kl)) >= 0) {
	                c += 1 ;
	                switch (ci) {
	                case cmd_help:
	                    rs = progcmder_help(pip,aip,ofp,fd,sap,sal) ;
	                    break ;
	                case cmd_status:
	                    rs = progcmder_status(pip,aip,ofp,fd,sap,sal) ;
	                    break ;
	                case cmd_listeners:
	                    rs = progcmder_listeners(pip,aip,ofp,fd,sap,sal) ;
	                    break ;
	                case cmd_mark:
	                    rs = progcmder_mark(pip,aip,ofp,fd,sap,sal) ;
	                    break ;
	                default:
			    fmt = "%s: unknown cmd=%t\n" ;
	                    bprintf(pip->efp,fmt,pn,kp,kl) ;
	                    break ;
	                } /* end switch */
	            } /* end if (valid) */
	        } /* end if (ok) */

	    } /* end while */

	    keyopt_curend(&pip->cmds,&kcur) ;
	} /* end if (keyopt-cur) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progcmd) */


static int progcmder_help(pip,aip,ofp,fd,sap,sal)
PROGINFO	*pip ;
ARGINFO		*aip ;
bfile		*ofp ;
int		fd ;
SOCKADDRESS	*sap ;
int		sal ;
{
	struct muximsg_gethelp	i13 ;
	struct muximsg_help	i14 ;
	struct msghdr	ipcmsg ;
	struct iovec	vecs[NIOVECS + 1] ;
	const int	to = TO_RECVMSG ;
	int		rs = SR_OK ;
	int		size ;
	int		mlen ;
	int		len ;
	int		i ;
	int		wlen = 0 ;
	int		f_alive = FALSE ;
	int		f_done = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		ipcbuf[IPCBUFLEN + 1] ;

/* some initialization (there is a good bit to setup for this operation) */

	size = NIOVECS * sizeof(struct iovec) ;
	memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;

	memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = sap ;
	ipcmsg.msg_namelen = sal ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;
	ipcmsg.msg_control = NULL ;
	ipcmsg.msg_controllen = 0 ;

/* formulate the content of our message */

	memset(&i13,0,sizeof(struct muximsg_gethelp)) ;

	for (i = 0 ; rs >= 0 ; i += 1) {
	    f_alive = FALSE ;

	    i13.idx = i ;
	    rs = muximsg_gethelp(&i13,0,ipcbuf,IPCBUFLEN) ;
	    mlen = rs ;
	    if (rs < 0) break ;

	    vecs[0].iov_len = mlen ;

/* send it */

	    if ((rs = u_sendmsg(fd,&ipcmsg,0)) >= 0) {

	        vecs[0].iov_len = IPCBUFLEN ;
	        ipcmsg.msg_control = NULL ;
	        ipcmsg.msg_controllen = 0 ;

	        if ((rs = uc_recvmsge(fd,&ipcmsg,0,to,0)) >= 0) {
	            len = rs ;
	            if (len > 0) {
	                if ((rs = muximsg_help(&i14,1,ipcbuf,len)) >= 0) {
			    uint	rc = i14.rc ;
	            	    if (rc == muximsgrc_ok) {
	                	if (i14.name[0] != '\0') {
				    cchar	*n = i14.name ;
			    	    f_alive = TRUE ;
				    fmt = "i=%u cmd=%s\n" ;
	                            rs = bprintf(ofp,fmt,i,n) ;
	                    	    wlen += rs ;
	                        }
			    } else if (rc == muximsgrc_notavail) {
				f_done = TRUE ;
			    } else {
			fmt = "server error (%u)\n" ;
	                rs = bprintf(ofp,fmt,rc) ;
			wlen += rs ;
			    }
		        } else if (isBadMsg(rs)) {
			fmt = "%s: protocol error (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
			    rs = SR_OK ;
			}
	            } else {
			fmt = "%s: protocol error (MSG)\n" ;
	                bprintf(pip->efp,fmt,pn) ;
		    }
	        } else if (isBadRecv(rs)) {
			fmt = "%s: protocol (RECV) error (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	            rs = SR_OK ;
		}
	    } else if (isBadSend(rs)) {
			fmt = "%s: server unresponsive (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
			fmt = "server unresponsive (%d)\n" ;
	                rs = bprintf(ofp,fmt,rs) ;
		wlen += rs ;
	    } /* end if */

	    if (f_done) break ;
	    if (! f_alive) break ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progcmder_help) */


static int progcmder_status(pip,aip,ofp,fd,sap,sal)
PROGINFO	*pip ;
ARGINFO		*aip ;
bfile		*ofp ;
int		fd ;
SOCKADDRESS	*sap ;
int		sal ;
{
	struct muximsg_response	i0 ;
	struct muximsg_noop	i1 ;
	struct msghdr	ipcmsg ;
	struct iovec	vecs[NIOVECS + 1] ;
	const int	to = TO_RECVMSG ;
	int		rs ;
	int		size ;
	int		len ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		ipcbuf[IPCBUFLEN + 1] ;

/* some initialization (there is a good bit to setup for this operation) */

	size = NIOVECS * sizeof(struct iovec) ;
	memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;

	memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = sap ;
	ipcmsg.msg_namelen = sal ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;
	ipcmsg.msg_control = NULL ;
	ipcmsg.msg_controllen = 0 ;

/* formulate the content of our message */

	memset(&i1,0,sizeof(struct muximsg_noop)) ;

	if ((rs = muximsg_noop(&i1,0,ipcbuf,IPCBUFLEN)) >= 0) {
	    int	mlen = rs ;

	    vecs[0].iov_len = mlen ;

/* send it */

	    if ((rs = u_sendmsg(fd,&ipcmsg,0)) >= 0) {

	        vecs[0].iov_len = IPCBUFLEN ;
	        ipcmsg.msg_control = NULL ;
	        ipcmsg.msg_controllen = 0 ;

	        if ((rs = uc_recvmsge(fd,&ipcmsg,0,to,0)) >= 0) {
	            len = rs ;
	            if (len > 0) {
	                if ((rs = muximsg_response(&i0,1,ipcbuf,len)) >= 0) {
	                    uint	pid = i0.pid ;
	                    uint	rc = i0.rc ;
			    fmt = "server rc=%d pid=%u\n" ;
			    bprintf(ofp,fmt,rc,pid) ;
			} else if (isBadMsg(rs)) {
			    fmt = "%s: protocol error (%d)\n" ;
	                    bprintf(pip->efp,fmt,pn,rs) ;
			    rs = SR_OK ;
			}
		    } else {
			fmt = "%s: protocol error (MSG)\n" ;
	                bprintf(pip->efp,fmt,pn) ;
	            }
	        } else if (isBadRecv(rs)) {
			fmt = "%s: protocol (RECV) error (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	            rs = SR_OK ;
		}

	    } else if (isBadSend(rs)) {
			fmt = "%s: server unresponsive (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
			fmt = "server unresponsive (%d)\n" ;
	                rs = bprintf(ofp,fmt,rs) ;
		wlen += rs ;
	    } /* end if */

	} /* end if (muximsg_noop) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progcmder_status) */


static int progcmder_listeners(pip,aip,ofp,fd,sap,sal)
PROGINFO	*pip ;
ARGINFO		*aip ;
bfile		*ofp ;
int		fd ;
SOCKADDRESS	*sap ;
int		sal ;
{
	struct muximsg_getlistener	i9 ;
	struct muximsg_listener		i10 ;
	struct msghdr	ipcmsg ;
	struct iovec	vecs[NIOVECS + 1] ;
	const int	to = TO_RECVMSG ;
	int		rs = SR_OK ;
	int		size ;
	int		mlen ;
	int		len ;
	int		i ;
	int		wlen = 0 ;
	int		f_alive = FALSE ;
	int		f_done = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		ipcbuf[IPCBUFLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progcmder_listeners: ent\n") ;
#endif

/* some initialization (there is a good bit to setup for this operation) */

	size = NIOVECS * sizeof(struct iovec) ;
	memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;

	memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = sap ;
	ipcmsg.msg_namelen = sal ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;
	ipcmsg.msg_control = NULL ;
	ipcmsg.msg_controllen = 0 ;

/* formulate the content of our message */

	memset(&i9,0,sizeof(struct muximsg_getlistener)) ;

	for (i = 0 ; rs >= 0 ; i += 1) {
	    f_alive = FALSE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progcmder_listeners: i=%d\n",i) ;
#endif

	    i9.idx = i ;
	    rs = muximsg_getlistener(&i9,0,ipcbuf,IPCBUFLEN) ;
	    mlen = rs ;
	    if (rs < 0) break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progcmder_listeners: muximsg_getlistener() "
	            "rs=%d ml=%u\n",
	            rs,i9.msglen) ;
#endif

	    vecs[0].iov_len = mlen ;

/* send it */

	    if ((rs = u_sendmsg(fd,&ipcmsg,0)) >= 0) {

	        vecs[0].iov_len = IPCBUFLEN ;
	        ipcmsg.msg_control = NULL ;
	        ipcmsg.msg_controllen = 0 ;

	        if ((rs = uc_recvmsge(fd,&ipcmsg,0,to,0)) >= 0) {
	            len = rs ;
	            if (len > 0) {
	                if ((rs = muximsg_listener(&i10,1,ipcbuf,len)) >= 0) {
			    uint	rc = i10.rc ;
	            	    if (rc == muximsgrc_ok) {
	                	if (i10.name[0] != '\0') {
	                    	const int	ls = i10.ls ;
	                    	const char	*sn ;
				f_alive = TRUE ;
	                    if (i10.ls & LISTENSPEC_MDELPEND) {
	                        sn = "D" ;
	                    } else if (i10.ls & LISTENSPEC_MBROKEN) {
	                        sn = "B" ;
	                    } else if (i10.ls & LISTENSPEC_MACTIVE) {
	                        sn = "A" ;
	                    } else if (ls == 0) {
	                        sn = "C" ;
	                    } else {
	                        sn = "U" ;
	                    }
	                    fmt = "i=%u %s type=%s addr=%s (%d)\n" ;
	                    rs = bprintf(ofp,fmt,i,sn,i10.name,i10.addr,ls) ;
	                    wlen += rs ;
			} else {
	                    fmt = "i=%u no-name\n" ;
	                    rs = bprintf(ofp,fmt,i) ;
	                    wlen += rs ;
	                } /* end if (name) */
			} else if (rc == muximsgrc_notavail) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progcmder_listeners: rc=%u not-avail \n",rc) ;
#endif
			    f_done = TRUE ;
			} else {
			    fmt = "%s: server error (%u)\n" ;
	                    bprintf(pip->efp,fmt,pn,rc) ;
			    fmt = "server error (%u)\n" ;
	                    rs = bprintf(ofp,fmt,rc) ;
			    wlen += rs ;
			}
			} else if (isBadMsg(rs)) {
			    fmt = "%s: protocol error (%d)\n" ;
	                    bprintf(pip->efp,fmt,pn,rs) ;
			    rs = SR_OK ;
			} /* end if */
		    } else {
			fmt = "%s: protocol error (MSG)\n" ;
	                bprintf(pip->efp,fmt,pn) ;
	            }
		} else if (isBadMsg(rs)) {
			fmt = "%s: protocol (RECV) error (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	            rs = SR_OK ;
		}
	    } else if (isBadSend(rs)) {
			fmt = "%s: server unresponsive (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
			fmt = "server unresponsive (%d)\n" ;
	                rs = bprintf(ofp,fmt,rs) ;
			wlen += rs ;
	    } /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progcmder_listeners: bot rs=%d f_alive=%u\n",
		rs,f_alive) ;
#endif

	    if (f_done) break ;
	    if (! f_alive) break ;
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progcmder_listeners: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progcmder_listeners) */


static int progcmder_mark(pip,aip,ofp,fd,sap,sal)
PROGINFO	*pip ;
ARGINFO		*aip ;
bfile		*ofp ;
int		fd ;
SOCKADDRESS	*sap ;
int		sal ;
{
	struct muximsg_response	i0 ;
	struct muximsg_mark	i11 ;
	struct msghdr	ipcmsg ;
	struct iovec	vecs[NIOVECS + 1] ;
	const int	to = TO_RECVMSG ;
	const int	ipclen = IPCBUFLEN ;
	int		rs ;
	int		size ;
	int		len ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		ipcbuf[IPCBUFLEN + 1] ;

/* some initialization (there is a good bit to setup for this operation) */

	size = NIOVECS * sizeof(struct iovec) ;
	memset(&vecs,0,size) ;

	vecs[0].iov_base = ipcbuf ;
	vecs[0].iov_len = IPCBUFLEN ;

	memset(&ipcmsg,0,sizeof(struct msghdr)) ;

	ipcmsg.msg_name = sap ;
	ipcmsg.msg_namelen = sal ;
	ipcmsg.msg_iov = vecs ;
	ipcmsg.msg_iovlen = NIOVECS ;
	ipcmsg.msg_control = NULL ;
	ipcmsg.msg_controllen = 0 ;

/* formulate the content of our message */

	memset(&i11,0,sizeof(struct muximsg_mark)) ;

	if ((rs = muximsg_mark(&i11,0,ipcbuf,ipclen)) >= 0) {
	    int	mlen = rs ;

	    vecs[0].iov_len = mlen ;

/* send it */

	    if ((rs = u_sendmsg(fd,&ipcmsg,0)) >= 0) {

	        vecs[0].iov_len = IPCBUFLEN ;
	        ipcmsg.msg_control = NULL ;
	        ipcmsg.msg_controllen = 0 ;

	        if ((rs = uc_recvmsge(fd,&ipcmsg,0,to,0)) >= 0) {
	            len = rs ;
	            if (len > 0) {
	                if ((rs = muximsg_response(&i0,1,ipcbuf,len)) >= 0) {
	            	    uint	pid = i0.pid ;
			    uint	rc = i0.rc ;
	            	    fmt = "server rc=%u pid=%u\n" ;
	                    bprintf(ofp,fmt,rc,pid) ;
			} else if (isBadMsg(rs)) {
			fmt = "%s: protocol error (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
			rs = SR_OK ;
			}
		    } else {
			fmt = "%s: protocol error (MSG)\n" ;
	                bprintf(pip->efp,fmt,pn) ;
		    }
	        } else if (isBadRecv(rs)) {
			fmt = "%s: protocol (RECV) error (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	            rs = SR_OK ;
		}
	    } else if (isBadSend(rs)) {
			fmt = "server unresponsive (%d)\n" ;
	                bprintf(ofp,fmt,pn,rs) ;
	        rs = SR_OK ;
	    } /* end if */

	} /* end if (muximsg_mark) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progcmder_mark) */


