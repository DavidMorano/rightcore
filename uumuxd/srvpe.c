/* srvpe */

/* process server file entries */
/* version %I% last modified %G% */


#define	CF_DEBUG	1
#define	CF_DEBUG2	0


/* revision history:

	= 91/09/10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	These subroutines (object module) are used to expand out and
	otherwise process the server file entries when we need one of
	them.

	Expansion is intentionally done as late as possible so that we
	can take advantage of the latest bindings.  Talk to Professor
	Dave Cheriton over at Stanford University if you do not agree !! :-)


*****************************************************************************/


#define	SRVPE_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<limits.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<baops.h>
#include	<srvtab.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"srventry.h"
#include	"srvpe.h"

#if	CF_DEBUG
#include	"config.h"
#include	"defs.h"
#endif


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	(4 * MAXPATHLEN)
#endif


/* external subroutines */


/* externals variables */

#if	CF_DEBUG
extern struct proginfo	g ;
#endif


/* forward references */

static int	expand(char *,int,SRVPE *,SRVPE_ARGS *,char *,int) ;


/* local global variables */


/* local structures */






int srvpe_init(pbp,version,rn,nn,dn)
SRVPE	*pbp ;
char	version[], rn[], nn[], dn[] ;
{
	int	hlen ;

	char	hostname[MAXHOSTNAMELEN + 1] ;


	pbp->version = version ;
	pbp->programroot = rn ;
	pbp->nodename = nn ;
	pbp->domainname = dn ;
	pbp->hostname = NULL ;
	if ((dn != NULL) && (dn[0] != '\0')) {

	    hlen = bufprintf(hostname,MAXHOSTNAMELEN,"%s.%s",
	        nn,dn) ;

	    pbp->hostname = mallocstrw(hostname,hlen) ;

	} else
	    pbp->hostname = mallocstrw(nn,-1) ;

	return SR_OK ;
}
/* end subroutine (srvpe_init) */


int srvpe_free(pbp)
SRVPE	*pbp ;
{


	if (pbp->hostname != NULL)
		free(pbp->hostname) ;

	pbp->version = NULL ;
	return SR_OK ;
}
/* end subroutine (srvpe_free) */


/* expand out a server entry */

/*
	char	*program ;
	char	*srvargs ;
	char	*username ;
	char	*groupname ;
	char	*options ;
*/

int srvpe_sub(pbp,pdp,sep,psp)
SRVPE		*pbp ;
SRVPE_ARGS	*pdp ;
SRVTAB_ENT	*sep ;
SRVENTRY	*psp ;
{
	int	elen ;

	char	ebuf[BUFLEN + 1] ;


#ifdef	COMMENT
	if (pbp->version == NULL)
		return SR_FAULT ;
#endif

	if (sep->program != NULL) {

	    elen = expand(sep->program,-1,pbp,pdp,ebuf,BUFLEN) ;

	    srventry_addprogram(psp,ebuf,elen) ;

	}

	if (sep->args != NULL) {

	    elen = expand(sep->args,-1,pbp,pdp,ebuf,BUFLEN) ;

	    srventry_addsrvargs(psp,ebuf,elen) ;

	}

#ifdef	COMMENT

	if (sep->username != NULL) {

	    elen = expand(sep->username,-1,pbp,pdp,ebuf,BUFLEN) ;

	    srventry_addusername(psp,ebuf,elen) ;

	}

	if (sep->groupname != NULL) {

	    elen = expand(sep->groupname,-1,pbp,pdp,ebuf,BUFLEN) ;

	    srventry_addgroupname(psp,ebuf,elen) ;

	}

#endif /* COMMENT */

	if (sep->options != NULL) {

	    elen = expand(sep->options,-1,pbp,pdp,ebuf,BUFLEN) ;

	    srventry_addoptions(psp,ebuf,elen) ;

	}

	return SR_OK ;
}
/* end subroutine (srvpe_sub) */



/* INTERNAL SUBROUTINES */



/* expand out a program argument with the substitution parameters */

/*
#	The following substitutions are made on command strings :

#		%V	Directory Watcher version string
#		%R	program root
#		%N	machine nodename
#		%D	machine DNS domain
#		%H	machine hostname

#		%P	socket peername (if available)
#		%s	service
#		%a	service arguments (if available)
#		%h	originating host (if known)
#		%u	network username (if known)
#		%p	netwotk password (if known)
#
*/

static int expand(buf,len,pbp,pdp,rbuf,rlen)
char			rbuf[], buf[] ;
int			rlen, len ;
SRVPE			*pbp ;
SRVPE_ARGS		*pdp ;
{
	int	elen = 0, sl ;

	char	hostbuf[MAXHOSTNAMELEN + 1] ;
	char	*rbp = rbuf ;
	char	*bp = buf ;
	char	*cp ;


#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("srvpe/expand: entered >%W<\n",buf,len) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 2) {

	    if (buf == NULL)
	        debugprintf("srvpe/expand: buf is NULL\n") ;

	    if (rbuf == NULL)
	        debugprintf("srvpe/expand: rbuf is NULL\n") ;

	}
#endif
	rbuf[0] = '\0' ;
	if (len == 0)
	    return 0 ;

	if (len < 0)
	    len = strlen(buf) ;

#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("srvpe/expand: top of while\n") ;
#endif

	rlen -= 1 ;			/* reserve for zero terminator */
	while ((len > 0) && (elen < rlen)) {

#if	CF_DEBUG2
	    if (g.debuglevel > 2)
	        debugprintf("srvpe/expand: switching on >%c<\n",*bp) ;
#endif

	    switch ((int) *bp) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) return elen ;

	        sl = 0 ;
	        switch ((int) *bp) {

	        case 'V':
	            cp = pbp->version ;
	            sl = strlen(cp) ;

	            break ;

	        case 'R':
	            cp = pbp->programroot ;
	            sl = strlen(cp) ;

	            break ;

	        case 'N':
	            cp = pbp->nodename ;
	            sl = strlen(cp) ;

	            break ;

	        case 'D':
	            cp = pbp->domainname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'H':
	            cp = pbp->hostname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'P':
	            cp = pdp->peername ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 's':
	            cp = pdp->service ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'a':

#if	CF_DEBUG2
	            if (g.debuglevel > 2)
	                debugprintf("srvpe/expand: srvargs >%s<\n",pdp->srvargs) ;
#endif
	            cp = pdp->srvargs ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'h':
	            cp = pdp->nethost ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'u':
	            cp = pdp->netuser ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'p':
	            cp = pdp->netpass ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        default:
	            cp = bp ;
	            sl = 1 ;

	        } /* end switch */

	        bp += 1 ;
	        len -= 1 ;

	        if ((elen + sl) > rlen)
	            return BAD ;

	        strncpy(rbp,cp,sl) ;

	        rbp += sl ;
	        elen += sl ;
	        break ;

	    default:
	        *rbp++ = *bp++ ;
	        elen += 1 ;
	        len -= 1 ;

	    } /* end switch */

#if	CF_DEBUG2
	    if (g.debuglevel > 2)
	        debugprintf("srvpe/expand: bottom while\n") ;
#endif

	} /* end while */

#if	CF_DEBUG
	if (g.debuglevel > 2)
	    debugprintf("srvpe/expand: normal exit >%W<\n",rbuf,elen) ;
#endif

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */



