/* getdomainname */

/* get the INET domain name -- for a "user" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */
#define	CF_GUESS	1	/* try to guess domain names? */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Get the INET domain name based on the nodename -- for a "user". This
        gets a user-specific INET domain, not a general "system" domain.

	Synopsis:

	int getdomainname(dbuf,dlen,nodename)
	char		*dbuf ;
	int		dlen ;
	const char	nodename[] ;

	Arguments:

	dbuf		result buffer
	dlen		result buffer length
	nodename	supplied local node name

	Returns:

	SR_OK		if OK
	SR_NOTFOUND	if could not get something needed for correct operation


	NOTE: Searching for the "current" domain is not an easy task and
	never has been.  There is no easy way to find out the domain part
	of the hostname for the current machine node.

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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp()| */

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<estrings.h>
#include	<char.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */

#define	RESOLVFNAME	"/etc/resolv.conf"

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

extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodename(char *,int) ;
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
	const char	*nodename ;	/* passed caller argument (supplied) */
	char		*dbuf ;		/* passed caller argument (returned) */
	const char	*varnode ;
	char		*bufnodename ;
	char		*bufhostname ;
	TRY_FL		f ;
	int		dlen ;		/* passed caller argument */
} ;

struct guess {
	const char	*name ;
	const char	*domain ;
} ;


/* forward references */

static int	try_start(TRY *,char *,int,const char *) ;
static int	try_vardomain(TRY *) ;
static int	try_varlocaldomain(TRY *) ;
static int	try_nodeuser(TRY *) ;
static int	try_gethostuser(TRY *) ;
static int	try_nodesys(TRY *) ;
static int	try_gethostsys(TRY *) ;
static int	try_resolve(TRY *) ;
static int	try_resolvefile(TRY *,const char *) ;
static int	try_guess(TRY *) ;
static int	try_finish(TRY *) ;
static int	try_bufnodename(TRY *) ;
static int	try_bufhostname(TRY *) ;
static int	try_hashostdomain(TRY *,const char *) ;
static int	try_nodename(TRY *) ;

static int	sfdomain(const char *,int,const char **) ;
static int	rmwhitedot(char *,int) ;


/* local variables */

