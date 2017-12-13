/* opensvc_daytime */

/* LOCAL facility open-service (daytime) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_GETORGCODE	1		/* use 'localgetorgcode(3dam)' */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_daytime(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<nulstr.h>
#include	<nistinfo.h>
#include	<localmisc.h>

#include	"opensvc_daytime.h"
#include	"defs.h"


/* local defines */

#ifndef	ORGCODE
#define	ORGCODE		"X-local"
#endif

#define	OCFNAME		"etc/orgcode"

#define	OCBUFLEN	40


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	localgetorgcode(const char *,char *,int,const char *) ;
extern int	readfileline(char *,int,const char *) ;
extern int	isNotPresent(int) ;

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_nist(time_t,struct nistinfo *,char *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opensvc_daytime(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	struct nistinfo	ni ;
	const time_t	daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		rs1 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		sl ;
	const char	*sp = NULL ;
	const char	*orgcode = NULL ;
	char		ntbuf[NISTINFO_BUFLEN+2] ;
	char		ocbuf[OCBUFLEN+1] ;
	char		*bp ;

	ocbuf[0] = '\0' ;

/* were we given an org-code? */

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc >= 2) && (argv[1] != '\0')) {
		orgcode = argv[1] ;
	    }
	}

/* find our org-code (if we have one specified) */

#if	CF_GETORGCODE
	if ((orgcode == NULL) || (orgcode[0] == '\0')) {
	    rs1 = localgetorgcode(pr,ocbuf,OCBUFLEN,"-") ;
	    if (rs1 >= 0) orgcode = ocbuf ;
	    if ((rs1 < 0) && (! isNotPresent(rs1))) rs = rs1 ;
	}
#else /* CF_GETORGCODE */
	if ((orgcode == NULL) || (orgcode[0] == '\0')) {
	    char	ocfname[MAXPATHLEN+1] ;
	    rs = mkpath2(ocfname,pr,OCFNAME) ;
	    if (rs >= 0) {
	        rs1 = readfileline(ocbuf,OCBUFLEN,ocfname) ;
	        if (rs1 >= 0) orgcode = ocbuf ;
	        if ((rs1 < 0) && (! isNotPresent(rs1))) rs = rs1 ;
	    }
	}
#endif /* CF_GETORGCODE */

/* default org-code */

	if ((orgcode == NULL) || (orgcode[0] == '\0')) orgcode = ORGCODE ;

/* formulate the time string */

	if (rs >= 0) {
	    memset(&ni,0,sizeof(struct nistinfo)) ;

	    if (orgcode != NULL) {
	        strwcpy(ni.org,orgcode,NISTINFO_ORGLEN) ;
	    }
	    bp = ntbuf ;
	    sp = timestr_nist(daytime,&ni,ntbuf) ;

	    sl = strlen(sp) ;

	    bp[sl++] = '\n' ;
	    bp[sl] = '\0' ;
	}

/* write everything out */

	if (rs >= 0) {
	    if ((rs = u_pipe(pipes)) >= 0) {
	        const int	wfd = pipes[1] ;
	        fd = pipes[0] ;

	        rs = u_write(wfd,sp,sl) ;

	        u_close(wfd) ;
	        if (rs < 0) u_close(fd) ;
	    } /* end if (u_pipe) */
	} /* end if (ok) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_daytime) */


