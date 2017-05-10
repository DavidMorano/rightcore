/* prognotify */

/* fairly generic (PCS) subroutine */


#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_ISSAMEHOST	1		/* use 'issamehostname(3dam)' */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

*/

/* Copyright © 1995 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine sends a message to hosts indicating that some mail has
        come in for a user.

	Implementation notes:

        Note that we use the same socket for all messages. This avoids creating
        new sockets for each one. But it also means that we only bother with one
        protocol family, and that family is PF_INET. This has not proved to be a
        big problem!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<vecobj.h>
#include	<paramfile.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mcmsg.h"
#include	"recip.h"


/* local defines */

#define	REPORTINFO	struct reportinfo

#define	BUFLEN		MAX((2 * MAXHOSTNAMELEN),MSGBUFLEN)
#define	HOSTBUFLEN	(10 * MAXHOSTNAMELEN)
#define	NSBUFLEN	(MAXHOSTNAMELEN + 32)

#define	PROTONAME	"udp"

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif

#ifndef	IPPORT_BIFFUDP
#define	IPPORT_BIFFUDP	512
#endif

#ifndef	IPPORT_MAILPOLL
#define	IPPORT_MAILPOLL	5110
#endif

#define	MSGSENDTRIES	5


/* external subroutines */

extern uint	hashelf(const void *,int) ;

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snscs(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getserial(const char *) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	getourhe(cchar *,cchar *,struct hostent *,char *,int) ;
extern int	issamehostname(const char *,const char *,const char *) ;
extern int	isNotPresent(int) ;

extern int	parsenodespec(PROGINFO *,char *,const char *,int) ;

extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct reportinfo {
	SOCKADDRESS	sa ;
	struct mcmsg_report	m1 ;
	uint		tag ;
	int		defport ;
	int		salen ;
	int		fd ;
	int		sendflags ;
	char		buf[MSGBUFLEN + 1] ;
} ;


/* local typedefs */

#if	defined(IRIX) && (! defined(TYPEDECF_INADDRT))
#define	TYPEDECF_INADDRT	1
typedef unsigned int	in_addr_t ;
#endif


/* forward references */

static int	prognotifyrecip(PROGINFO *,vecobj *,
			REPORTINFO *,PARAMFILE *,RECIP *) ;
static int	prognotifyrecipnode(PROGINFO *,vecobj *,
			REPORTINFO *,RECIP *,const char *,int) ;
static int	report(PROGINFO *,REPORTINFO *) ;

static int	searchfunc() ;


/* local variables */


/* exported subroutines */


int prognotify(PROGINFO *pip,vecobj *mip,vecobj *rsp)
{
	REPORTINFO	ri ;
	PARAMFILE	mbtab ;
	int		rs, rs1 ;
	int		defport ;
	int		cl ;
	int		c = 0 ;
	cchar		*proto = PROTONAME ;
	cchar		*svc = PORTSPEC_MAILPOLL ;
	cchar		*cluster = pip->cluster ;

/* do we have a MBTAB database? */

	if ((pip->mbfname == NULL) || (! pip->f.mbtab))
	    return SR_OK ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: mbtab=%s\n",
	        pip->progname,pip->mbfname) ;
	}

	if (pip->open.logmsg)
	    logfile_printf(&pip->lh,"mbtab=%s\n",pip->mbfname) ;

