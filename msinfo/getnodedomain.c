/* getnodedomain */

/* get the local node name and INET domain name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */
#define	CF_GUESS	1	/* try to guess domain names? */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the local host node name and INET domain name.

	Synopsis:

	int getnodedomain(nodename,domainname)
	char	nodename[] ;
	char	domainname[] ;

	Arguments:

	nodename	buffer to receive the local node name
	domainname	buffer to receive the local INET domain name

	Returns:

	SR_OK		if OK
	SR_NOTFOUND	if could not get something needed for correct operation


	The algorithm for finding the local nodename is:

	1. use the first component of the environment variable NODE

	2. use the first component of the nodename returned from the system


	NOTE: Searching for the "current" domain is not an easy task and never
	has been.  There is no easy way to find out the domain part of the
	hostname for the current machine node.

	We use the following algorithm for finding the local domain:

	1. use the environment variable DOMAIN

	2. use the first component of the LOCALDOMAIN environment variable

	3. see if the environment variable NODE has a domain

	4. get the node name from the system and see if it has a domain

	5. lookup the system node name with the system resolver functions
	   and grab the first name for the current node that has a domain
	   attached

	6. use the domainname given in the resolver configuration file
           (keyword 'domain') if it is not corrupted

	7. use the first component of the 'search' keyword from the
	   resolver configuration file (NOT YET IMPLEMENTED!)

	8. we try to guess what the domain name is from the given node name

	9. return that we couln't find a domain for the current node!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/systeminfo.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp()| */

#include	<vsystem.h>
#include	<estrings.h>
#include	<char.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */

#define	BUFLEN		(MAXPATHLEN + (8 * 1024))

#undef	HOSTBUFLEN
#define	HOSTBUFLEN	(MAXPATHLEN + (8 * MAXHOSTNAMELEN))

#define	RESOLVFNAME	"/etc/resolv.conf"
#define	LOCALHOSTNAME	"localhost"

#ifndef	VARNODE
#define	VARNODE		"NODE"
#endif

#ifndef	VARDOMAIN
#define	VARDOMAIN	"DOMAIN"
#endif

#ifndef	VARLOCALDOMAIN
#define	VARLOCALDOMAIN	"LOCALDOMAIN"
#endif

#define	FILEBUFLEN	1024
#define	TO_OPEN		10		/* time-out for open */
#define	TO_READ		30		/* time-out for read */

#undef	TRY
#define	TRY		struct try
#define	TRY_FL		struct try_flags


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */

struct try_flags {
	uint		initvarnode:1 ;
	uint		initsysinfo:1 ;
	uint		inituname:1 ;
	uint		initnode:1 ;
	uint		varnode:1 ;
	uint		uname:1 ;
	uint		sysinfo:1 ;
	uint		node:1 ;
} ;

struct try {
	char		*nodename ;	/* passed caller argument (returned) */
	char		*domainname ;	/* passed caller argument (returned) */
	const char	*varnode ;
	const char	*sysnodename ;
	TRY_FL		f ;
	char		nodenamebuf[NODENAMELEN + 1] ;
	char		sibuf[NODENAMELEN + 1] ;
} ;

struct guess {
	const char	*name ;
	const char	*domain ;
} ;


/* forward references */

int		getnodedomain(char *,char *) ;

static int	try_start(TRY *,char *,char *) ;
static int	try_startvarnode(TRY *) ;
static int	try_startsysinfo(TRY *) ;
static int	try_startuname(TRY *) ;
static int	try_startnode(TRY *) ;
static int	try_vardomain(TRY *) ;
static int	try_varlocaldomain(TRY *) ;
static int	try_varnode(TRY *) ;
static int	try_sysinfo(TRY *) ;
static int	try_uname(TRY *) ;
static int	try_gethost(TRY *) ;
static int	try_resolve(TRY *) ;
static int	try_resolvefile(TRY *,const char *) ;
static int	try_guess(TRY *) ;
static int	try_finish(TRY *) ;

static int	rmwhitedot(char *,int) ;


/* local variables */

