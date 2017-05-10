/* fspipe */

/* FileSystem Pipe */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-31, David A­D­ Morano

	This object module was written with code borrowed from previous
	subroutines that performed the substantially similar function.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We do something with a pipe here.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/conf.h>
#include	<sys/socket.h>
#include	<sys/utsname.h>
#include	<stropts.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"fspipe.h"


/* local defines */

#define	FSPIPE_MAGIC	0x98712365

#undef	LINELEN
#define	LINELEN		(MAXPATHLEN + NODENAMELEN + 50)


/* external subroutines */

extern int	matcasestr(const char **,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	fspipe_readfile(FSPIPE *,const char *) ;
static int	fspipe_freeall(FSPIPE *) ;
static int	fspipe_findentry(FSPIPE *,const char *,FSPIPE_ENT **) ;

static int	entry_init(FSPIPE_ENT *) ;
static int	entry_initlocal(FSPIPE_ENT *,int) ;
static int	entry_initinit(FSPIPE_ENT *,int) ;
static int	entry_free(FSPIPE_ENT *) ;
static int	entry_loadlocal(FSPIPE_ENT *,char *) ;
static int	entry_loadinet(FSPIPE_ENT *,char *) ;
static int	entry_openlocal(FSPIPE_ENT *,int) ;
static int	entry_openinet(FSPIPE_ENT *,int) ;

static void	freeit() ;


/* local variables */

static const char	*transports[] = {
	"local",
	"inet",
	NULL
} ;

#define	FSPIPE_TRANSUNIX	1
#define	FSPIPE_TRANSINET	2
#define	FSPIPE_TRANSLOCAL	3


/* exported subroutines */


int fspipe_open(pfp,fname)
FSPIPE		*pfp ;
const char	fname[] ;
{
	FSPIPE_ENT	*ep ;

	int	rs ;


	if (pfp == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;


	memset(pfp,0,sizeof(FSPIPE)) ;

	rs = vecitem_start(&pfp->e,4,0) ;
	if (rs < 0)
	    goto bad0 ;

#if	CF_DEBUGS
	debugprintf("fspipe_open: about to read file\n") ;
#endif

	rs = fspipe_readfile(pfp,fname) ;
	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUGS
	debugprintf("fspipe_open: openfile rs=%d\n",rs) ;
#endif


/* OK, now try and actually open one of these transports ! */

	rs = fspipe_findentry(pfp,"local",&ep) ;

#if	CF_DEBUGS
	debugprintf("fspipe_open: findentry rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    struct utsname	u ;


	    u_uname(&u) ;

	    if (strcmp(ep->p.t_local.nodename,u.nodename) == 0) {

	        rs = entry_openlocal(ep,(int) pfp->f.listen) ;

#if	CF_DEBUGS
	        debugprintf("fspipe_open: entry_openlocal rs=%d\n",rs) ;
#endif

	    } else
	        rs = SR_NOTSUP ;

	} /* end if (local) */


	if ((rs < 0) &&
	    ((rs = fspipe_findentry(pfp,"inet",&ep)) >= 0)) {

#if	CF_DEBUGS
	    debugprintf("fspipe_open: trying INET\n") ;
#endif

	    rs = entry_openinet(ep,(int) pfp->f.listen) ;

	} /* end if (inet) */

#if	CF_DEBUGS
	debugprintf("fspipe_open: open result rs=%d\n",rs) ;
#endif

/* we're out of here! */

	if (rs < 0)
	    goto bad1 ;

	fspipe_freeall(pfp) ;

	pfp->fd = rs ;
	pfp->magic = FSPIPE_MAGIC ;

ret0:
	return rs ;

/* bad control flow comes here */
bad1:
	fspipe_freeall(pfp) ;

bad0:
	goto ret0 ;
}
/* end subroutine (fspipe_open) */


/* close the "pipe" */
int fspipe_close(pfp)
FSPIPE		*pfp ;
{
	FSPIPE	*ep ;

	int	rs = SR_OK ;
	int	i ;


	if (pfp == NULL)
	    return SR_FAULT ;

	pfp->magic = 0 ;
	if (pfp->f.listen) {

	    for (i = 0 ; (rs = vecitem_get(&pfp->e,i,&ep)) >= 0 ; i += 1) {

	        if (ep == NULL) continue ;

	        if (ep->fd >= 0)
	            u_close(ep->fd) ;

	    } /* end for */

	    fspipe_freeall(pfp) ;

	} else
	    u_close(pfp->fd) ;

	return SR_OK ;
}
/* end subroutine (fspipe_close) */


/* open and listen to a fspipe */
int fspipe_openlisten(pfp,fname,oflags,perm)
FSPIPE		*pfp ;
const char	fname[] ;
int		oflags ;
int		perm ;
{
	FSPIPE_ENT	e ;

	int	rs ;
	int	f_openalready = FALSE ;


	if (pfp == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;

/* try and see if there is a server on this fspipe already ! */

	rs = fspipe_open(pfp,fname) ;

	if (rs >= 0)
	    f_openalready = TRUE ;

	(void) fspipe_close(pfp) ;

	if (f_openalready)
	    return SR_ALREADY ;

/* initialize the entry list */

	rs = vecitem_start(&pfp->e,4,0) ;

	if (rs < 0)
	    return rs ;

/* create the various sub-things to listen on and write the fspipe file */

	rs = entry_initlocal(&e,perm) ;
	if (rs < 0)
	    goto bad0 ;

	pfp->magic = FSPIPE_MAGIC ;

/* bad stuff */
bad0:
	return rs ;

bad1:
	fspipe_freeall(pfp) ;

	goto ret0 ;
}
/* end subroutine (fspipe_openlisten) */


/* accept connections on this fspipe */
int fspipe_accept(pfp)
FSPIPE		*pfp ;
{
	int	rs = SR_OK ;



	return rs ;
}
/* end subroutine (fspipe_accept) */


/* write to this fspipe */
int fspipe_write(pfp,buf,buflen)
FSPIPE		*pfp ;
const char	buf[] ;
int		buflen ;
{


	if (pfp == NULL)
	    return SR_NOTOPEN ;

	if (buf == NULL)
	    return SR_FAULT ;

	return u_write(pfp->fd,buf,buflen) ;
}
/* end subroutine (fspipe_write) */


int fspipe_read(pfp,buf,buflen)
FSPIPE		*pfp ;
char		buf[] ;
int		buflen ;
{


	if (pfp == NULL)
	    return SR_NOTOPEN ;

	if (buf == NULL)
	    return SR_FAULT ;

	return u_read(pfp->fd,buf,buflen) ;
}
/* end subroutine (fspipe_read) */


#ifdef	COMMENT

int fspipe_accepter(pfp,semp,sbp,intfunc,intarg)
PSPIPE	*pfp ;
VSSEM	*semp ;
VSSTAT	*sbp ;
void	(*intfunc)() ;
void	*intarg ;
{



}

#endif /* COMMENT */


/* private subroutines */


/* open the fspipe file and read into entry structures */
static int fspipe_readfile(pfp,fname)
FSPIPE		*pfp ;
const char	fname[] ;
{
	FSPIPE_ENT	e ;

	bfile	pfile, *pp = &pfile ;

	int	rs ;
	int	i ;
	int	len, len2 ;
	int	sl, sl2, sl3 ;
	int	val ;
	int	count = 0 ;

	char	linebuf[LINELEN + 1] ;
	char	*cp, *cp2, *cp3 ;


	if (pfp->f.listen)
	    rs = bopen(pp,fname,"rw",0666) ;

	else
	    rs = bopen(pp,fname,"r",0666) ;

	if (rs < 0)
	    goto bad0 ;

/* read the first line */

	len = breadline(pp,linebuf,LINELEN) ;

	rs = SR_NOTOPEN ;
	if (len <= 0)
	    goto bad1 ;

	rs = SR_BADFMT ;
	if (strncmp(linebuf,FSPIPE_MAGICSTR,FSPIPE_MAGICSTRLEN) != 0)
	    goto bad1 ;

/* this is the version string */

	len2 = len - FSPIPE_MAGICSTRLEN ;
	sl = nextfield((linebuf + FSPIPE_MAGICSTRLEN),len2,&cp) ;

	rs = cfdeci(cp,sl,&val) ;

	if ((rs < 0) || (val > FSPIPE_VERSION)) {

	    rs = SR_NOTSUP ;
	    goto bad1 ;
	}

	pfp->version = val ;

/* that is all we are interested in at this point with the general stuff */

/* read the rest of the lines */

	while ((len = breadline(pp,linebuf,LINELEN)) > 0) {

	    sl = sfshrink(linebuf,len,&cp) ;

	    if (sl <= 0) continue ;

	    if (cp[0] == '#') continue ;

	    entry_init(&e) ;

/* get the transport/protocol string */

	    sl = nextfield(cp,-1,&cp2) ;

	    cp3 = cp2 + sl ;
	    if ((cp = strchr(cp2,':')) != NULL) {

	        *cp++ = '\0' ;
	        sl -= (cp - cp2) ;
	        strwcpy(e.protocol,cp,MIN(sl,FSPIPE_PROTOLEN)) ;

	        sl = cp - cp2 - 1 ;
	    }

	    strwcpy(e.transport,cp2,sl) ;

/* get the transport argumements */

	    rs = SR_NOTSUP ;
	    if ((i = matcasestr(transports,e.transport,-1)) >= 0) {

	        switch (i) {

	        case FSPIPE_TRANSINET:
	            rs = entry_loadinet(&e,cp3) ;

	            break ;

	        case FSPIPE_TRANSLOCAL:
	            rs = entry_loadlocal(&e,cp3) ;

	            break ;

	        } /* end switch */

	    } /* end if (known transport) */

	    if (rs >= 0)
	        rs = vecitem_add(&pfp->e,&e,sizeof(FSPIPE_ENT)) ;

	    else
	        entry_free(&e) ;

	    if (rs >= 0)
	        count += 1 ;

	} /* end while (reading lines) */

	rs = (len < 0) ? len : count ;

bad1:
	bclose(pp) ;

ret0:
bad0:
	return rs ;
}
/* end subroutine (fspipe_readfile) */


/* find a transport entry by name */
static fspipe_findentry(pfp,name,epp)
FSPIPE		*pfp ;
const char	name[] ;
FSPIPE_ENT	**epp ;
{
	int	rs ;
	int	i ;


	for (i = 0 ; (rs = vecitem_get(&pfp->e,i,epp)) >= 0 ; i += 1) {

	    if ((*epp) == NULL) continue ;

	    if (strcmp((*epp)->transport,name) == 0)
	        break ;

	} /* end for */

	return ((rs >= 0) ? i : rs) ;
}
/* end subroutine (fspipe_findentry) */


/* free all of the resources associated with the transport entries */
static int fspipe_freeall(pfp)
FSPIPE		*pfp ;
{
	FSPIPE_ENT	*ep ;

	int	rs ;
	int	i ;


	for (i = 0 ; (rs = vecitem_get(&pfp->e,i,&ep)) >= 0 ; i += 1) {

	    if (ep == NULL) continue ;

	    entry_free(ep) ;

	} /* end for */

	vecitem_finish(&pfp->e) ;

	return SR_OK ;
}
/* end subroutine (fspipe_freeall) */


static int entry_init(ep)
FSPIPE_ENT	*ep ;
{


	memset(&ep->p,0,sizeof(union fspipe_param)) ;

	ep->type = -1 ;
	ep->transport[0] = '\0' ;
	ep->protocol[0] = '\0' ;
	ep->fd = -1 ;
	return SR_OK ;
}
/* end subroutine (entry_init) */


/* initialize an INET TCP transport */
static int entry_initinet(ep,perm)
FSPIPE_ENT	*ep ;
int		perm ;
{
	struct utsname	u ;

	struct protoent	*pep ;

	int	rs ;
	int	proto ;
	int	pipes[2] ;

	char	tmpfname[MAXPATHLEN + 2] ;


	memset(&ep->p,0,sizeof(union fspipe_param)) ;

/* get the dumb protocol (dumb because they want it and it is not reentrant) */

	pep = getprotobyname("tcp") ;

	if (pep != NULL)
	    proto = pep->p_proto ;

	else
	    proto = IPPROTO_TCP ;

	rs = u_socket(PF_INET,SOCK_STREAM,proto) ;
	if (rs < 0)
	    goto ret0 ;

/* try and push this */

	rs = u_ioctl(pipes[1],I_PUSH,"connld") ;
	if (rs < 0)
	    goto bad2 ;

/* attach the client end to the file created above */

	rs = uc_fattach(pipes[1],tmpfname) ;
	if (rs < 0)
	    goto bad2 ;

/* fill in this entry */

	ep->type = FSPIPE_TRANSLOCAL ;
	ep->fd = pipes[0] ;
	strwcpy(ep->transport,"local",FSPIPE_TRANSLEN) ;

	strwcpy(ep->protocol,"co",FSPIPE_PROTOLEN) ;

	u_uname(&u) ;		/* can't fail! */

	ep->p.t_local.nodename = mallocstr(u.nodename) ;

	ep->p.t_local.filepath = mallocstr(tmpfname) ;

	rs = SR_NOMEM ;
	if ((ep->p.t_local.nodename == NULL) || 
	    (ep->p.t_local.filepath == NULL))
	    goto bad4 ;

/* are we done yet ??? :-) */

/* we're out of here */
ret0:
	return rs ;

/* bad things come here */
bad4:
	freeit(ep->p.t_local.nodename) ;

	freeit(ep->p.t_local.filepath) ;

bad2:
	u_close(pipes[0]) ;

	u_close(pipes[1]) ;

bad1:
	u_unlink(tmpfname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (entry_initinet) */


/* initialize a local transport */
static int entry_initlocal(ep,perm)
FSPIPE_ENT	*ep ;
int		perm ;
{
	struct utsname	u ;

	int	rs ;
	int	pipes[2] ;

	const char	*template ;

	char	tmpfname[MAXPATHLEN + 2] ;


	memset(&ep->p,0,sizeof(union fspipe_param)) ;

	template = "/tmp/fspXXXXXXXXXXX" ;
	rs = mktmpfile(tmpfname,(perm & 0777),template) ;
	if (rs < 0)
	    goto bad0 ;

	rs = u_pipe(pipes) ;
	if (rs < 0)
	    goto bad1 ;

/* we'll use pipes[0] for the server end */

	rs = u_ioctl(pipes[1],I_PUSH,"connld") ;
	if (rs < 0)
	    goto bad2 ;

/* attach the client end to the file created above */

	rs = uc_fattach(pipes[1],tmpfname) ;
	if (rs < 0)
	    goto bad2 ;

/* fill in this entry */

	ep->type = FSPIPE_TRANSLOCAL ;
	ep->fd = pipes[0] ;
	strwcpy(ep->transport,"local",FSPIPE_TRANSLEN) ;

	strwcpy(ep->protocol,"co",FSPIPE_PROTOLEN) ;

	u_uname(&u) ;		/* can't fail ! */

	rs = SR_NOMEM ;
	ep->p.t_local.nodename = mallocstr(u.nodename) ;

	ep->p.t_local.filepath = mallocstr(tmpfname) ;

	if ((ep->p.t_local.nodename == NULL) || 
	    (ep->p.t_local.filepath == NULL))
	    goto bad4 ;

/* are we done yet ??? :-) */

/* we're out of here */
ret0:
	return rs ;

/* bad things come here */
bad4:
	freeit(ep->p.t_local.nodename) ;

	freeit(ep->p.t_local.filepath) ;

bad2:
	u_close(pipes[0]) ;

	u_close(pipes[1]) ;

bad1:
	u_unlink(tmpfname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (entry_initlocal) */


/* handle a LOCAL entry */
static int entry_loadlocal(ep,s)
FSPIPE_ENT	*ep ;
char		s[] ;
{
	int	sl ;
	int	size ;

	char	*cp, *cp2 ;


	sl = nextfield(s,-1,&cp) ;

	if (sl <= 0)
	    return BAD ;

	cp2 = cp ;
	if ((cp = strchr(cp2,':')) == NULL)
	    return BAD ;

	size = MIN((cp - cp2),NODENAMELEN) ;
	ep->p.t_local.nodename = mallocstrw(cp2,size) ;

	if (ep->p.t_local.nodename == NULL)
	    return BAD ;

	cp += 1 ;
	size = MIN(MAXPATHLEN,(sl - (cp - cp2))) ;
	ep->p.t_local.filepath = mallocstrw(cp,size) ;

	if (ep->p.t_local.filepath == NULL) {

	    free(ep->p.t_local.nodename) ;

	    return BAD ;
	}

	ep->type = FSPIPE_TRANSLOCAL ;
	return OK ;
}
/* end subroutine (entry_loadlocal) */


/* handle an INET entry */
static int entry_loadinet(ep,s)
FSPIPE_ENT	*ep ;
char		s[] ;
{
	int	rs = SR_OK ;
	int	sl ;
	int	size ;

	char	*cp, *cp2 ;


	sl = nextfield(s,-1,&cp) ;

	if (sl <= 0)
	    return BAD ;

	cp2 = cp ;
	if ((cp = strchr(cp2,':')) == NULL)
	    return BAD ;

	size = MIN((cp - cp2),MAXHOSTNAMELEN) ;
	ep->p.t_inet.hostname = mallocstrw(cp2,size) ;

	if (ep->p.t_inet.hostname == NULL)
	    return BAD ;

	cp += 1 ;
	size = sl - (cp - cp2) ;
	ep->p.t_inet.portspec = mallocstrw(cp,size) ;

	if (ep->p.t_inet.portspec == NULL) {

	    free(ep->p.t_inet.hostname) ;

	    return BAD ;
	}

	ep->type = FSPIPE_TRANSINET ;
	return rs ;
}
/* end subroutine (entry_loadinet) */


static int entry_openlocal(ep,f_listen)
FSPIPE_ENT	*ep ;
int		f_listen ;
{
	struct ustat	sb ;

	int	rs ;
	int	fd ;


	rs = u_open(ep->p.t_local.filepath,O_RDWR,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto bad0 ;

	rs = u_fstat(fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("fspipe/entry_openlocal: mode=%07o\n",sb.st_mode) ;
#endif

	if (rs < 0)
	    goto bad1 ;

	if (! S_ISCHR(sb.st_mode)) {
		rs = SR_BADF ;
		goto bad1 ;
	}

ret0:
	return (rs >= 0) ? fd : rs ;

bad1:
	u_close(fd) ;

bad0:
	goto ret0 ;
}
/* end subroutine (entry_openlocal) */


static int entry_openinet(ep,f_listen)
FSPIPE_ENT	*ep ;
int		f_listen ;
{
	struct ustat	sb ;

	int	rs ;
	int	fd ;
	int	to, opts ;
	int	af = 0 ;

	const char	*hostname ;
	const char	*portspec ;

#if	CF_DEBUGS
	debugprintf("fspipe/entry_openinet: protocol=%s\n",
	    ep->protocol) ;
#endif

	if (strcmp(ep->protocol,"tcp") != 0)
	    return SR_NOTSUP ;

	hostname = ep->p.t_inet.hostname ;
	portspec = ep->p.t_inet.portspec ;
	to = (5*60) ;
	opts = 0 ;
	rs = dialtcp(hostname,portspec,af,to,opts) ;
	fd = rs ;

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (entry_openinet) */


/* free up an entry */
static int entry_free(ep)
FSPIPE_ENT	*ep ;
{


	switch (ep->type) {

	case FSPIPE_TRANSLOCAL:
	    if (ep->p.t_local.filepath != NULL)
	        free(ep->p.t_local.filepath) ;

	    if (ep->p.t_local.nodename != NULL)
	        free(ep->p.t_local.nodename) ;

	    break ;

	case FSPIPE_TRANSINET:
	    if (ep->p.t_inet.hostname != NULL)
	        free(ep->p.t_inet.hostname) ;

	    break ;

	} /* end switch */

	return OK ;
}
/* end subroutine (entry_free) */


/* free up a malloc'ed thing */
static void freeit(p)
void	*p ;
{


	if (p != NULL)
	    free(p) ;

}
/* end subroutine (freeit) */



