/* progcomsat */

/* handle COMAST matters */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_ISSAMEHOST	1		/* use 'issamehostname(3dam)' */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This code module was completely rewritten to replace the previous
	mail-delivery program for PCS, written around 1990 or so and which also
	served for a time to filter environment for the SENDMAIL daemon.

	= 2004-02-17, David A­D­ Morano
	This was modified to add the MSGID object.  That is a database that
	stores message IDs.  We used it to eliminate duplicate mail deliveries
	which as of late are coming from several popular sources!

*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module provides the handling for COMSAT matters.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<kvsfile.h>
#include	<sbuf.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"recip.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MBUFLEN		MAX((MAXPATHLEN+USERNAMELEN+32),MSGBUFLEN)

#define	PROTONAME	"udp"
#define	LOCALHOST	"localhost"

#ifndef	PORTSPEC_COMSAT
#define	PORTSPEC_COMSAT	"biff"
#endif

#ifndef	IPPORT_BIFFUDP
#define	IPPORT_BIFFUDP	512
#endif

#define	DEFNODES	20


/* external subroutines */

extern int	snscs(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	headkeymat(cchar *,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	mklogid(char *,int,cchar *,int,int) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_loadfile(vecstr *,int,cchar *) ;
extern int	getserial(cchar *) ;
extern int	getourhe(cchar *,cchar *,struct hostent *,char *,int) ;
extern int	mkgecosname(char *,int,cchar *) ;
extern int	mkrealame(char *,int,cchar *,int) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	issamehostname(cchar *,cchar *,cchar *) ;
extern int	isNotPresent(int) ;

extern int	parsenodespec(PROGINFO *,char *,cchar *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strdcpy3(char *,int,cchar *,cchar *,cchar *) ;
extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* local typedefs */


/* forward references */

static int	specs(PROGINFO *,vecobj *,vecstr *,int,int) ;
static int	recips(PROGINFO *,int,SOCKADDRESS *,int,vecobj *) ;
static int	rmtabnodes(PROGINFO *,vecstr *) ;
static int	report_node(PROGINFO *,cchar *) ;
static int	report_recip(PROGINFO *,cchar *,int,int,int) ;
static int	mkcsmsg(char *,int,cchar *,int,uint) ;


/* local variables */


/* exported subroutines */


int progcomsat(PROGINFO	*pip,vecobj *rsp)
{
	const int	pf = PF_INET4 ;
	int		rs ;
	int		rs1 ;
	int		defport ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (rsp == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcomsat: ent csfn=%s\n",pip->csfname) ;
#endif

/* get some information about COMSAT */

	defport = pip->port_comsat ;
	if (defport == 0) defport = IPPORT_BIFFUDP ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcomsat: defport=%d\n",defport) ;
#endif

/* create the socket to use for the messages */

	if ((rs = u_socket(pf,SOCK_DGRAM,IPPROTO_UDP)) >= 0) {
	    VECSTR	h ;
	    const int	n = DEFNODES ;
	    const int	fd = rs ;

/* get the COMSAT hosts */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progcomsat: get COMSAT hosts\n") ;
#endif

	    if ((rs = vecstr_start(&h,n,0)) >= 0) {
	        cchar	*csfn = pip->csfname ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progcomsat: csfn=%s\n",csfn) ;
#endif

	        if ((csfn != NULL) && (csfn[0] != '-') && (csfn[0] != '\0')) {

	            if (csfn[0] == '+') {

	                rs1 = rmtabnodes(pip,&h) ;
	                if (rs1 != SR_NOENT) rs = rs1 ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    int		i ;
	                    cchar	*np ;
	                    debugprintf("progcomsat: rmtabnodes() rs=%d\n",
	                        rs1) ;
	                    for (i = 0 ; vecstr_get(&h,i,&np) >= 0 ; i += 1) {
	                        debugprintf("progcomsat: node=%s\n",np) ;
	                    }
	                }
#endif /* CF_DEBUG */

	            } else {
	                rs = vecstr_loadfile(&h,0,csfn) ;
	                if (isNotPresent(rs)) rs = SR_OK ;
	            }

	        } /* end if (comsat-file-name) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progcomsat: mid rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            const int	rsn = SR_NOTFOUND ;
	            cchar	*nn = pip->nodename ;
	            if ((rs = vecstr_find(&h,nn)) == rsn) {
	                rs = vecstr_add(&h,nn,-1) ;
	            }
	        }

	        if (rs >= 0) {
	            rs = specs(pip,rsp,&h,fd,defport) ;
	            c += rs ;
	        } /* end if (ok) */

	        rs1 = vecstr_finish(&h) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */

	    u_close(fd) ;
	} /* end if (socket) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcomsat: ret rs=%d n=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (progcomsat) */


/* private subroutines */


static int specs(pip,rsp,nlp,fd,defport)
PROGINFO	*pip ;
vecobj		*rsp ;
vecstr		*nlp ;
int		fd ;
int		defport ;
{
	struct hostent	he, *hep = &he ;
	const int	helen = getbufsize(getbufsize_he) ;
	int		rs ;
	int		n = 0 ;
	char		*hebuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcomsat/specs: ent\n") ;
#endif

	if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	    int		i ;
	    int		port ;
	    cchar	*nsp, *np ;
	    char	nodename[NODENAMELEN + 1] ;
	    for (i = 0 ; vecstr_get(nlp,i,&nsp) >= 0 ; i += 1) {
	        if (nsp == NULL) continue ;

/* separate the node-spec into node and port */

	        np = nodename ;
	        rs = parsenodespec(pip,nodename,nsp,-1) ;
	        port = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("progcomsat/specs: parsenodespec() rs=%d\n",
			rs) ;
	            debugprintf("progcomsat/specs: nodename=%s\n",
			nodename) ;
	        }
#endif

	        if (rs == SR_NOTFOUND) rs = 0 ;
	        if (rs == 0) port = defport ;

	        if ((rs >= 0) && (nodename[0] != '\0')) {
	            const int	af = AF_INET4 ;

/* continue */

#if	CF_ISSAMEHOST
	            if (issamehostname(np,pip->nodename,pip->domainname))
	                np = LOCALHOST ;
#else
	            if (strcmp(np,pip->nodename) == 0)
	                np = LOCALHOST ;
#endif /* CF_ISSAMEHOST */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progcomsat/specs: i=%d port=%u np=%s\n",
	                    i,port,np) ;
#endif

		    report_node(pip,np) ;

	            if ((rs = getourhe(np,NULL,hep,hebuf,helen)) >= 0) {
	                if (hep->h_addrtype == af) {
	                    SOCKADDRESS	sa ;
			    const int	ps = port ;
	                    cchar	*a = (cchar *) hep->h_addr ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    in_addr_t	ia ;
	                    memcpy(&ia,hep->h_addr,sizeof(int)) ;
	                    debugprintf("progcomsat/specs: addr=\\x%08x\n",
	                        ntohl(ia)) ;
	                }
#endif /* CF_DEBUG */

	                    if ((rs = sockaddress_start(&sa,af,a,ps,0)) >= 0) {
	                        const int	sal = rs ;

	                        rs = recips(pip,fd,&sa,sal,rsp) ;
	                        n += rs ;

	                        sockaddress_finish(&sa) ;
	                    } /* end if (sockaddress) */
			} /* end if (address-family) */
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            }

	        } /* end if (had a possible node) */

	        if (rs < 0) break ;
	    } /* end for (hosts) */
	    uc_free(hebuf) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcomsat/specs: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (specs) */


static int recips(pip,fd,sap,sal,rsp)
PROGINFO	*pip ;
int		fd ;
SOCKADDRESS	*sap ;
int		sal ;
vecobj		*rsp ;
{
	RECIP		*rp ;
	const int	clen = MBUFLEN ;
	const int	mflags = 0 ;
	int		rs = SR_OK ;
	int		j, ul ;
	int		cl ;
	int		n = 0 ;
	cchar		*up ;
	char		cbuf[MBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcomsat/recips: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ;

	for (j = 0 ; vecobj_get(rsp,j,&rp) >= 0 ; j += 1) {
	    int		mbo ;
	    if (rp == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progcomsat/recips: "
	            "recip=%s ds=%d\n",rp->recipient,rp->ds) ;
#endif

	    if ((mbo = recip_getmbo(rp)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progcomsat/recips: mbo=%u\n",mbo) ;
#endif
	        if ((ul = recip_get(rp,&up)) > 0) {
	            int		k ;
	            int		mo, ml ;
	            for (k = 0 ; (ml = recip_getmo(rp,k,&mo)) >= 0 ; k += 1) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progcomsat/recips: "
	                        "msg=%u mo=%u ml=%u\n", k,mo,ml) ;
#endif

			report_recip(pip,up,ul,mbo,mo) ;

	                if ((rs = mkcsmsg(cbuf,clen,up,ul,mbo)) > 1) {
	                    cl = rs ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progcomsat/recips: cm=>%t<\n",
	                            cbuf,cl) ;
#endif

	                    rs = u_sendto(fd,cbuf,cl,mflags,sap,sal) ;
	                    if (rs >= 0) n += 1 ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progcomsat/recips: "
	                            "u_sendto() rs=%d\n", rs) ;
#endif

	                } /* end if */

	                mbo += ml ;

	                if (rs < 0) break ;
	            } /* end for (messages) */
	        } /* end if (recipient-name) */
	    } /* end if (get MBO) */

	    if (rs < 0) break ;
	} /* end for (looping through recipients) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progcomsat/recips: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (recips) */


static int rmtabnodes(PROGINFO *pip,vecstr *nlp)
{
	KVSFILE		info ;
	KVSFILE_CUR	cur ;
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs ;
	int		rs1 ;
	int		hl ;
	int		c = 0 ;
	char		hbuf[MAXHOSTNAMELEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = kvsfile_open(&info,50,RMTABFNAME)) >= 0) {

	    if ((rs = kvsfile_curbegin(&info,&cur)) >= 0) {
	        const int	rsn = SR_NOTFOUND ;

	        while (rs >= 0) {

	            hl = kvsfile_enumkey(&info,&cur,hbuf,hlen) ;
	            if (hl == SR_NOTFOUND) break ;
	            rs = hl ;

	            if (rs >= 0) {
	                if ((rs = vecstr_findn(nlp,hbuf,hl)) == rsn) {
	                    rs = vecstr_add(nlp,hbuf,hl) ;
	                    c += 1 ;
	                }
	            } /* end if */

	        } /* end while (getting keys) */

	        rs1 = kvsfile_curend(&info,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */

	    rs1 = kvsfile_close(&info) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (kvsfile) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (rmtabnodes) */


static int report_node(PROGINFO *pip,cchar *node)
{
	int		rs = SR_OK ;

	proglog_printf(pip,"comsat n=%s",node) ;

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: comsat n=%s\n" ;
	    bprintf(pip->efp,fmt,node) ;
	}

	return rs ;
}
/* end subroutine (report_node) */


static int report_recip(PROGINFO *pip,cchar *up,int ul,int mbo,int mo)
{
	int		rs = SR_OK ;

	proglog_printf(pip,"comsat   r=%t mbo=%u mo=%u",up,ul,mbo,mo) ;

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: comsat r=%t mbo=%u mo=%u\n" ;
	    bprintf(pip->efp,fmt,up,ul,mbo,mo) ;
	}

	return rs ;
}
/* end subroutine (report_recip) */


/* make (marshall) the COMSAT message itself */
static int mkcsmsg(char mbuf[],int mlen,cchar *up,int ul,uint val)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&b,mbuf,mlen)) >= 0) {

	    sbuf_strw(&b,up,ul) ;

	    sbuf_char(&b,'@') ;

	    sbuf_deci(&b,val) ;

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mkcsmsg) */