static int	(*tries[])(TRY *) = {
	try_vardomain,
	try_varlocaldomain,
	try_varnode,
	try_sysinfo,
	try_uname,
	try_gethost,
	try_resolve,
	try_guess,
	NULL
} ;

static int	(*systries[])(TRY *) = {
	try_sysinfo,
	try_uname,
	try_gethost,
	try_resolve,
	try_guess,
	NULL
} ;

static const char	*resolvefnames[] = {
	RESOLVFNAME,
	"/var/run/resolv.conf",
	NULL
} ;

static const struct guess	ga[] = {
	{ "rc", "rightcore.com" },
	{ "jig", "rightcore.com" },
	{ "gateway", "ece.neu.com" },
	{ "vinson", "ece.neu.com" },
	{ "frodo", "ece.neu.com" },
	{ "olive", "ece.neu.com" },
	{ "gilmore", "ece.neu.com" },
	{ "dr", "dr.lucent.com" },
	{ "ho", "ho.lucent.com" },
	{ "mh", "mh.lucent.com" },
	{ "mt", "mt.lucent.com" },
	{ "cb", "cb.lucent.com" },
	{ NULL, NULL }
} ;


/* exported subroutines */


int getnodedomain(char *nbuf,char *dbuf)
{
	TRY		ti ;
	int		rs ;
	int		rs1 ;
	int		i = 0 ;

	if ((rs = try_start(&ti,nbuf,dbuf)) >= 0) {

/* do we need a nodename? */

	    if (nbuf != NULL) {
	        nbuf[0] = '\0' ;
	        if (! ti.f.initnode) rs = try_startnode(&ti) ;
	    } /* end if (trying to get a nodename) */

/* do we need a domainname? */

	    if ((rs >= 0) && (dbuf != NULL)) {

	        dbuf[0] = '\0' ;
	        for (i = 0 ; tries[i] != NULL ; i += 1) {
	            if ((rs = (*tries[i])(&ti)) != 0) break ;
	        } /* end for */

/* remove any stupid trailing dots from the domain name if any */

	        if ((rs >= 0) && (dbuf[0] != '\0')) {
	            rs = rmwhitedot(dbuf,rs) ;
		}

	    } /* end if (trying to get a domainname) */

	    rs1 = try_finish(&ti) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (try) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (getnodedomain) */


int getsysdomain(char *dbuf,int dlen)
{
	TRY		ti ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("getsysdomain: ent\n") ;
#endif

	if (dbuf == NULL) return SR_FAULT ;

	if (dlen < 0) dlen = MAXHOSTNAMELEN ;

	dbuf[0] = '\0' ;
	if ((rs = try_start(&ti,NULL,dbuf)) >= 0) {
	    int	i ;

/* do we need a domainname? */

	    for (i = 0 ; systries[i] != NULL ; i += 1) {
	        if ((rs = (*systries[i])(&ti)) != 0) break ;
	    } /* end for */

/* remove any stupid trailing dots from the domain name if any */

	    if ((rs >= 0) && (dbuf[0] != '\0')) {
	        int	dl = MIN(dlen,rs) ;
	        rs = rmwhitedot(dbuf,dl) ;
		len = rs ;
	    }

	    rs1 = try_finish(&ti) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (try) */

#if	CF_DEBUGS
	debugprintf("getsysdomain: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (getsysdomain) */


int getuserdomain(char *dbuf,int dlen)
{
	int		rs ;
	char		dn[MAXHOSTNAMELEN+1] ;

	if (dbuf == NULL) return SR_FAULT ;

	if ((rs = getnodedomain(NULL,dn)) >= 0) {
	    rs = sncpy1(dbuf,dlen,dn) ;
	}

	return rs ;
}
/* end subroutine (getuserdomain) */


/* local subroutines */


static int try_start(TRY *tip,char *nodename,char *domainname)
{

	memset(&tip->f,0,sizeof(struct try_flags)) ;
	tip->nodename = (nodename != NULL) ? nodename : tip->nodenamebuf ;
	tip->domainname = domainname ;
	tip->varnode = NULL ;
	tip->sysnodename = NULL ;

	tip->nodename[0] = '\0' ;
	tip->sibuf[0] = '\0' ;

	return SR_OK ;
}
/* end subroutine (try_start) */


static int try_startvarnode(TRY *tip)
{

	if (! tip->f.initvarnode) {
	    const char	*cp ;
	    tip->f.initvarnode = TRUE ;
	    if ((cp = getenv(VARNODE)) != NULL) {
	        tip->f.varnode = TRUE ;
	        tip->varnode = cp ;
	    }
	}

	return SR_OK ;
}
/* end subroutine (try_startvarnode) */


/* this should pretty much never be executed on any modern UNIX® box */
static int try_startuname(TRY *tip)
{
	int		rs = SR_OK ;

	if (! tip->f.inituname) {
	    struct utsname	un ;
	    tip->f.inituname = TRUE ;
	    if ((rs = u_uname(&un)) >= 0) {
	        const char	*cp ;
	        const char	*np = un.nodename ;
	        int		nl = strlen(un.nodename) ;
	        if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	            tip->f.uname = TRUE ;
	            tip->sysnodename = cp ;
	            rs = 0 ;
	        } /* end if (memory-allocation) */
	    } /* end if (uname) */
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (try_startuname) */


static int try_startsysinfo(TRY *tip)
{
	int		rs = SR_OK ;

	if (! tip->f.initsysinfo) {
	    int	rs1 ;
	    tip->f.initsysinfo = TRUE ;
#ifdef	SI_HOSTNAME
	    rs1 = u_sysinfo(SI_HOSTNAME,tip->sibuf,NODENAMELEN) ;
#else
	    rs1 = SR_NOSYS ;
#endif /* SI_HOSTNAME */
	    if (rs1 >= 0) {
	        tip->f.sysinfo = TRUE ;
	        rs = 0 ;
	    }
	}

	return rs ;
}
/* end subroutine (try_startsysinfo) */


static int try_startnode(TRY *tip)
{
	int		rs = SR_OK ;
	int		sl = -1 ;
	int		cl ;
	const char	*sp = NULL ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("getnodedomain/try_startnode: ent initnode=%u\n",
		tip->f.initnode) ;
#endif

	if (! tip->f.initnode) {
	    tip->f.initnode = TRUE ;

	    if ((rs >= 0) && (sp == NULL)) {
	        if (! tip->f.initvarnode) rs = try_startvarnode(tip) ;
	        cp = tip->varnode ;
	        if (tip->f.varnode && (cp != NULL) && (cp[0] != '\0')) sp = cp ;
	    }

#if	CF_DEBUGS
	debugprintf("getnodedomain/try_startnode: rs=%d 1 sp=%s\n",rs,sp) ;
#endif

	    if ((rs >= 0) && (sp == NULL)) {
	        if (! tip->f.initsysinfo) rs = try_startsysinfo(tip) ;
	        cp = tip->sibuf ;
	        if (tip->f.initsysinfo && (cp[0] != '\0')) sp = cp ;
	    }

#if	CF_DEBUGS
	debugprintf("getnodedomain/try_startnode: rs=%d 2 sp=%s\n",rs,sp) ;
#endif

	    if ((rs >= 0) && (sp == NULL)) {
	        if (! tip->f.inituname) rs = try_startuname(tip) ;
	        if (rs >= 0) {
	            cp = tip->sysnodename ;
	            if (tip->f.inituname && (cp[0] != '\0')) sp = cp ;
	        }
	    }

#if	CF_DEBUGS
	debugprintf("getnodedomain/try_startnode: rs=%d 3 sp=%s\n",rs,sp) ;
#endif

	    if (rs >= 0) {
	        if (sp != NULL) {
	            if ((cl = sfwhitedot(sp,sl,&cp)) > 0) {
	                tip->f.node = TRUE ;
	                strdcpy1w(tip->nodename,NODENAMELEN,cp,cl) ;
	            }
		} else
		    rs = SR_IDRM ;
	    } /* end if */

	} /* end if (needed initialization) */

#if	CF_DEBUGS
	debugprintf("getnodedomain/try_startnode: ret rs=%d n=%s\n",
		rs,tip->nodename) ;
#endif

	return (rs >= 0) ? 0 : rs ;
}
/* end subroutine (try_startnode) */


static int try_vardomain(TRY *tip)
{
	int		rs = SR_OK ;
	const char	*sp ;

	if ((sp = getenv(VARDOMAIN)) != NULL) {
	    int		cl ;
	    const char	*cp ;
	    if ((cl = sfshrink(sp,-1,&cp)) > 0) {
	        rs = snwcpy(tip->domainname,MAXHOSTNAMELEN,cp,cl) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (try_vardomain) */


static int try_varlocaldomain(TRY *tip)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*sp, *cp ;

	if ((sp = getenv(VARLOCALDOMAIN)) != NULL) {

	    while (CHAR_ISWHITE(*sp)) {
	        sp += 1 ;
	    }

	    cp = sp ;
	    while (*sp && (! CHAR_ISWHITE(*sp)) && (*sp != ':')) {
	        sp += 1 ;
	    }

	    cl = (sp - cp) ;
	    if (cl > 0)
	        rs = snwcpy(tip->domainname,MAXHOSTNAMELEN,cp,cl) ;

	} /* end if (localdomain) */

	return rs ;
}
/* end subroutine (try_varlocaldomain) */


static int try_varnode(TRY *tip)
{
	int		rs = SR_OK ;

	if (! tip->f.initvarnode) rs = try_startvarnode(tip) ;

	if ((rs >= 0) && tip->f.varnode) {
	    const char	*tp, *cp ;
	    const char	*sp = tip->varnode ;
	    if ((tp = strchr(sp,'.')) != NULL) {
	        cp = (tp + 1) ;
	        if (cp[0] != '\0')
	            rs = sncpy1(tip->domainname,MAXHOSTNAMELEN,cp) ;
	    }
	}

	return rs ;
}
/* end subroutine (try_varnode) */


static int try_sysinfo(TRY *tip)
{
	int		rs = SR_OK ;

	if (! tip->f.initsysinfo) rs = try_startsysinfo(tip) ;

	if ((rs >= 0) && tip->f.sysinfo) {
	    const char	*tp, *sp, *cp ;
	    sp = tip->sibuf ;
	    rs = 0 ;
	    if ((tp = strchr(sp,'.')) != NULL) {
	        cp = (tp + 1) ;
	        if (cp[0] != '\0')
	            rs = sncpy1(tip->domainname,MAXHOSTNAMELEN,cp) ;
	    }
	}

	return rs ;
}
/* end subroutine (try_sysinfo) */


static int try_uname(TRY *tip)
{
	int		rs = SR_OK ;

	if (! tip->f.inituname) rs = try_startuname(tip) ;

	if ((rs >= 0) && tip->f.uname) {
	    const char	*tp, *sp, *cp ;
	    sp = tip->sysnodename ;
	    if ((tp = strchr(sp,'.')) != NULL) {
	        cp = (tp + 1) ;
	        if (cp[0] != '\0')
	            rs = sncpy1(tip->domainname,MAXHOSTNAMELEN,cp) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (try_uname) */


static int try_gethost(TRY *tip)
{
	int		rs = SR_OK ;

	if (! tip->f.initnode) rs = try_startnode(tip) ;

	if ((rs >= 0) && tip->f.node) {
	    struct hostent	he, *hep = &he ;
	    const int		hlen = HOSTBUFLEN ;
	    const char		*tp ;
	    char		hbuf[HOSTBUFLEN + 1] ;
	    if ((rs = uc_gethostbyname(tip->nodename,&he,hbuf,hlen)) >= 0) {
		const int	dlen = MAXHOSTNAMELEN ;

		rs = 0 ;
	        if ((hep->h_name != NULL) &&
	            ((tp = strchr(hep->h_name,'.')) != NULL)) {

	            rs = sncpy1(tip->domainname,dlen,(tp + 1)) ;

	        } /* end if (official name) */

	        if ((rs == 0) && (hep->h_aliases != NULL)) {
	            int		i ;

	            for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {

	                if ((tp = strchr(hep->h_aliases[i],'.')) != NULL) {
	                    rs = sncpy1(tip->domainname,dlen,(tp+1)) ;
	                } /* end if */

			if (rs > 0) break ;
	            } /* end for */

	        } /* end if (alias names) */

	    } else if (isNotPresent(rs))
	        rs = SR_OK ;
	} /* end if (have node) */

	return rs ;
}
/* end subroutine (try_gethost) */


static int try_resolve(TRY *tip)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; resolvefnames[i] != NULL ; i += 1) {
	    if ((rs = try_resolvefile(tip,resolvefnames[i])) != 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (try_resolve) */


static int try_resolvefile(TRY *tip,cchar *fname)
{
	const int	dlen = MAXHOSTNAMELEN ;
	const int	to = TO_READ ;
	int		rs ;
	int		f_found = FALSE ;

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    FILEBUF	b ;
	    const int	fd = rs ;

	    if ((rs = filebuf_start(&b,fd,0L,FILEBUFLEN,0)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		len ;
		int		sl, cl ;
		const char	*tp, *sp, *cp ;
		char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            sp = lbuf ;
	            sl = len ;
	            if ((sl == 0) || (sp[0] == '#')) continue ;

	            cl = nextfield(sp,sl,&cp) ;

	            if (cl < 6) continue ;

	            if ((strncasecmp(cp,"domain",6) != 0) ||
	                (! CHAR_ISWHITE(cp[6])))
	                continue ;

	            sp += 6 ;
	            sl -= 6 ;
	            cl = nextfield(sp,sl,&cp) ;

	            if ((cp[0] == '#') ||
	                (strncmp(cp,"..",2) == 0)) continue ;

	            if ((tp = strnchr(cp,cl,'#')) != NULL)
	                cl = (tp - cp) ;

	            if (cl > 0) {
	                f_found = TRUE ;
	                break ;
	            }

	        } /* end while (reading lines) */

		if ((rs >= 0) && f_found)
	    	    rs = snwcpy(tip->domainname,dlen,cp,cl) ;

	        filebuf_finish(&b) ;
	    } /* end if (filebuf) */

	    u_close(fd) ;
	} else if (isNotPresent(rs))
	    rs = SR_OK ;

	return rs ;
}
/* end subroutine (try_resolvefile) */


static int try_guess(TRY *tip)
{
	int		rs = SR_OK ;

#if	CF_GUESS
	if (! tip->f.initnode) rs = try_startnode(tip) ;

	if ((rs >= 0) && tip->f.node) {
	    int		i ;
	    int		si = -1 ;
	    int		m ;
	    int		m_max = 0 ;
	    int		gnl ;
	    const char	*nn = tip->nodename ;
	    const char	*gnp ;
	    rs = 0 ;
	    for (i = 0 ; ga[i].name != NULL ; i += 1) {
	        gnp = ga[i].name ;
		gnl = strlen(gnp) ;
	        if ((m = nleadstr(gnp,nn,-1)) >= gnl) {
	            if (m > m_max) {
	                m_max = m ;
	                si = i ;
	            }
	        } /* end if */
	    } /* end for */
	    if (si >= 0)
	        rs = sncpy1(tip->domainname,MAXHOSTNAMELEN,ga[si].domain) ;
	} /* end if (have node) */
#endif /* CF_GUESS */

	return rs ;
}
/* end subroutine (try_guess) */


static int try_finish(TRY *tip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (tip->sysnodename != NULL) {
	    rs1 = uc_free(tip->sysnodename) ;
	    if (rs >= 0) rs = rs1 ;
	    tip->sysnodename = NULL ;
	}

	tip->varnode = NULL ;
	return rs ;
}
/* end subroutine (try_finish) */


/* remove trailing whitespace and dots */
static int rmwhitedot(char *bp,int bl)
{

	while ((bl > 0) && ((bp[bl-1] == '.') || CHAR_ISWHITE(bp[bl-1]))) {
	    bp[--bl] = '\0' ;
	}

	return (bl > 0) ? bl : SR_NOTFOUND ;
}
/* end subroutine (rmwhitedot) */