static int	(*tries[])(TRY *) = {
	try_vardomain,
	try_varlocaldomain,
	try_nodeuser,
	try_gethostuser,
	try_nodesys,
	try_gethostsys,
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


int getdomainname(char *dbuf,int dlen,cchar *nodename)
{
	TRY		ti ;
	int		rs ;
	int		rs1 ;

	if (dbuf == NULL) return SR_FAULT ;

	if (dlen < 0) dlen = MAXHOSTNAMELEN ;

	dbuf[0] = '\0' ;
	if ((rs = try_start(&ti,dbuf,dlen,nodename)) >= 0) {
	    int	i ;

	    for (i = 0 ; tries[i] != NULL ; i += 1) {
#if	CF_DEBUGS
	        debugprintf("getdomainname: try-i=%u\n",i) ;
#endif
	        rs = (*tries[i])(&ti) ;
	        if (rs != 0) break ;
	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("getdomainname: mid rs=%d\n",rs) ;
#endif
	    if ((rs >= 0) && (dbuf[0] != '\0'))
	        rs = rmwhitedot(dbuf,rs) ;

	    rs1 = try_finish(&ti) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (try) */

#if	CF_DEBUGS
	debugprintf("getdomainname: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getdomainname) */


/* local subroutines */


static int try_start(TRY *tip,char *dbuf,int dlen,cchar *nodename)
{
	int		rs = SR_OK ;

	memset(&tip->f,0,sizeof(struct try_flags)) ;
	tip->dbuf = dbuf ;
	tip->dlen = dlen ;
	tip->nodename = nodename ;
	tip->varnode = NULL ;
	tip->bufnodename = NULL ;
	tip->bufhostname = NULL ;

	return rs ;
}
/* end subroutine (try_start) */


static int try_nodename(TRY *tip)
{
	int		rs = SR_OK ;

	if (tip->nodename == NULL) {
	    const char	*nodename = getenv(VARNODE) ;
	    if (nodename == NULL) {
	        if ((rs = try_bufnodename(tip)) >= 0) {
	            const int	nlen = NODENAMELEN ;
	            char	*nbuf = tip->bufnodename ;
	            rs = getnodename(nbuf,nlen) ;
	            if (rs >= 0) nodename = tip->bufnodename ;
	        }
	    }
	    if (rs >= 0)
	        tip->nodename = nodename ;
	} /* end if */

	return rs ;
}
/* end subroutine (try_nodename) */


static int try_vardomain(TRY *tip)
{
	int		rs = SR_OK ;
	cchar		*ep ;

	if ((ep = getenv(VARDOMAIN)) != NULL) {
	    int		 cl ;
	    const char	*cp ;
	    if ((cl = sfshrink(ep,-1,&cp)) > 0) {
	        if (cl <= MAXHOSTNAMELEN)
	            rs = snwcpy(tip->dbuf,tip->dlen,cp,cl) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (try_vardomain) */


static int try_varlocaldomain(TRY *tip)
{
	int		rs = SR_OK ;
	cchar		*ep ;

	if ((ep = getenv(VARLOCALDOMAIN)) != NULL) {
	    int		cl = -1 ;
	    const char	*tp ;
	    const char	*cp ;

	    while (CHAR_ISWHITE(*ep)) ep += 1 ;

	    cp = ep ;
#ifdef	COMMENT
	    if ((tp = strpbrk(ep," :")) != NULL)
	        cl = (tp-ep) ;
#else /* COMMENT */
	    tp = ep ;
	    while (*tp && (! CHAR_ISWHITE(*tp)) && (*tp != ':'))
	        tp += 1 ;
	    cl = (tp-ep) ;
#endif /* COMMENT */

	    if (cl > 0) {
	        if (cl <= MAXHOSTNAMELEN)
	            rs = snwcpy(tip->dbuf,tip->dlen,cp,cl) ;
	    }

	} /* end if (localdomain) */

	return rs ;
}
/* end subroutine (try_varlocaldomain) */


static int try_nodeuser(TRY *tip)
{
	int		rs = SR_OK ;

	if (tip->nodename != NULL) {
	    int		cl ;
	    cchar	*cp ;
	    cchar	*nn = tip->nodename ;
	    if ((cl = sfdomain(nn,-1,&cp)) > 0) {
	        if (cl <= MAXHOSTNAMELEN) {
	            rs = snwcpy(tip->dbuf,tip->dlen,cp,cl) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (try_nodeuser) */


static int try_gethostuser(TRY *tip)
{
	int		rs = SR_OK ;

	if (tip->nodename != NULL) {
	    rs = try_hashostdomain(tip,tip->nodename) ;
	} /* end if (nodename available) */

	return rs ;
}
/* end subroutine (try_gethostuser) */


static int try_nodesys(TRY *tip)
{
	int		rs ;

	if ((rs = try_bufhostname(tip)) >= 0) {
	    const int	nlen = MAXHOSTNAMELEN ;
	    char	*nbuf = tip->bufhostname ;
#if	CF_DEBUGS
	    debugprintf("try_nodesys: nbuf=%s\n",nbuf) ;
#endif
	    if ((rs = uc_gethostname(nbuf,nlen)) > 0) {
	        int	nl = rs ;
	        int	cl ;
	        cchar	*cp ;
	        rs = 0 ;
	        if ((cl = sfdomain(nbuf,nl,&cp)) > 0) {
	            if (cl <= MAXHOSTNAMELEN)
	                rs = snwcpy(tip->dbuf,tip->dlen,cp,cl) ;
	        }
	    }  /* end if (gethostname) */
#if	CF_DEBUGS
	    debugprintf("try_nodesys: 2 ret rs=%d\n",rs) ;
#endif
	    if (isNotPresent(rs)) {
	        rs = SR_OK ;
	        tip->bufhostname[0] = '\0' ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("try_nodesys: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (try_nodesys) */


static int try_gethostsys(TRY *tip)
{
	int		rs = SR_OK ;

	if (tip->bufnodename != NULL) {
	    rs = try_hashostdomain(tip,tip->bufnodename) ;
	} /* end if (nodename available) */

	return rs ;
}
/* end subroutine (try_gethostsys) */


static int try_resolve(TRY *tip)
{
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; resolvefnames[i] != NULL ; i += 1) {
	    rs = try_resolvefile(tip,resolvefnames[i]) ;
	    if (rs != 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (try_resolve) */


static int try_resolvefile(TRY *tip,cchar *fname)
{
	FILEBUF		b ;
	const int	to = TO_READ ;
	int		rs ;
	int		rs1 ;

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    const int	fd = rs ;
	    int		f_found = FALSE ;

	    if ((rs = filebuf_start(&b,fd,0L,FILEBUFLEN,0)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		len ;
		int		sl ;
		int		cl = 0 ;
		cchar		*tp, *sp ;
		cchar		*cp = NULL ;
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

		if (rs >= 0) {
	    	    rs = 0 ;
	    	    if (f_found && (cp != NULL))
	        	rs = snwcpy(tip->dbuf,tip->dlen,cp,cl) ;
		}

	        rs1 = filebuf_finish(&b) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */

	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("try_resolvefile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (try_resolvefile) */


static int try_guess(TRY *tip)
{
	int		rs = SR_OK ;
	cchar		*nn = tip->nodename ;

	if (nn == NULL) {
	    rs = try_nodename(tip) ;
	    nn = tip->nodename ;
	}

	if (rs >= 0) {
	    int		i ;
	    int		si = -1 ;
	    int		m ;
	    int		m_max = 0 ;
	    int		gnl ;
	    cchar	*gnp ;
	    rs = 0 ;
	    for (i = 0 ; ga[i].name != NULL ; i += 1) {
	        gnp = ga[i].name ;
		gnl = strlen(gnp) ;
	        if ((m = nleadstr(gnp,nn,-1)) == gnl) {
	            if (m > m_max) {
	                m_max = m ;
	                si = i ;
	            }
	        } /* end if */
	    } /* end for */
	    if (si >= 0)
	        rs = sncpy1(tip->dbuf,tip->dlen,ga[si].domain) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (try_guess) */


static int try_finish(TRY *tip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (tip->bufhostname != NULL) {
	    rs1 = uc_free(tip->bufhostname) ;
	    tip->bufhostname = NULL ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (tip->bufnodename != NULL) {
	    rs1 = uc_free(tip->bufnodename) ;
	    tip->bufnodename = NULL ;
	    if (rs >= 0) rs = rs1 ;
	}

	tip->varnode = NULL ;
	return rs ;
}
/* end subroutine (try_finish) */


static int try_bufnodename(TRY *tip)
{
	int		rs = SR_OK ;

	if (tip->bufnodename == NULL) {
	    const int	size = (NODENAMELEN+1) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        bp[0] = '\0' ;
	        tip->bufnodename = bp ;
	    }
	}

	return rs ;
}
/* end subroutine (try_bufnodename) */


static int try_bufhostname(TRY *tip)
{
	int		rs = SR_OK ;

	if (tip->bufhostname == NULL) {
	    const int	size = (MAXHOSTNAMELEN+1) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        bp[0] = '\0' ;
	        tip->bufhostname = bp ;
	    }
	}

	return rs ;
}
/* end subroutine (try_bufhostname) */


static int try_hashostdomain(TRY *tip,cchar *nn)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (nn != NULL) {
	    struct hostent	he, *hep = &he ;
	    const int		helen = getbufsize(getbufsize_he) ;
	    char		*hebuf ;
	    if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	        if ((rs = uc_gethostbyname(nn,&he,hebuf,helen)) >= 0) {
	            cchar	*hname = hep->h_name ;
	            cchar	*tp ;
	            cchar	*np = NULL ;
	            int		nl = -1 ;

		    rs = 0 ;
	            if (hname != NULL) {
		        if ((tp = strchr(hname,'.')) != NULL) {
	                    np = (tp+1) ;
	                    nl = strlen(np) ;
	                    if (nl > MAXHOSTNAMELEN) np = NULL ;
		        }
	            } /* end if (official name) */

	            if ((np == NULL) && (hep->h_aliases != NULL)) {
	                int	i ;
	                for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {
	                    if ((tp = strchr(hep->h_aliases[i],'.')) != NULL) {
	                        np = (tp+1) ;
	                        nl = strlen(np) ;
	                        if (nl > MAXHOSTNAMELEN) np = NULL ;
	                        if (np != NULL) break ;
	                    } /* end if */
	                } /* end for */
	            } /* end if (aliases) */

	            if (np != NULL) {
	                rs = snwcpy(tip->dbuf,tip->dlen,np,nl) ;
		    }

	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
		rs1 = uc_free(hebuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a) */
	} /* end if (nodename available) */

	return rs ;
}
/* end subroutine (try_hashostdomain) */


static int sfdomain(const char *sp,int sl,const char **rpp)
{
	int		cl = -1 ;
	cchar		*tp ;
	cchar		*cp ;
	if ((tp = strnchr(sp,sl,'.')) != NULL) {
	    cp = (tp + 1) ;
	    if (cp[0] != '\0') cl = strlen(cp) ;
	}
	if (rpp != NULL) {
	    *rpp = (cl >= 0) ? cp : NULL ;
	}
	return cl ;
}
/* end subroutine (sfdomain) */


/* remove trailing whitespace and dots */
static int rmwhitedot(char *bp,int bl)
{

	if (bl < 0) bl = strlen(bp) ;
	while ((bl > 0) && ((bp[bl-1] == '.') || CHAR_ISWHITE(bp[bl-1]))) {
	    bp[--bl] = '\0' ;
	}

	return bl ;
}
/* end subroutine (rmwhitedot) */


