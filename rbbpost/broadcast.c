/* broadcast */


#define	CF_DEBUG	1		/* run-time debugging */
#define	CF_QUEUE	0


/* revision history:

	= 1995-03-01, David A­D­ Morano
        I made this from scratch to handle the posting of articles to
        remote machines.

	= 1998-02-09, David A­D­ Morano
	This was adjusted to replace the 'path' object w/ 'retpath'.

*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine posts the articles that we are given to the remote
        machines in our little cluster (whatever).


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<pcsconf.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecitem.h>
#include	<mailmsg.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"ng.h"
#include	"bbhosts.h"
#include	"retpath.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#undef	BUFLEN
#define	BUFLEN		100


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	getchostname(const char *,char *) ;

extern int	pcsngdir(const char *,char *,const char *,const char *) ;
extern int	forward(struct proginfo *,
			struct article *,const char *,const char *) ;

extern int	proglog_printf(PROGINFO *,cchar *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* forward references */

static int	isourself(struct proginfo *,BBHOSTS *,const char *) ;


/* exported subroutines */


int broadcast(pip,aqp,bhp,bnp)
struct proginfo	*pip ;
vecitem		*aqp ;
BBHOSTS		*bhp, *bnp ;
{
	struct ustat		sb ;

	struct newsgroup	*ngp ;

	struct article		*ap ;

	dev_t	device ;

	uino_t	inode ;

	int	rs = SR_OK ;
	int	i, j ;
	int	sl ;
	int	copies = 0 ;
	int	narticles = 0 ;

	const char	*pathname ;
	const char	*cp, *hp ;

	char	hostname[MAXHOSTNAMELEN + 1] ;
	char	tmphostname[MAXHOSTNAMELEN + 1] ;
	char	afname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 2] ;
	char	*bp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("broadcast: entered\n") ;
#endif

/* create something we are going to call a "pathname" */

	pathname = pip->mailhost ;
	cp = NULL ;
	if (pip->uucpnode != NULL)
	    cp = pip->uucpnode ;

	if (cp == NULL)
	    cp = pip->mailnode ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("broadcast: cp=%s orgdomain=%s\n",
		cp,pip->orgdomainname) ;
#endif

#ifdef	COMMENT
	bufprintf(tmphostname,MAXPATHLEN,"%s.%s",cp,pip->orgdomainname) ;
#else
	{
	    const char	*dn = pip->orgdomainname ;
	    if (dn == NULL) dn = pip->domainname ;
	    snsds(tmphostname,MAXPATHLEN,cp,dn) ;
	}
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("broadcast: getchostname=%s\n",tmphostname) ;
#endif

	if (getchostname(tmphostname,hostname) >= 0) {
	    pathname = cp ;
	} else
	    pathname = tmphostname ;

/* loop through all of the articles that we have queued up */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("broadcast: looping through articles \n") ;
#endif

	for (i = 0 ; vecitem_get(aqp,i,&ap) >= 0 ; i += 1) {
	    if (ap == NULL) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        debugprintf("broadcast: on article %d, articleid=%s\n",
	            i,ap->articleid) ;

	        debugprintf("broadcast: article ngdir=%s\n",ap->ngdir) ;
	    }
#endif /* CF_DEBUG */


/* first, link this article to the other newsgroups that it should go to */

	    sl = mkpath2(afname,pip->newsdname,ap->ngdir) ;

	    bp = afname + sl ;

	    if (u_stat(afname,&sb) < 0) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("broadcast: got a good stat() on the NGDIR\n") ;
#endif

	    device = sb.st_dev ;
	    inode = sb.st_ino ;

/* make the fill filename of the article */

#ifdef	COMMENT
	    bp = strwcpy(bp,"/",1) ;
#else
	    *bp++ = '/' ;
#endif

	    bp = strwcpy(bp,ap->af,-1) ;


/* OK, loop through the newsgroups that it is supposed to go to */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("broadcast: top of for where it goes to\n") ;
#endif

	    for (j = 0 ; ng_get(&ap->ngs,j,&ngp) >= 0 ; j += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: back from ng_get, ngp=%p\n",
	                ngp) ;
#endif

	        if (ngp == NULL) continue ;

	        if ((ngp->name == NULL) || (ngp->name[0] == '\0'))
	            continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: good name \n") ;
