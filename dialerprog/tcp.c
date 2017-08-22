/* tcp */

/* TCP dialer */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This was created as one of the first dialer modules for the SYSDIALER
        object.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a dialer module.

	Synopsis:

	tcp [[<host>:]<port>] [-f <af>]

	Arguments:

	host		override hn
	port		service port
	af		address family

	Returns:

	<0		error
	>=0		file-descriptor


*******************************************************************************/


#define	TCP_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"tcp.h"
#include	"sysdialer.h"


/* local defines */

#define	TCP_MNAME	"tcp"
#define	TCP_VERSION	"0"
#define	TCP_INAME	""
#define	TCP_FLAGS1	\
		(SYSDIALER_MFULL | SYSDIALER_MHALFOUT | SYSDIALER_MHALFIN)
#define	TCP_FLAGS2	(SYSDIALER_MCOR | SYSDIALER_MCO)
#define	TCP_FLAGS	(TCP_FLAGS1 | TCP_FLAGS2)

#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#define	ARGBUFLEN	(MAXPATHLEN + 35)

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100
#define	NARGPRESENT	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(const char *,const char *,int,const char *,int,int) ;
extern int	dialtcpmux(cchar *,cchar *,int,cchar *,cchar **,int,int) ;
extern int	isdigitlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct afamily {
	const char	*name ;
	int		af ;
} ;


/* forward references */


/* global variables (module information) */

SYSDIALER_INFO	tcp = {
	TCP_MNAME,
	TCP_VERSION,
	TCP_INAME,
	sizeof(TCP),
	TCP_FLAGS
} ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"af",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_af,
	argopt_overlast
} ;

static const struct afamily	afs[] = {
	{ "inet", AF_INET },
	{ "inet4", AF_INET },
#ifdef	AF_INET6
	{ "inet6", AF_INET6 },
#endif
	{ NULL, 0 }
} ;


/* exported subroutines */


int tcp_open(TCP *op,SYSDIALER_ARGS *ap,cchar hn[],cchar svc[],cchar *av[])
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		to = -1 ;
	int		af = AF_UNSPEC ;
	int		opts = 0 ;
	const char	*pr = NULL ;
	char		hnbuf[MAXHOSTNAMELEN + 1] ;
	char		svcbuf[SVCNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic == TCP_MAGIC) return SR_INUSE ;

#if	CF_DEBUGS
	debugprintf("tcp_open: entered hn=%s svc=%s\n",
	    hn,svc) ;
