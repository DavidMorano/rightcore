/* progexpand */

/* expand out some per program parameters */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine expands out some per program (the daemon program)
        parameters into the configuration strings. Actually this little
        subroutine is used by many programs.


#	The following substitutions are made on command strings:

#		%V	version string
#		%B	programroot basename
#		%P	program name
#		%S	program search name
#		%N	machine node name
#		%D	machine INET domain name
#		%H	machine INET host name
#		%U	invoking username
#		%G	invoking groupname
#		%R	program root directory
#


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;


/* external variables */


/* exported subroutines */


int progexpand(PROGINFO *pip,char *rbuf,int rlen,cchar *sbuf,int slen)
{
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs = SR_OK ;
	int		len ;
	int		cl ;
	int		ch ;
	int		elen = 0 ;
	cchar		*bp = sbuf ;
	cchar		*cp ;
	char		hbuf[MAXHOSTNAMELEN + 1] ;
	char		*rbp = rbuf ;

#if	CF_DEBUGS
	debugprintf("expnader: ent slen=%d rlen=%d\n",slen,rlen) ;
#endif

	if (pip == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	len = strnlen(sbuf,slen) ;

#if	CF_DEBUGS
	debugprintf("expnader: sbuf=>%t<\n",sbuf,len) ;
#endif

	while ((len > 0) && (elen < rlen)) {
	    ch = MKCHAR(*bp) ;
	    switch (ch) {
	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) break ;
		cl = 0 ;
	        switch (ch) {
	        case 'V':
	            cp = pip->version ;
	            cl = strlen(cp) ;
	            break ;
#if	defined(DEFS_ROOTNAME) && (DEFS_ROOTNAME > 0)
	        case 'B':
		    if ((cp = pip->rootname) != NULL) {
	            	cl = strlen(cp) ;
		    }
	            break ;
#endif /* P_RCPMUXD */
	        case 'P':
	            cp = pip->progname ;
	            cl = strlen(cp) ;
	            break ;
	        case 'S':
		    if ((cp = pip->searchname) != NULL) {
	            	cl = strlen(cp) ;
		    }
	            break ;
	        case 'N':
	            cp = pip->nodename ;
	            cl = strlen(cp) ;
	            break ;
	        case 'D':
	            cp = pip->domainname ;
	            cl = strlen(cp) ;
	            break ;
	        case 'H':
	            cp = hbuf ;
	            rs = snsds(hbuf,hlen,pip->nodename,pip->domainname) ;
		    cl = rs ;
	            break ;
/* handle the expansion of our program root */
	        case 'R':
	            if ((cp = pip->pr) != NULL) {
	                cl = strlen(cp) ;
		    }
	            break ;
	        case 'U':
		    if ((cp = pip->username) != NULL) {
			cl = strlen(cp) ;
		    }
	            break ;
	        case 'G':
		    if ((cp = pip->groupname) != NULL) {
			cl = strlen(cp) ;
		    }
	            break ;
	        default:
	            cp = bp ;
	            cl = 1 ;
		    break ;
	        } /* end switch */
	        bp += 1 ;
	        len -= 1 ;
	        if ((elen + cl) > rlen) {
		    rs = SR_OVERFLOW ;
	            break ;
		}
	        if (cl > 0) strncpy(rbp,cp,cl) ;
	        rbp += cl ;
	        elen += cl ;
	        break ;
	    default:
	        *rbp++ = *bp++ ;
	        elen += 1 ;
	        len -= 1 ;
		break ;
	    } /* end switch */
	} /* end while */

	if ((rs >= 0) && (elen >= rlen)) {
	    rs = SR_OVERFLOW ;
	}

	rbuf[elen] = '\0' ;

#if	CF_DEBUGS
	debugprintf("progexpand: ret rs=%d elen=%d\n",rs,elen) ;
	if (rs >= 0)
	    debugprintf("progexpand: ebuf=>%t<\n",rbuf,elen) ;
#endif

	return (rs >= 0) ? elen : rs ;
}
/* end subroutine (progexpand) */