#endif

	        if (ngp->dir == NULL) {
	            char	ngdir[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("broadcast: NULL dir\n") ;
#endif

	            rs = pcsngdir(pip->pr,ngdir,pip->newsdname,ngp->name) ;

		    if (rs >= 0)
	                ngp->dir = mallocstr(ngdir) ;

	        } /* end if */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: NULL dir test\n") ;
#endif

	        if ((ngp->dir == NULL) || (ngp->dir[0] == '\0'))
	            continue ;

/* construct the path to the other newsgroup directory file to be created */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: construct it\n") ;
#endif

	        bp = strwcpy(tmpfname,pip->newsdname,MAXPATHLEN) ;

	        bp = strwcpy(bp,"/",MIN(1,(MAXPATHLEN - (bp - tmpfname)))) ;

	        bp = strwcpy(bp,ngp->dir,MAXPATHLEN) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: about to stat\n") ;
#endif

	        if ((u_stat(tmpfname,&sb) < 0) ||
	            (! S_ISDIR(sb.st_mode))) continue ;

/* do not put it where it already is ! */

	        if ((device == sb.st_dev) && (inode == sb.st_ino))
	            continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 0)
	            debugprintf("broadcast: we got one (another)\n") ;
#endif

	        bp = strwcpy(bp,"/",MIN(1,(MAXPATHLEN - (bp - tmpfname)))) ;

	        bp = strwcpy(bp,ap->af,MAXPATHLEN + 1 - (bp - tmpfname)) ;

	        if ((bp - tmpfname) > MAXPATHLEN)
	            continue ;

/* do the deed ! */

	        if (device == sb.st_dev) {
	            u_link(afname,tmpfname) ;
	        } else
	            u_symlink(afname,tmpfname) ;

	    } /* end for (linking article to other newsgroups) */

/* OK, now we forward this article to the other BB hosts on the network */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("broadcast: forwarding\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("broadcast: adding outgoing path\n") ;
#endif

	    retpath_add(&ap->path,pathname,-1) ;

/* forward this article to each of the hosts on our forward list */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("broadcast: looping through forward hosts\n") ;
#endif

	    for (j = 0 ; bbhosts_get(bhp,j,&hp) >= 0 ; j += 1) {
	        if (hp == NULL) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: host is us ?\n") ;
#endif

	        if (isourself(pip,bnp,hp)) 
			continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: already sent there ?\n") ;
#endif

	        if (retpath_search(&ap->path,hp,&cp) >= 0)
	            continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1) {
	            debugprintf("broadcast: forwarding to host=%s\n",hp) ;
	            debugprintf("broadcast: afname=%s\n",afname) ;
	        }
#endif /* CF_DEBUG */

	        rs = forward(pip,ap,afname,hp) ;

		if (rs >= 0)
	            copies += 1 ;

#if	CF_QUEUE
	        if (rs < 0)
	            (void) queue(ap,afname,hp) ;
#endif /* CF_QUEUE */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("broadcast: bottom of loop\n") ;
#endif

		if (rs < 0) break ;
	    } /* end for (forwarding this article) */

	    narticles += 1 ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("broadcast: bottom loop\n") ;
#endif

	    if (rs < 0) break ;
	} /* end for (processing article messages) */

	if (pip->open.logfile) {
	    proglog_printf(pip,"articles live=%d forwarded=%d\n",
	        narticles,copies) ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("broadcast: exiting narticles=%d\n",narticles) ;
#endif

	return (rs >= 0) ? copies : rs ;
}
/* end subroutine (broadcast) */


/* local subroutines */


/* is the supplied name a synonym for us? */
static int isourself(pip,bnp,name)
struct proginfo	*pip ;
BBHOSTS		*bnp ;
const char	name[] ;
{
	const char	*cp ;

	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;


	domainname[0] = '\0' ;
	if ((cp = strchr(name,'.')) != NULL) {

	    strwcpy(nodename,name,cp - name) ;

	    strcpy(domainname,cp) ;

	} else
	    strcpy(nodename,name) ;


	if ((pip->mailnode != NULL) && (strcmp(pip->mailnode,nodename) == 0)) 
		return YES ;

	if ((pip->uucpnode != NULL) && (strcmp(pip->uucpnode,nodename) == 0)) 
		return YES ;

	if ((pip->mailhost != NULL) && (strcmp(pip->mailhost,name) == 0)) 
		return YES ;

	if ((pip->uucphost != NULL) && (strcmp(pip->uucphost,name) == 0)) 
		return YES ;

	if (pip->userhost != NULL) {

	    if (strcmp(pip->userhost,name) == 0) 
		return YES ;

	    if ((cp = strchr(pip->userhost,'.')) != NULL) {

	        if (strncmp(pip->userhost,nodename,cp - pip->userhost) == 0)
	            return YES ;

	    }
	}

	if (bnp == NULL) 
		return NO ;

	if (bbhosts_find(bnp,name) >= 0)
	    return YES ;

	return NO ;
}
/* end subroutine (isourself) */



