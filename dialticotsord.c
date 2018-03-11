/* dialticotsord */

/* subroutine to dial over to a UNIX® domaiun socket */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_PUSHMOD	0		/* push TIRDWR */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will dial out to the TICOTSORD transport.

	Synopsis:

	int dialticotsord(addr,alen,to,opts)
	const char	addr[] ;
	int		alen ;
	int		to ;
	int		opts ;

	Arguments:

	addr		XTI address
	alen		address of XTI address
	to		to ('>=0' mean use one, '-1' means don't)
	opts		any dial options

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<xti.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	ADDRBUFLEN	MAXPATHLEN
#define	CONNTIMEOUT	120		/* seconds */
#define	LINGERTIME	(3 * 6)		/* seconds */
#define	TPIDEV		"/dev/ticotsord"

#define	HEXBUFLEN	((2 * MAXPATHLEN) + 2)

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	snxtilook(char *,int,int) ;
extern int	sfshrink(cchar *,int,const char **) ;
extern int	cfhexs(cchar *,int,char *) ;

extern char	*strnwcpy(char *,int,const char *,int) ;


/* external variables */


/* local structures */

struct subinfo {
	int		f ;
} ;


/* forward references */

static int	makeconn(SUBINFO *,const char *,int,int) ;

#if	CF_PUSHMOD
static int	pushmod(int,const char *) ;
#endif /* CF_PUSHMOD */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	mkhexstr(char *,int,const void *,int) ;
static int	shownetbuf(struct netbuf *,cchar *) ;
#endif


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int dialticotsord(cchar abuf[],int alen,int to,int opts)
{
	SUBINFO		g ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	char		addrbuf[ADDRBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("dialticotsord: ent\n") ;
#endif

	if (abuf == NULL) return SR_FAULT ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("dialticotsord: alen=%d\n",alen) ;
#endif

	if (alen < 0) {
	    if (strncmp(abuf,"\\x",2) == 0) {
	        abuf += 2 ;
	        alen = strlen(abuf) ;
#if	CF_DEBUGS
		debugprintf("dialticotsord: al=%u a=>%t<\n",alen,abuf,alen) ;
#endif
	        if ((alen >> 1) <= ADDRBUFLEN) {
	            rs = cfhexs(abuf,alen,addrbuf) ;
	            abuf = addrbuf ;
		    alen = rs ;
	        } else
	            rs = SR_TOOBIG ;
	    } else
	        alen = strlen(abuf) ;
#if	CF_DEBUGS
	    debugprintf("dialticotsord: X rs=%d alen=%d\n",rs,alen) ;
#endif
	} /* end if */

#if	CF_DEBUGS
	{
	    int		hl ;
	    char	hbuf[HEXBUFLEN + 1] ;
	    hl = mkhexstr(hbuf,HEXBUFLEN,abuf,alen) ;
	    debugprintf("dialticotsord: XTI alen=%d abuf=%t\n",
	        alen,hbuf,hl) ;
	}
#endif /* CF_DEBUGS */

/* try to connect to the remote machine */

	if (rs >= 0) {
	    rs = makeconn(&g,abuf,alen,to) ;
	    fd = rs ;
	}

#if	CF_DEBUGS
	debugprintf("dialticotsord: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (dialticotsord) */


/* local subroutines */


static int makeconn(SUBINFO *gp,cchar addr[],int alen,int to)
{
	struct t_info	info ;
	int		rs ;
	int		fd = -1 ;

#if	CF_DEBUGS
	debugprintf("dialticotsord/makeconn: ent to=%d\n",to) ;
#endif

	if (gp == NULL) return SR_FAULT ;

/* create the primary socket */

	memset(&info,0,sizeof(struct t_info)) ;

	if ((rs = ut_open(TPIDEV,O_RDWR,&info)) >= 0) {
	    fd = rs ;

#if	CF_DEBUGS
	    debugprintf("dialticotsord/makeconn: opened rs=%d\n",rs) ;
	    debugprintf("dialticotsord/makeconn: addrlen=%d\n",info.addr) ;
	    debugprintf("dialticotsord/makeconn: optlen=%d\n",info.options) ;
	    debugprintf("dialticotsord/makeconn: tsdu=%d\n",info.tsdu) ;
	    debugprintf("dialticotsord/makeconn: connlen=%d\n",info.connect) ;
	    debugprintf("dialticotsord/makeconn: dislen=%d\n",info.discon) ;
	    debugprintf("dialticotsord/makeconn: svctype=%d\n",info.servtype) ;
#endif

	    if ((rs = ut_bind(fd,NULL,NULL)) >= 0) {
	        struct t_call	*sndcall ;

/* attempt to connect to the host */

#if	CF_DEBUGS
	        debugprintf("dialticotsord/makeconn: attempt connect\n") ;
#endif

	        if ((rs = ut_alloc(fd,T_CALL,0,(void **) &sndcall)) >= 0) {

	            sndcall->addr.maxlen = alen ;
	            sndcall->addr.buf = (char *) addr ;
	            sndcall->addr.len = alen ;

#if	CF_DEBUGS
	            debugprintf("dialticotsord/makeconn: ut_alloc() rs=%d\n",
			rs) ;
	            shownetbuf(&sndcall->addr,"addr") ;
	            shownetbuf(&sndcall->opt,"opt") ;
	            shownetbuf(&sndcall->udata,"udata") ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	            {
	                int	hl ;
	                char	hbuf[HEXBUFLEN + 1] ;
	                hl = mkhexstr(hbuf,HEXBUFLEN,addr,alen) ;
	                debugprintf("dialticotsord/makeconn: "
				"XTI alen=%u addr=%t\n",
				alen,hbuf,hl) ;
	            }
#endif /* CF_DEBUGS */

	            rs = ut_connect(fd,sndcall,NULL) ;

#if	CF_DEBUGS
	            debugprintf("dialticotsord/makeconn: ut_connect() rs=%d\n",
			rs) ;
#endif

	            sndcall->addr.maxlen = 0 ;
	            sndcall->addr.buf = NULL ;
	            sndcall->addr.len = 0 ;
	            ut_free(sndcall,T_CALL) ;
	        } /* end if (alloc) */

#if	CF_DEBUGS
	            debugprintf("dialticotsord/makeconn: mid1 rs=%d\n",
			rs) ;
#endif

/* was this "busy" at all, requiring a TLOOK operation? */

	        if ((rs == SR_BUSY) || (rs == SR_LOOK)) {

#if	CF_DEBUGS
	            debugprintf("dialticotsord/makeconn: got a BUSY! rs=%d\n",
			rs) ;
#endif

	            rs = ut_look(fd) ;

#if	CF_DEBUGS
		    {
			const int	tlen = MAXNAMELEN ;
			char	tbuf[MAXNAMELEN+1] ;
			snxtilook(tbuf,tlen,rs) ;
	            debugprintf("dialticotsord/makeconn: "
			"ut_look() rs=%d (%s)\n",rs,tbuf) ;
		    }
#endif

	        }

#if	CF_PUSHMOD
	        if (rs >= 0) {
	            rs = pushmod(fd,"tirdwr") ;
	        }
#endif /* CF_PUSHMOD */

	    } /* end if (bind) */

#if	CF_DEBUGS
	    debugprintf("dialticotsord/makeconn: bind-out rs=%d\n",rs) ;
#endif

	    if (rs < 0) {
	        u_close(fd) ;
	    }
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("dialticotsord/makeconn: open-out rs=%d",rs) ;
#endif

#if	CF_DEBUGS
	debugprintf("dialticotsord/makeconn: ret rs=%d fd=%d\n",
	    rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (makeconn) */


/* local subroutines */


#if	CF_PUSHMOD
static int pushmod(int fd,cchar mods[])
{
	int		rs = SR_OK ;
	const char	*timod = "timod" ;

	if (mods == NULL) return SR_FAULT ;
	if (fd < 0) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("dialticotsord/pushmod: ent\n") ;
#endif

	if (strcmp(mods,timod) == 0) {

	    if ((rs = u_ioctl(fd,I_LOOK,timod)) == SR_INVALID) {
	        rs = u_ioctl(fd,I_PUSH,timod) ;
	    }

	} else { /* pop 'timod' if it is on the stack */
	    char	mbuf[MAXNAMELEN + 1] ;

	    if ((rs = u_ioctl(fd,I_LOOK,mbuf)) >= 0) {
	        if (strcmp(mbuf,timod) == 0) {
	            rs = u_ioctl(fd,I_POP,0) ;
	        }
	    } else if (rs == SR_INVALID)
	        rs = SR_OK ;

	    if (rs >= 0) {
	        int		cl ;
	        const char	*sp = mods ;
	        const char	*tp, *cp ;

	        while ((tp = strchr(sp,',')) != NULL) {
	            if ((cl = sfshrink(sp,(tp-sp),&cp)) > 0) {
	                strnwcpy(mbuf,MAXNAMELEN,cp,cl) ;
	                rs = u_ioctl(fd,I_PUSH,mbuf) ;
	            }
	            sp = (tp + 1) ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && sp[0]) {
	            if ((cl = sfshrink(sp,-1,&cp)) > 0) {
	                strnwcpy(mbuf,MAXNAMELEN,cp,cl) ;
	                rs = u_ioctl(fd,I_PUSH,mbuf) ;
	            }
	        } /* end if */

	    } /* end if (pushing) */

	} /* end if (easy or more complex) */

	return rs ;
}
/* end subroutine (pushmod) */
#endif /* CF_PUSHMOD */


#if	CF_DEBUGS
static int shownetbuf(struct netbuf *p,cchar *s)
{
	debugprintf("shownetbuf: id=%s\n",s) ;
	debugprintf("shownetbuf: maxlen=%d\n",p->maxlen) ;
	debugprintf("shownetbuf: len=%d\n",p->len) ;
	debugprinthex("shownetbuf",80,p->buf,p->len) ;
	return 0 ;
}
/* end suboroutine (shownetbuf) */
#endif /* CF_DEBUGS */