/* get some information about MAILPOLL */

	if ((rs = getportnum(proto,svc)) >= 0) {
	    defport = rs ;
	} else if (isNotPresent(rs)) {
	    defport = IPPORT_MAILPOLL ;
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("prognotify: defport=%d\n",defport) ;
#endif

	memset(&ri,0,sizeof(REPORTINFO)) ;
	ri.defport = defport ;

/* set some stuff that is relatively constant and used later */

	if (cluster == NULL) cluster = pip->nodename ;
	{
	    char	hashbuf[MAXHOSTNAMELEN+ 1] ;
	    cl = snsds(hashbuf,MAXHOSTNAMELEN,cluster,pip->logid) ;
	    ri.tag = hashelf(hashbuf,cl) ;
	}

	if (rs >= 0) {
	   const int	pf = PF_INET ;
	   const int	st = SOCK_DGRAM ;
	   const int	pnum = IPPROTO_UDP ;

/* initialze the message to send */

	memset(&ri.m1,0,sizeof(struct mcmsg_report)) ;
	ri.m1.tag = ri.tag ;
	ri.m1.rc = mcmsgrc_ok ;
	strwcpy(ri.m1.cluster,cluster,MCMSG_LCLUSTER) ;

/* go through the recipients */

	if ((rs = u_socket(pf,st,pnum)) >= 0) {
	    cchar	**envv = pip->envv ;
	    cchar	*mbfname = pip->mbfname ;
	    ri.fd = rs ;

	    if ((rs = paramfile_open(&mbtab,envv,mbfname)) >= 0) {
	        RECIP	*rp ;
	        int	i ;

	        for (i = 0 ; vecobj_get(rsp,i,&rp) >= 0 ; i += 1) {
	            if (rp != NULL) {
	                if (rp->ds >= 0) {
	                    rs = prognotifyrecip(pip,mip,&ri,&mbtab,rp) ;
	                }
		    }
	            if (rs < 0) break ;
	        } /* end for (recipients) */

	        paramfile_close(&mbtab) ;
	    } /* end if (paramfile) */

	    u_close(ri.fd) ;
	} /* end if (socket) */

	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("prognotify: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (prognotify) */


/* local subroutines */


static int prognotifyrecip(pip,mip,rip,mbp,rp)
PROGINFO	*pip ;
vecobj		*mip ;
REPORTINFO	*rip ;
PARAMFILE	*mbp ;
RECIP		*rp ;
{
	PARAMFILE_CUR	cur ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ul ;
	int		c = 0 ;
	const char	*up ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("prognotify: recip=%s rs=%d offset=%u\n",
	        rp->recipient,rp->rs,rp->offset) ;
#endif

	up = rp->recipient ;
	ul = strlen(up) ;

/* prepare some additional message information */

	strwcpy(rip->m1.mailuser,up,MIN(ul,MCMSG_LMAILUSER)) ;

/* loop through target machines for this mailuser (recipient) */

	if ((rs = paramfile_curbegin(mbp,&cur)) >= 0) {
	    const int	nslen = NSBUFLEN ;
	    const int	rsn = SR_NOTFOUND ;
	    int		port ;
	    int		nsl ;
	    char	nsbuf[NSBUFLEN + 1] ;
	    char	nn[NODENAMELEN + 1] ;

	    while (rs >= 0) {

	        nsl = paramfile_fetch(mbp,up,&cur,nsbuf,nslen) ;
	        if (nsl == rsn) break ;
	        rs = nsl ;
	        if (rs < 0) break ;

/* separate the node-spec into node and port */

	        if ((rs1 = parsenodespec(pip,nn,nsbuf,nsl)) >= 0) {
	            port = rs1 ;
	        } else if (rs1 == rsn) {
	            port = rip->defport ;
	        } else
	            rs = rs1 ;

	        if ((rs >= 0) && (nn[0] != '\0')) {
	            rs = prognotifyrecipnode(pip,mip,rip,rp,nn,port) ;
	            if (rs > 0) c += 1 ;
	        }

	    } /* end while (target machines) */

	    paramfile_curend(mbp,&cur) ;
	} /* end if (cursor) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (prognotifyrecip) */


static int prognotifyrecipnode(pip,mip,rip,rp,nn,port)
PROGINFO	*pip ;
vecobj		*mip ;
REPORTINFO	*rip ;
RECIP		*rp ;
const char	*nn ;
int		port ;
{
	struct hostent	he, *hep = &he ;
	struct msginfo	mi, *iep = &mi ;
	int		moff ;
	const int	helen = getbufsize(getbufsize_he) ;
	const int	af = AF_INET4 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		k ;
	int		ml ;
	int		c = 0 ;
	const char	*np = nn ;
	char		*hebuf ;

#if	CF_ISSAMEHOST
	if (issamehostname(np,pip->nodename,pip->domainname))
	    np = LOCALHOST ;
#else
	if (strcmp(np,pip->nodename) == 0)
	    np = LOCALHOST ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("prognotify: i=%d np=%s\n",i,np) ;
#endif

	if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	    if ((rs = getourhe(np,NULL,hep,hebuf,helen)) >= 0) {
		if (hep->h_addrtype == AF_INET) {
	    	    cchar	*a = (const char *) hep->h_addr ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        in_addr_t	ia ;
	        memcpy(&ia,hep->h_addr,sizeof(int)) ;
	        debugprintf("prognotify: got INET "
	            "address=\\x%08x\n",
	            ntohl(ia)) ;
	    }
#endif /* CF_DEBUG */

	    if ((rs = sockaddress_start(&rip->sa,af,a,port,0)) >= 0) {
	        rip->salen = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("prognotify: sockaddress_start() "
	                "rs=%d\n",
	                rip->salen) ;
#endif

/* send the messages */

	        for (k = 0 ; (ml = recip_getmo(rp,k,&moff)) >= 0 ; k += 1) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("prognotify: msg=%u mlen=%u\n", 
	                    k,ml) ;
#endif

/* find this message in the MSGINFO list */

	            mi.moff = moff ;
	            mi.mlen = ml ;
	            rs1 = vecobj_search(mip,&mi,searchfunc,&iep) ;

	            if (rs1 >= 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("prognotify: mlen=%u\n",
	                        iep->mlen) ;
#endif

	                rip->m1.mlen = iep->mlen ;
	                rip->m1.flags = 0 ;
	                rip->m1.flags |= (iep->f.spam) ? 1 : 0 ;
	                rip->m1.flags |= (iep->f.messageid) ? 2 : 0 ;
	                strwcpy(rip->m1.msgid,iep->h_messageid,
	                    MCMSG_LMSGID) ;

	                strwcpy(rip->m1.from,iep->h_from,
	                    MCMSG_LFROM) ;

/* send the report */

	                rs = report(pip,rip) ;
	                if (rs > 0) c += 1 ;

	            } /* end if (found message) */

	        } /* end for (messages) */

	        sockaddress_finish(&rip->sa) ;
	    } /* end if */

	    } /* end if (ver=inet4) */
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	    uc_free(hebuf) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (prognotifyrecipnode) */


static int report(PROGINFO *pip,struct reportinfo *rip)
{
	struct mcmsg_ack	m2 ;
	struct sockaddr		*sap = (struct sockaddr *) &rip->sa ;
	const int	mlen = MSGBUFLEN ;
	const int	to = pip->to_msgread ;
	const int	sflags = rip->sendflags ;
	const int	sal = rip->salen ;
	const int	n = MSGSENDTRIES ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, blen ;
	int		f = FALSE ;

	for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {

	    rip->m1.seq = (uchar) i ;
	    rip->m1.timestamp = (uint) time(NULL) ;

	    rs = mcmsg_report(&rip->m1,0,rip->buf,mlen) ;
	    blen = rs ;
	    if (rs < 0) break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("prognotify/report: fd=%u blen=%d sf=%08x\n",
	            rip->fd,blen,rip->sendflags) ;
	        debugprintf("prognotify/report: salen=%u\n",
	            rip->salen) ;
	    }
#endif

	    if ((rs = u_sendto(rip->fd,rip->buf,blen,sflags,sap,sal)) >= 0) {
		const int	fm = FM_TIMED ;
	        if ((rs1 = uc_recve(rip->fd,rip->buf,mlen,0,to,fm)) >= 0) {
	            blen = mcmsg_ack(&m2,1,rip->buf,mlen) ;
	            if ((blen > 0) && (m2.tag == rip->m1.tag)) f = TRUE ;
	        }
	    }

	    if (f) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (report) */


/* compare MSGINFO records */
static int searchfunc(e1pp,e2pp)
struct msginfo	**e1pp, **e2pp ;
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = ((*e1pp)->moff - (*e2pp)->moff) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	} else
	    rc = 0 ;
	return rc ;
}
/* end subroutine (searchfunc) */


