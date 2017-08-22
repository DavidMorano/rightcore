/* udp */

/* SYSDIALER "udp" dialer */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This was created as one of the first dialer modules for the SYSDIALER
        object.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a SYSDIALER module.

	Synopsis:

	udp [[<host>:]<port>] [-f af] [-bp <backupport>]

	Arguments:

	+ host		override hostname
	+ port		service port
	+ af		address family


*******************************************************************************/


#define	UDP_MASTER	0


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

#include	"udp.h"
#include	"sysdialer.h"


/* local defines */

#define	UDP_MNAME	"udp"
#define	UDP_VERSION	"0"
#define	UDP_INAME	""
#define	UDP_MF1		(SYSDIALER_MFULL | SYSDIALER_MHALFOUT)
#define	UDP_MF2		(SYSDIALER_MCOR | SYSDIALER_MCO)
#define	UDP_MF3		(SYSDIALER_MHALFIN)
#define	UDP_MF		(UDP_MF1 | UDP_MF2|UDP_MF3)

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
extern int	dialudp(const char *,const char *,int,int,int) ;
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

SYSDIALER_INFO	udp = {
	UDP_MNAME,
	UDP_VERSION,
	UDP_INAME,
	sizeof(UDP),
	UDP_MF
} ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"RN",
	"af",
	"bp",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_rn,
	argopt_af,
	argopt_bp,
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


int udp_open(op,ap,hostname,svcname,av)
UDP		*op ;
SYSDIALER_ARGS	*ap ;
const char	hostname[] ;
const char	svcname[] ;
const char	*av[] ;
{
	int		rs = SR_OK ;
	int		to = -1 ;
	int		af = AF_UNSPEC ;
	int		opts = 0 ;

	char		hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char		svcnamebuf[SVCNAMELEN + 1] ;
	char		*pr = NULL ;
	char		*bpspec = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic == UDP_MAGIC) return SR_INUSE ;

#if	CF_DEBUGS
	debugprintf("udp_open: entered hostname=%s svcname=%s\n",
		hostname,svcname) ;
#endif

	if (ap != NULL) {

	    int	argr, argl, aol, avl ;
	    int	ai, maxai, pan, npa, kwi ;
	    int		argvalue = -1 ;
	    int	rs, i ;
	    int	cl ;
	    int	f_optminus, f_optplus, f_optequal ;
	    int	f_extra = FALSE ;

	    char	**argv, *argp, *aop, *avp ;
	    char	argpresent[NARGPRESENT] ;
	    char	*afspec = NULL ;
	    char	*hostsvc = NULL ;
	    char	*cp ;

#if	CF_DEBUGS
	debugprintf("udp_open: arguments\n") ;
#endif

	    hostnamebuf[0] = '\0' ;
	    svcnamebuf[0] = '\0' ;

	    to = ap->timeout ;
	    opts = ap->options ;
	    argv = (char **) ap->argv ;
	    if (ap->pr != NULL)
	        pr = (char *) ap->pr ;

/* process program arguments */

	    for (ai = 0 ; ai < NARGPRESENT ; ai += 1) 
		argpresent[ai] = 0 ;

	    npa = 0 ;			/* number of positional so far */
	    maxai = 0 ;
	    ai = 0 ;
	    while ((rs >= 0) && 
		(argv[ai] != NULL) && (argv[ai + 1] != NULL)) {

	        argp = argv[++ai] ;
	        argl = strlen(argp) ;

	        f_optminus = (*argp == '-') ;
	        f_optplus = (*argp == '+') ;
	        if ((argl > 1) && (f_optminus || f_optplus)) {
			const int	ach = MKCHAR(argp[1]) ;

	                if (isdigitlatin(ach)) {

			    if ((argl - 1) > 0)
	                        rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	                } else {

	                    aop = argp + 1 ;
	                    aol = argl - 1 ;
	                    f_optequal = FALSE ;
	                    if ((avp = strchr(aop,'=')) != NULL) {

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

	                                if (argv[ai + 1] == NULL) {
					    rs = SR_INVALID ;
	                                    break ;
					}

	                                argp = argv[++ai] ;
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

	                                if (argv[ai + 1] == NULL) {
					    rs = SR_INVALID ;
	                                    break ;
					}

	                                argp = argv[++ai] ;
	                                argl = strlen(argp) ;

	                                if (argl)
						afspec = argp ;

	                            }

	                            break ;

	                        case argopt_bp:
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
						bpspec = avp ;

	                            } else {

	                                if (argv[ai + 1] == NULL) {
					    rs = SR_INVALID ;
	                                    break ;
					}

	                                argp = argv[++ai] ;
	                                argl = strlen(argp) ;

	                                if (argl)
						bpspec = argp ;

	                            }

	                            break ;

	                        } /* end switch (key words) */

	                    } else {

	                        while (aol--) {

	                            switch ((int) *aop) {

	                            case 'f':
	                                if (argv[ai + 1] == NULL) {
					    rs = SR_INVALID ;
	                                    break ;
					}

	                                argp = argv[++ai] ;
	                                argl = strlen(argp) ;

	                                if (argl)
						afspec = argp ;

	                                break ;

/* service name */
	                            case 's':
	                                if (argv[ai + 1] == NULL) {
					    rs = SR_INVALID ;
	                                    break ;
					}

	                                argp = argv[++ai] ;
	                                argl = strlen(argp) ;

	                                if (argl)
	                                    svcname = argp ;

	                                break ;

/* timeout */
	                            case 't':
	                                if (argv[ai + 1] == NULL) {
					    rs = SR_INVALID ;
	                                    break ;
					}

	                                argp = argv[++ai] ;
	                                argl = strlen(argp) ;

	                                if (argl) {

	                                    rs = cfdeci(argp,argl,&to) ;

	                                }

	                                break ;

	                            default:
	                                rs = SR_INVALID ;

	                            } /* end switch */

	                            aop += 1 ;
				    if (rs < 0)
					break ;

	                        } /* end while */

	                    } /* end if (individual option key letters) */

	                } /* end if (digits as argument or not) */

	        } else {

	            if (ai < MAXARGINDEX) {

	                BASET(argpresent,ai) ;
	                maxai = ai ;
	                npa += 1 ;

	            } else {

			rs = SR_INVALID ;
	                f_extra = TRUE ;
	            }

	        } /* end if (key letter/word or positional) */

	    } /* end while (all command line argument processing) */

	    if (rs < 0)
	        goto badarg ;

		pan = 0 ;
		if (npa > 0) {

		for (ai = 0 ; ai <= maxai ; ai += 1) {

		    if (BATST(argpresent,ai)) {

			switch (pan) {
			case 0:
				hostsvc = argv[ai] ;
				break ;
			} /* end switch */
			pan += 1 ;

                    } /* end if (argument present) */

		} /* end for */

	    } /* end if (positional arguments) */

	    if ((hostsvc != NULL) && (hostsvc[0] != '\0')) {

	        if ((cp = strchr(hostsvc,':')) != NULL) {

	            cl = MIN((cp - hostsvc),SVCNAMELEN) ;
	            if (cl > 0) {

	                hostname = hostnamebuf ;
	                strwcpy(hostnamebuf,hostsvc,cl) ;

	            }

	            cp += 1 ;
	            if (cp[0] != '\0')
	                svcname = cp ;

	        } else
	            svcname = hostsvc ;

	    } /* end if */

		if ((afspec != NULL) && (afspec[0] != '\0')) {

			for (i = 0 ; afs[i].name != NULL ; i += 1) {

				if (strcmp(afs[i].name,afspec) == 0)
					break ;

			}

			if (afs[i].name != NULL)
				af = afs[i].af ;

			else
			rs = SR_INVALID ;

		} /* end if (address family specification) */

	} /* end if (had arguments) */

