/* inputlogin */

/* handle a connect request for a service */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* non-switchable */
#define	CF_DEBUG	1		/* switchable */
#define	CF_NPRINTF	1		/* extra special debug print-outs */


/* revision history:

	= 1996-07-01, David A­D­ Morano

	This program was originally written.


	= 1998-07-01, David A­D­ Morano

	I added the ability to specify the "address_from"
	for the case when we add an envelope header to the message.


*/

/* Copyright © 1996,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine prompts for and gets the login name and password.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)
#define	NLOGINS		4
#define	NPASSWORDS	2
#define	DEBUGFILE	"/tmp/uucpd.deb"

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;

extern char	*strbasename(char *) ;
extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */

#if	CF_NPRINTF
static char	*strhex(const char *,char *) ;
#endif


/* local variables */

#if	CF_NPRINTF

static const char	hextable[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
} ;

#endif /* CF_NPRINTF */


/* exported subroutines */


int inputlogin(pip,cip,to,svcspec,slen,password,plen)
struct proginfo		*pip ;
struct clientinfo	*cip ;
int			to ;
char			svcspec[], password[] ;
int			slen, plen ;
{
	int	ifd = FD_STDIN, ofd = FD_STDOUT ;
	int	rs, i, j, cl, len ;

#if	CF_NPRINTF
	char	hexbuf[TIMEBUFLEN + 1] ;
#endif

	char	buf[BUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("inputlogin: entered, ifd=%d ofd=%d\n",ifd,ofd) ;
#endif

	svcspec[0] = '\0' ;
	password[0] = '\0' ;

	for (i = 0 ; i < NLOGINS ; i += 1) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("inputlogin: prompting i=%d\n",i) ;
#endif

/* issue the login prompt */

	    cp = LOGIN_PROMPT ;
	    cl = strlen(cp) ;

	    rs = uc_writen(ofd,cp,cl) ;

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s send >%W< rs=%d\n", 
		pip->logid,cp,cl,rs) ;
#endif

	    if (rs < 0)
	        break ;

/* pop off the login name and arguments */

	    buf[0] = '\0' ;
	    rs = getcodedtimed(ifd,buf,BUFLEN,to) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("inputlogin: getcodedtimed() rs=%d\n",rs) ;
#endif

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s input rs=%d svspec=%s\n",
		pip->logid,rs,strhex(buf,hexbuf)) ;
#endif

	    if (rs <= 0) {

		rs = SR_TIMEDOUT ;
	        break ;
	    }

	    len = rs ;
	    for (j = 0 ; j < len ; j += 1)
		buf[j] &= 0x7f ;

	    if (buf[len - 1] == '\n')
	        len -= 1 ;

	    buf[len] = '\0' ;
	    cl = sfshrink(buf,len,&cp) ;

	    if (cl > 0)
	        break ;

	} /* end for (looping on NLOGINS) */

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s svcspec i=%d rs=%d\n",
		pip->logid,i,rs) ;
#endif

	if (rs < 0)
	    goto badtrans ;

	if (i >= NLOGINS) {

	    rs = SR_NOENT ;
	    goto badtries ;
	}

	strwcpy(svcspec,cp,MIN(cl,slen)) ;

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s login=%s\n",
		pip->logid,svcspec) ;
#endif


/* handle password */

	for (i = 0 ; i < NPASSWORDS ; i += 1) {

/* issue the login prompt */

	    cp = PASSWORD_PROMPT ;
	    cl = strlen(cp) ;

	    rs = uc_writen(ofd,cp,cl) ;

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s send >%W< rs=%d\n", 
		pip->logid,cp,cl,rs) ;
#endif

	    if (rs < 0)
	        break ;

/* get the password */

	    buf[0] = '\0' ;
	    rs = getcodedtimed(ifd,buf,BUFLEN,to) ;

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s input rs=%d password=%s\n",
		pip->logid,rs,strhex(buf,hexbuf)) ;
#endif

	    if (rs <= 0) {

		rs = SR_TIMEDOUT ;
	        break ;
	    }

	    len = rs ;
	    for (j = 0 ; j < len ; j += 1)
		buf[j] &= 0x7f ;

	    if (buf[len - 1] == '\n')
	        len -= 1 ;

	    buf[len] = '\0' ;
	    cl = sfshrink(buf,len,&cp) ;

	    if (cl > 0)
	        break ;

	} /* end for (looping on NPASSWORDS) */

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s password i=%d rs=%d\n",
		pip->logid,i,rs) ;
#endif

	if (rs < 0)
		goto badtrans ;

	if (i >= NPASSWORDS) {

	    rs = SR_NOENT ;
	    goto badtries ;
	}

	strwcpy(password,cp,MIN(cl,plen)) ;

#if	CF_NPRINTF
	nprintf(DEBUGFILE,"%-14s password=%s\n",
		pip->logid,password) ;
#endif


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("inputlogin: login=%s password=%s\n",svcspec,password) ;
#endif

badtrans:
badtries:
	return rs ;
}
/* end subroutine (inputlogin) */


/* local subroutines */


#if	CF_NPRINTF

static char *strhex(s,buf)
const char	s[] ;
char		buf[] ;
{
	int	i, j ;


	j = 0 ;
	buf[j] = '\0' ;
	for (i = 0 ; s[i] && (i < 8) ; i += 1) {

		buf[j++] = ' ' ;
		buf[j++] = hextable[(s[i] >> 4) & 15] ;
		buf[j++] = hextable[s[i] & 15] ;

	} /* end for */

	buf[j] = '\0' ;
	return buf ;
}
/* end subroutine (strhex) */

#endif /* CF_NPRINTF */


