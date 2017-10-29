/* srventry */

/* build up a server entry piece-meal as it were */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1996-09-01, David Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object is used to create a server entry and to
	populate aspects of it with different operations on the
	object.  This object is used in "server" types of programs.
	This object is usually created from elements taken from the
	parsing of a server file.


*******************************************************************************/


#define	SRVENTRY_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"srvtab.h"
#include	"srventry.h"


/* local object defines */

#undef	BUFLEN
#define	BUFLEN		(10 * MAXPATHLEN)



/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;


/* forward references */

static int	process(varsub *,char *,SRVENTRY_ARGS *,char **) ;
static int	expand() ;

static void	freeit(char **) ;


/* external variables */


/* exported subroutines */


int srventry_start(sep)
SRVENTRY	*sep ;
{


	memset(sep,0,sizeof(SRVENTRY)) ;
	sep->program = NULL ;
	sep->srvargs = NULL ;
	sep->username = NULL ;
	sep->groupname = NULL ;
	sep->options = NULL ;
	sep->access = NULL ;

	return SR_OK ;
}
/* end subroutine (srventry_init) */


int srventry_finish(sep)
SRVENTRY	*sep ;
{


	if (sep->program != NULL)
	    free(sep->program) ;

	if (sep->srvargs != NULL)
	    free(sep->srvargs) ;

	if (sep->username != NULL)
	    free(sep->username) ;

	if (sep->groupname != NULL)
	    free(sep->groupname) ;

	if (sep->options != NULL)
	    free(sep->options) ;

	if (sep->access != NULL)
	    free(sep->access) ;

	return SR_OK ;
}
/* end subroutine (srventry_free) */


/* process server entry */
int srventry_process(sep,ssp,envv,step,esap)
SRVENTRY	*sep ;
varsub		*ssp ;
char		**envv ;
SRVTAB_ENTRY	*step ;
SRVENTRY_ARGS	*esap ;
{


#if	CF_DEBUGS
	debugprintf("srventry_process: entered\n") ;
#endif


/* pop them */

	if (step->program != NULL)
	    process(ssp,step->program,esap,&sep->program) ;

#if	CF_DEBUGS
	debugprintf("srventry_process: svcarg=>%s<\n",
	    step->args) ;
#endif

	if (step->args != NULL) {

#if	CF_DEBUGS
	    debugprintf("srventry_process: processing srvargs=>%s<\n",
	        step->args) ;
#endif

	    process(ssp,step->args,esap,&sep->srvargs) ;

#if	CF_DEBUGS
	    debugprintf("srventry_process: srvargs=>%s<\n",
	        sep->srvargs) ;
#endif

	}

	if (step->username != NULL)
	    process(ssp,step->username,esap,&sep->username) ;

	if (step->groupname != NULL)
	    process(ssp,step->groupname,esap,&sep->groupname) ;

	if (step->options != NULL)
	    process(ssp,step->options,esap,&sep->options) ;

	if (step->access != NULL)
	    process(ssp,step->access,esap,&sep->access) ;

	return SR_OK ;
}
/* end subroutine (srventry_process) */


int srventry_addprogram(sep,program)
SRVENTRY	*sep ;
char		program[] ;
{


	sep->program = mallocstr(program) ;

	return ((sep->program != NULL) ? SR_OK : SR_NOMEM) ;
}


int srventry_addsrvargs(sep,srvargs)
SRVENTRY	*sep ;
char		srvargs[] ;
{


	sep->srvargs = mallocstr(srvargs) ;

	return ((sep->srvargs != NULL) ? SR_OK : SR_NOMEM) ;
}


int srventry_addusername(sep,username)
SRVENTRY	*sep ;
char		username[] ;
{


	sep->username = mallocstr(username) ;

	return ((sep->username != NULL) ? SR_OK : SR_NOMEM) ;
}


int srventry_addgroupname(sep,groupname)
SRVENTRY	*sep ;
char		groupname[] ;
{


	sep->groupname = mallocstr(groupname) ;

	return ((sep->groupname != NULL) ? SR_OK : SR_NOMEM) ;
}


int srventry_addoptions(sep,options)
SRVENTRY	*sep ;
char		options[] ;
{


	sep->options = mallocstr(options) ;

	return ((sep->options != NULL) ? SR_OK : SR_NOMEM) ;
}
/* end subroutine (srventry_addoptions) */


/* local subroutines */