#endif

	if (ap != NULL) {
	    int	argr, argl, aol, avl ;
	    int	maxai, pan, npa, kwi, i ;
	    int	argnum ;
	    int	cl ;
	    int	f_optminus, f_optplus, f_optequal ;
	    int	f_extra = FALSE ;
	    int	f_bad = FALSE ;

	    const char	**argv, *argp, *aop, *avp ;
	    char	argpresent[NARGPRESENT] ;
	    const char	*afspec = NULL ;
	    const char	*hostsvc = NULL ;
	    const char	*cp ;


#if	CF_DEBUGS
	    debugprintf("tcp_open: arguments\n") ;
#endif

	    hnbuf[0] = '\0' ;
	    svcbuf[0] = '\0' ;

	    to = ap->timeout ;
	    opts = ap->options ;
	    argv = ap->argv ;
	    if (ap->pr != NULL)
	        pr = (char *) ap->pr ;

/* process program arguments */

	    for (i = 0 ; i < NARGPRESENT ; i += 1) argpresent[i] = 0 ;

	    npa = 0 ;			/* number of positional so far */
	    maxai = 0 ;
	    i = 0 ;
	    while ((argv[i] != NULL) && (argv[i + 1] != NULL)) {

	        argp = argv[++i] ;
	        argl = strlen(argp) ;

#if	CF_DEBUGS
	        debugprintf("tcp_open: argl=%u argp=%p\n",argl,argp) ;
#endif

	        f_optminus = (*argp == '-') ;
	        f_optplus = (*argp == '+') ;
	        if ((argl > 0) && (f_optminus || f_optplus)) {

	            if (argl > 1) {
			const int	ach = MKCHAR(argp[1]) ;

	                if (isdigitlatin(ach)) {

	                    if (cfdeci(argp + 1,argl - 1,&argnum))
	                        goto badargval ;

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option\n") ;
#endif

	                    aop = argp + 1 ;
	                    aol = argl - 1 ;
	                    f_optequal = FALSE ;
	                    if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                        debugprintf("main: key w/ value\n") ;
#endif

	                        aol = avp - aop ;
	                        avp += 1 ;
	                        avl = aop + argl - 1 - avp ;
	                        f_optequal = TRUE ;

	                    } else
	                        avl = 0 ;

	                    if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                        switch (kwi) {

/* program root */
	                        case argopt_root:
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    pr = avp ;

	                            } else {

	                                if (argv[i + 1] == NULL)
	                                    goto badargnum ;

	                                argp = argv[++i] ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    pr = argp ;

	                            }

	                            break ;

	                        case argopt_af:
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    afspec = avp ;

	                            } else {

	                                if (argv[i + 1] == NULL)
	                                    goto badargnum ;

	                                argp = argv[++i] ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    afspec = argp ;

	                            }

	                            break ;

	                        } /* end switch (key words) */

	                    } else {

	                        while (aol--) {

				    int	kc = (*aop & 0xff) ;
	                            switch (kc) {

	                            case 'f':
	                                if (argv[i + 1] == NULL)
	                                    goto badargnum ;

	                                argp = argv[++i] ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    afspec = argp ;

	                                break ;

/* service name */
	                            case 's':
	                                if (argv[i + 1] == NULL)
	                                    goto badargnum ;

	                                argp = argv[++i] ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    svc = argp ;

	                                break ;

/* timeout */
	                            case 't':
	                                if (argv[i + 1] == NULL)
	                                    goto badargnum ;

	                                argp = argv[++i] ;
	                                argl = strlen(argp) ;

	                                if (argl) {

	                                    rs = cfdeci(argp,argl,&to) ;
	                                    if (rs < 0)
	                                        goto badargval ;

	                                }

	                                break ;

	                            default:
	                                f_extra = TRUE ;
	                                f_bad = TRUE ;

	                            } /* end switch */

	                            aop += 1 ;
				    if (rs < 0)
					break ;

	                        } /* end while */

	                    } /* end if (individual option key letters) */

	                } /* end if (digits as argument or not) */

	            } else {

	                if (i < MAXARGINDEX) {

	                    BASET(argpresent,i) ;
	                    maxai = i ;
	                    npa += 1 ;	/* increment position count */

	                }

	            } /* end if */

	        } else {

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;

	            } else {

	                f_extra = TRUE ;
	                f_bad = TRUE ;
	            }

	        } /* end if (key letter/word or positional) */

	        if (f_bad)
	            break ;

	    } /* end while (all command line argument processing) */

	    if (f_bad)
	        goto badarg ;

	    if (npa > 0) {

	        pan = 0 ;
	        for (i = 0 ; i <= maxai ; i += 1) {

	            if (BATST(argpresent,i)) {

	                switch (pan) {

	                case 0:
	                    hostsvc = argv[i] ;

#if	CF_DEBUGS
	                    debugprintf("tcp_open: pan=%d hostsvc=%s\n",
				pan,hostsvc) ;
#endif

	                    break ;

	                case 1:
	                    break ;

	                default:
	                     ;

	                } /* end switch */

	                pan += 1 ;

	            } /* end if (argument present) */

	        } /* end for */

	    } /* end if (positional arguments) */

	    if ((hostsvc != NULL) && (hostsvc[0] != '\0')) {

	        if ((cp = strchr(hostsvc,':')) != NULL) {

	            cl = MIN((cp - hostsvc),SVCNAMELEN) ;
	            if (cl > 0) {
	                hn = hnbuf ;
	                strwcpy(hnbuf,hostsvc,cl) ;
	            }

	            cp += 1 ;
	            if (cp[0] != '\0')
	                svc = cp ;

	        } else
	            svc = hostsvc ;

	    } /* end if */

	    if ((afspec != NULL) && (afspec[0] != '\0')) {

	        for (i = 0 ; afs[i].name != NULL ; i += 1) {

	            if (strcmp(afs[i].name,afspec) == 0)
	                break ;

	        }

	        if (afs[i].name != NULL) {
	            af = afs[i].af ;
		} else
	            rs = SR_INVALID ;

	    } /* end if (address family specification) */

	} /* end if (had arguments) */