#if	CF_DEBUGS
	debugprintf("udp_open: done w/ arguments\n") ;
	debugprintf("udp_open: hostname=%s svcname=%s\n",
		hostname,svcname) ;
	debugprintf("udp_open: af=%u\n",af) ;
#endif

/* OK, do the dial */

	if (rs >= 0) {

		rs = dialudp(hostname,svcname,af,to,opts) ;

		if ((rs < 0) && (bpspec != NULL))
			rs = dialudp(hostname,bpspec,af,to,opts) ;

	}

	op->fd = rs ;
	if (rs >= 0) {

	    op->magic = UDP_MAGIC ;
		uc_closeonexec(op->fd,TRUE) ;

	}

ret0:

#if	CF_DEBUGS
	debugprintf("udp_open: ret rs=%d fd=%d\n",rs,op->fd) ;
#endif

	return (rs >= 0) ? op->fd : rs ;

/* bad stuff */
badarg:

#if	CF_DEBUGS
	debugprintf("udp_open: ret BAD \n") ;
#endif

	return SR_INVALID ;
}
/* end subroutine (udp_open) */


int udp_reade(op,buf,blen,to,opts)
UDP		*op ;
char		buf[] ;
int		blen ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = uc_reade(op->fd,buf,blen,to,opts) ;

	return rs ;
}


int udp_recve(op,buf,blen,flags,to,opts)
UDP		*op ;
char		buf[] ;
int		blen ;
int		flags ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = uc_recve(op->fd,buf,blen,flags,to,opts) ;

	return rs ;
}


int udp_recvfrome(op,buf,blen,flags,sap,salenp,to,opts)
UDP		*op ;
char		buf[] ;
int		blen ;
int		flags ;
void		*sap ;
int		*salenp ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = uc_recvfrome(op->fd,buf,blen,flags,sap,salenp,to,opts) ;

	return rs ;
}


int udp_recvmsge(op,msgp,flags,to,opts)
UDP		*op ;
struct msghdr	*msgp ;
int		flags ;
int		to, opts ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = uc_recvmsge(op->fd,msgp,flags,to,opts) ;

	return rs ;
}


int udp_write(op,buf,blen)
UDP		*op ;
const char	buf[] ;
int		blen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = uc_writen(op->fd,((void *) buf),blen) ;

	return rs ;
}


int udp_send(op,buf,blen,flags)
UDP		*op ;
const char	buf[] ;
int		blen ;
int		flags ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = u_send(op->fd,buf,blen,flags) ;

	return rs ;
}


int udp_sendto(op,buf,blen,flags,sap,salen)
UDP		*op ;
const char	buf[] ;
int		blen ;
int		flags ;
void		*sap ;
int		salen ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = u_sendto(op->fd,buf,blen,flags,sap,salen) ;

	return rs ;
}


int udp_sendmsg(op,msgp,flags)
UDP		*op ;
struct msghdr	*msgp ;
int		flags ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = u_sendmsg(op->fd,msgp,flags) ;

	return rs ;
}


/* shutdown */
int udp_shutdown(op,cmd)
UDP		*op ;
int		cmd ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs = u_shutdown(op->fd,cmd) ;

	return rs ;
}
/* end subroutine (udp_shutdown) */


/* close the connection */
int udp_close(op)
UDP		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UDP_MAGIC) return SR_NOTOPEN ;

	rs1 = u_close(op->fd) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (udp_close) */


/* private subroutines */