static int process(vsp,inbuf,esap,opp)
varsub		*vsp ;
char		inbuf[] ;
SRVENTRY_ARGS	*esap ;
char		**opp ;
{
	int	rs ;

	int	vlen, elen ;
	int	fl ;

	char	vbuf[BUFLEN + 1] ;
	char	ebuf[BUFLEN + 1] ;
	char	*fp ;


#if	CF_DEBUGS
	debugprintf("srventry/process: entered\n") ;
#endif

	*opp = NULL ;
	rs = varsub_expand(vsp, vbuf,BUFLEN, inbuf,-1) ;
	vlen = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("srventry/process: vlen=%d\n",vlen) ;
#endif

	elen = expand(vbuf,vlen,esap,ebuf,BUFLEN) ;
	if (elen < 0)
	    rs = SR_OVERFLOW ;

#if	CF_DEBUGS
	debugprintf("srventry/process: elen=%d\n",elen) ;
#endif

	if (rs >= 0) {

	    fl = sfshrink(ebuf,elen,&fp) ;

	    *opp = mallocstrw(fp,fl) ;

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("srventry/process: ret rs=%d fl=%u\n",rs,fl) ;
#endif

	return (fl >= 0) ? rs : fl ;
}
/* end subroutine (process) */


/* expand out a program argument with the substitution parameters */

/*
#	The following substitutions are made on command strings :

#		%V	Directory Watcher version string
#		%S	search name
#		%R	program root
#		%N	machine nodename
#		%D	machine DNS domain
#		%H	machine hostname
#		%U	server username

#		%P	socket peername (if available)
#		%I	ident or network name (if available)

#		%s	service
#		%a	service arguments (if available)
#		%h	originating host (if known)
#		%u	network username (if known)
#		%p	network password (if known)
#
*/

static int expand(buf,len,esap,rbuf,rlen)
char		rbuf[], buf[] ;
int		rlen, len ;
SRVENTRY_ARGS	*esap ;
{
	int	elen = 0, sl ;

	char	hostbuf[MAXHOSTNAMELEN + 1] ;
	char	*rbp = rbuf ;
	char	*bp = buf ;
	char	*cp ;


#if	CF_DEBUGS
	    debugprintf("srventry/expand: entered >%W<\n",buf,len) ;
#endif

#if	CF_DEBUGS
	    if (buf == NULL)
	        debugprintf("srventry/expand: buf is NULL\n") ;

	    if (rbuf == NULL)
	        debugprintf("srventry/expand: rbuf is NULL\n") ;

#endif /* CF_DEBUGS */

	rbuf[0] = '\0' ;
	if (len == 0)
	    return 0 ;

	if (len < 0)
	    len = strlen(buf) ;

#if	CF_DEBUGS
	    debugprintf("srventry/expand: top of while\n") ;
#endif

	rlen -= 1 ;			/* reserve for zero terminator */
	while ((len > 0) && (elen < rlen)) {

#if	CF_DEBUGS
	        debugprintf("srventry/expand: switching on >%c<\n",*bp) ;
#endif

	    switch ((int) *bp) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) 
			return elen ;

	        sl = 0 ;
	        switch ((int) *bp) {

	        case 'V':
	            cp = esap->version ;
	            sl = strlen(cp) ;

	            break ;

	        case 'S':
	            cp = esap->searchname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'R':
	            cp = esap->programroot ;
	            sl = strlen(cp) ;

	            break ;

	        case 'N':
	            cp = esap->nodename ;
	            sl = strlen(cp) ;

	            break ;

	        case 'D':
	            cp = esap->domainname ;
	            sl = strlen(cp) ;

	            break ;

	        case 'H':
	            sl = -1 ;
	            if (esap->hostname == NULL) {

	                cp = hostbuf ;
	                sl = bufprintf(hostbuf,MAXHOSTNAMELEN,"%s.%s",
	                    esap->nodename,esap->domainname) ;

	            } else
	                cp = esap->hostname ;

	            if (sl < 0)
	                sl = strlen(cp) ;

	            break ;

		case 'U':
	            cp = esap->username ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

		    break ;

	        case 'P':
	            cp = esap->peername ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

#if	CF_DEBUGS
	                debugprintf("srventry/expand: peername >%s<\n",
				cp) ;
#endif

	            break ;

	        case 's':
	            cp = esap->service ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'a':

#if	CF_DEBUGS
	            if (g.debuglevel > 2)
	                debugprintf("srventry/expand: svcargs >%s<\n",
	                    esap->svcargs) ;
#endif
	            cp = esap->svcargs ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'h':
	            cp = esap->nethost ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'u':
	            cp = esap->netuser ;
	            if (cp != NULL)
	                sl = strlen(cp) ;

	            break ;

	        case 'p':
	            cp = esap->netpass ;
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

#if	CF_DEBUGS
	        debugprintf("srventry/expand: bottom while\n") ;
#endif

	} /* end while */

#if	CF_DEBUGS
	    debugprintf("srventry/expand: normal exit >%W<\n",rbuf,elen) ;
#endif

	rbuf[elen] = '\0' ;
	return elen ;
}
/* end subroutine (expand) */


static void freeit(pp)
char	**pp ;
{


	if (*pp != NULL) {

	    free(*pp) ;

	    *pp = NULL ;
	}
}
/* end subroutine (freeit) */