#if	CF_DEBUGS
	debugprintf("tcp_open: done w/ arguments\n") ;
	debugprintf("tcp_open: hn=%s svc=%s\n",
	    hn,svc) ;
	debugprintf("tcp_open: af=%u\n",af) ;
#endif

/* OK, do the dial */

	if (rs >= 0) {
	    rs = dialtcp(hn,svc,af,to,opts) ;
	    op->fd = rs ;
	}

	if (rs >= 0) {
	    op->magic = TCP_MAGIC ;
	    uc_closeonexec(op->fd,TRUE) ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("tcp_open: ret rs=%d fd=%d\n",rs,op->fd) ;
#endif

	return (rs >= 0) ? op->fd : rs ;

/* bad stuff */
badargval:
badargnum:
badarg:
	rs = SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("tcp_open: ret BAD \n") ;
#endif

	goto ret0 ;
}
/* end subroutine (tcp_open) */


int tcp_reade(TCP *op,char buf[],int blen,int to,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = uc_reade(op->fd,buf,blen,to,opts) ;

	return rs ;
}


int tcp_recve(TCP *op,char buf[],int blen,int flags,int to,int opts)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = uc_recve(op->fd,buf,blen,flags,to,opts) ;

	return rs ;
}


int tcp_recvfrome(op,buf,blen,flags,sap,salenp,to,opts)
TCP		*op ;
char		buf[] ;
int		blen ;
int		flags ;
void		*sap ;
int		*salenp ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = uc_recvfrome(op->fd,buf,blen,flags,sap,salenp,to,opts) ;

	return rs ;
}


int tcp_recvmsge(op,msgp,flags,to,opts)
TCP		*op ;
struct msghdr	*msgp ;
int		flags ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = uc_recvmsge(op->fd,msgp,flags,to,opts) ;

	return rs ;
}


int tcp_write(op,buf,blen)
TCP		*op ;
const char	buf[] ;
int		blen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = uc_writen(op->fd,((void *) buf),blen) ;

	return rs ;
}


int tcp_send(op,buf,blen,flags)
TCP		*op ;
const char	buf[] ;
int		blen ;
int		flags ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = u_send(op->fd,buf,blen,flags) ;

	return rs ;
}


int tcp_sendto(op,buf,blen,flags,sap,salen)
TCP		*op ;
const char	buf[] ;
int		blen ;
int		flags ;
void		*sap ;
int		salen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = u_sendto(op->fd,buf,blen,flags,sap,salen) ;

	return rs ;
}


int tcp_sendmsg(op,msgp,flags)
TCP		*op ;
struct msghdr	*msgp ;
int		flags ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = u_sendmsg(op->fd,msgp,flags) ;

	return rs ;
}


/* shutdown */
int tcp_shutdown(op,cmd)
TCP		*op ;
int		cmd ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = u_shutdown(op->fd,cmd) ;

	return rs ;
}
/* end subroutine (tcp_shutdown) */


/* close the connection */
int tcp_close(op)
TCP		*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TCP_MAGIC) return SR_NOTOPEN ;

	rs = u_close(op->fd) ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (tcp_close) */


