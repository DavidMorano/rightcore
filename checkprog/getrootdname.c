/* mkpr */

/* make program-root */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds a program-root directory given a software
	distribution name.  This subroutine is generally meant to be used by
	programs that are not themselves part of the facility that this
	subroutine is trying to get the program-root for.

	Synopsis:

	int mkpr(pbuf,plen,prname,domain)
	char		pbuf[] ;
	int		plen ;
	const char	prname[] ;
	const char	domain[] ;

	Arguments:

	pbuf		supplied buffer to receive the resulting directory
	plen		supplied length of buffer
	prname		the name of the software distribution to lookup
	domain		domain-name

	Returns:

	>=0		length of resulting directory path
	<0		could not find the program-root directory


	Implementation note:

	We use the subroutine 'dirsearch()' (below) to search the 'basenames'
	directories since they may be indirect automount points and we don't
	want to get those stupid SYSLOG messages blabbing on about some certain
	name in the automount map not being available.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<ids.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<fsdir.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	VARHOME
#define	VARHOME		"HOME"
#endif

#ifndef	VARDOMAIN
#define	VARDOMAIN	"DOMAIN"
#endif

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"		/* program-root */
#endif

#ifndef	VARPREXTRA
#define	VARPREXTRA	"EXTRA"		/* program-root */
#endif

#ifndef	SWDFNAME
#define	SWDFNAME	".swd"
#endif

#define	DMODE		(X_OK | R_OK)

#define	HOMEBASEDNAME	"add-on"

#define	PRNAME		"LOCAL"

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	mknpath3(char *,int,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	hasuc(const char *,int) ;
extern int	haslc(const char *,int) ;
extern int	isNotPresent(int) ;


/* external variables */


/* local structures */

struct subinfo_flags {
	uint		ids:1 ;
	uint		dname:1 ;
} ;

struct subinfo {
	IDS		id ;
	const char	*prname ;
	const char	*domain ;
	const char	*dname ;
	SUBINFO_FL	init, open ;
} ;

struct prmap {
	const char	*prname ;
	const char	*dname ;
} ;

struct domainbase {
	const char	*domain ;
	const char	*basedname ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char *,const char *) ;
static int	subinfo_id(SUBINFO *) ;
static int	subinfo_dir(SUBINFO *,const char *,mode_t) ;
static int	subinfo_finish(SUBINFO *) ;

static int	subinfo_env(SUBINFO *,char *,int) ;
static int	subinfo_domain(SUBINFO *,char *,int) ;
static int	subinfo_user(SUBINFO *,char *,int) ;
static int	subinfo_prmap(SUBINFO *,char *,int) ;
static int	subinfo_home(SUBINFO *,char *,int) ;
static int	subinfo_bases(SUBINFO *,char *,int) ;

static int	dirsearch(const char *,const char *) ;


/* local variables */

static int	(*gettries[])(SUBINFO *,char *,int) = {
	subinfo_domain,
	subinfo_user,
	subinfo_prmap,
	subinfo_home,
	NULL
} ;

static int	(*mktries[])(SUBINFO *,char *,int) = {
	subinfo_env,
	subinfo_domain,
	subinfo_user,
	subinfo_prmap,
	subinfo_home,
	subinfo_bases,
	NULL
} ;

static const struct prmap	prmaps[] = {
	{ "root", "/" },
	{ "usr", "/usr" },
	{ "xpg4", "/usr/xpg4" },
	{ "xpg5", "/usr/xpg5" },
	{ "xpg6", "/usr/xpg6" },
	{ "dt", "/usr/dt" },
	{ "ccs", "/usr/ccs" },
	{ "openwin", "/usr/openwin" },
	{ "java", "/usr/java" },
	{ "extra", "/usr/extra" },
	{ "preroot", "/usr/preroot" },
	{ "apache", "/usr/apache" },
	{ "postfix", "/usr/postfix" },
	{ NULL, NULL }
} ;

static const struct domainbase	domains[] = {
	{ "rightcore.com", "/usr/add-on" },
	{ "rightcore.org", "/usr/add-on" },
	{ "morano.ws", "/usr/add-on" },
	{ "custombibles.online", "/usr/add-on" },
	{ "clearresearch.org", "/usr/add-on" },
	{ "ece.neu.edu", "/home/student/dmorano/add-on" },
	{ NULL, NULL }
} ;

static const char	*basednames[] = {
	"/usr/add-on",
	"/usr",
	"/opt",
	NULL
} ;


/* exported subroutines */


int getrootdname(char *rbuf,int rlen,cchar *prname,cchar *domain)
{
	SUBINFO		si ;
	int		rs ;
	int		rl = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 0) rlen = MAXPATHLEN ;

	if ((prname == NULL) || (prname[0] == '\0'))
	    prname = PRNAME ;

	rbuf[0] = '\0' ;
	if ((rs = subinfo_start(&si,prname,domain)) >= 0) {
	    int	i ;

	    rs = SR_NOTDIR ;
	    for (i = 0 ; gettries[i] != NULL ; i += 1) {
	        rs = (*gettries[i])(&si,rbuf,rlen) ;
	        rl = rs ;
	        if (rs != 0) break ;
	    } /* end for */
	    if (rs == 0) rs = SR_NOTFOUND ;

	    i = subinfo_finish(&si) ;
	    if (rs >= 0) rs = i ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("getrootdname: ret rs=%d len=%u\n",rs,rl) ;
	debugprintf("getrootdname: buf=%s\n",buf) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (getrootdname) */


int mkpr(char *pbuf,int plen,cchar *prname,cchar *domain)
{
	SUBINFO		ti ;
	int		rs ;
	int		rl = 0 ;

	if (pbuf == NULL) return SR_FAULT ;
	if (prname == NULL) return SR_FAULT ;

	if (prname[0] == '\0') return SR_INVALID ;

	pbuf[0] = '\0' ;
	if ((rs = subinfo_start(&ti,prname,domain)) >= 0) {
	    int	i ;

	    rs = SR_NOTFOUND ;
	    for (i = 0 ; mktries[i] != NULL ; i += 1) {
	        rs = (*mktries[i])(&ti,pbuf,plen) ;
		rl = rs ;
	        if (rs != 0) break ;
	    } /* end for */
	    if (rs == 0) rs = SR_NOTFOUND ;

	    i = subinfo_finish(&ti) ;
	    if (rs >= 0) rs = i ;
	} /* end if (subinfo) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mkpr) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *prname,cchar *domain)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->prname = prname ;
	sip->domain = domain ;
	sip->dname = prname ;

	if (hasuc(prname,-1)) { /* while keeping stack mostly shallow */
	    const int	dlen = MAXPATHLEN ;
	    char	dbuf[MAXPATHLEN+1] ;
	    if ((rs = sncpylc(dbuf,dlen,prname)) >= 0) {
		cchar	*cp ;
		if ((rs = uc_mallocstrw(dbuf,rs,&cp)) >= 0) {
		    sip->open.dname = TRUE ;
	    	    sip->dname = cp ;
		} /* end if (m-a) */
	    } /* end if (sncpylc) */
	} /* end if (had some upper-case) */

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->open.ids) {
	    sip->open.ids = FALSE ;
	    rs1 = ids_release(&sip->id) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (sip->open.dname && (sip->dname != NULL)) {
	    sip->open.dname = FALSE ;
	    rs1 = uc_free(sip->dname) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->dname = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_id(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (! sip->init.ids) {
	    sip->init.ids = TRUE ;
	    if ((rs = ids_load(&sip->id)) >= 0) {
	        sip->open.ids = TRUE ;
	    }
	}

	return (rs >= 0) ? sip->open.ids : rs ;
}
/* end subroutine (subinfo_id) */


static int subinfo_dir(SUBINFO *sip,cchar *dname,mode_t dm)
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;

	if ((rs = uc_stat(dname,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        if ((rs = subinfo_id(sip)) >= 0) {
	            if ((rs = sperm(&sip->id,&sb,dm)) >= 0) {
			f = TRUE ;
		    } else if (rs == SR_ACCESS) {
			rs = SR_OK ;
		    }
		}
	    } /* end if (is-dir) */
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	} /* end if (uc_stat) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_dir) */


static int subinfo_env(SUBINFO *sip,char *rbuf,int rlen)
{
	const int	elen = MAXNAMELEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	const char	*envp = sip->prname ;
	char		ebuf[MAXNAMELEN + 1] ;

	if (haslc(envp,-1)) {
	    rs = sncpyuc(ebuf,elen,envp) ;
	    envp = ebuf ;
	}

	if (rs >= 0) {
	    if (envp[0] != '\0') {
		cchar	*cp ;
	        if ((cp = getenv(envp)) != NULL) {
	            if ((rs = mknpath1(rbuf,rlen,cp)) >= 0) {
	                len = rs ;
		        if ((rs = subinfo_dir(sip,rbuf,DMODE)) == 0) {
			    len = 0 ;
			}
	            }
	        } /* end if */
	    } /* end if (non-null) */
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_env) */


static int subinfo_domain(SUBINFO *sip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (sip->domain == NULL)
	    sip->domain = getenv(VARDOMAIN) ;

	if ((sip->domain != NULL) && (sip->domain[0] != '\0')) {
	    int		dlen = strlen(sip->domain) ;
	    cchar	*dnp = sip->domain ;

/* perform any necessary cleanup (needed because of NIS+ crap) */

	    while ((dlen > 0) && (sip->domain[dlen - 1] == '.')) {
	        dlen -= 1 ;
	    }

	    if (dlen > 0) {
	        char	dname[MAXHOSTNAMELEN + 1] ;

	        if (hasuc(dnp,dlen)) {
	            rs = sncpylc(dname,MAXHOSTNAMELEN,dnp) ;
	            dnp = dname ;
	        }

/* do the lookup (this is a full string match) */

	        if (rs >= 0) {
	            int		i ;
		    int		m ;
		    cchar	*bnp ;

	            for (i = 0 ; domains[i].domain != NULL ; i += 1) {
	                if ((m = nleadstr(domains[i].domain,dnp,dlen)) > 0) {
	                    if ((domains[i].domain[m] == '\0') && (m == dlen)) {
	                        break ;
		            }
	                }
	            } /* end for */

	            bnp = domains[i].basedname ;
	            if ((domains[i].domain != NULL) && (bnp != NULL)) {
	                if ((rs = mknpath2(rbuf,rlen,bnp,sip->dname)) >= 0) {
	                    len = rs ;
	                    if ((rs = subinfo_dir(sip,rbuf,DMODE)) == 0) {
		                len = 0 ;
		            }
	                }
	            } /* end if (got a domain match) */

	        } /* end if (ok) */

	    } /* end if (positive) */

	} /* end if (have domain name) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_domain) */


static int subinfo_user(SUBINFO *sip,char *rbuf,int rlen)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		len = 0 ;
	char		*pwbuf ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,sip->dname)) >= 0) {
	        char	tbuf[MAXPATHLEN + 1] ;
	        if ((rs = mkpath2(tbuf,pw.pw_dir,SWDFNAME)) >= 0) {
		    struct ustat	sb ;
		    if ((rs = u_lstat(tbuf,&sb)) >= 0) {
	                if (S_ISLNK(sb.st_mode)) {
	                    if ((rs = u_readlink(tbuf,rbuf,rlen)) >= 0) {
		                int	bl = rs ;
		                len = rs ;

		                rbuf[bl] = '\0' ;
	                        if ((bl == 1) && (rbuf[0] == '.')) {
	                            rs = mknpath1(rbuf,rlen,pw.pw_dir) ;
		                    len = rs ;
		                } else if ((bl > 0) && (rbuf[0] != '/')) {
			            char	tbuf[MAXPATHLEN + 1] ;
			            mkpath1(tbuf,rbuf) ;
	                            rs = mknpath2(rbuf,rlen,pw.pw_dir,tbuf) ;
		                    len = rs ;
		                } /* end if */

		                if ((rs >= 0) && (len > 0)) {
				    const mode_t	dm = DMODE ;
	        		    if ((rs = subinfo_dir(sip,rbuf,dm)) == 0) {
					len = 0 ;
				    }
				}

	        	    } /* end if (read link) */
	                } /* end if (symbolic link) */
		    } else if (isNotPresent(rs)) {
			rs = SR_OK ;
	            } /* end if (have SWD directory entry) */
	        } /* end if (mkpath) */
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    } /* end if (GETPW_NAME) */
	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_user) */


static int subinfo_prmap(SUBINFO *sip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		i ;
	int		len = 0 ;

	for (i = 0 ; prmaps[i].prname != NULL ; i += 1) {
	    int		m ;
	    if ((m = nleadstr(prmaps[i].prname,sip->dname,-1)) > 0) {
	        if ((prmaps[i].prname[m] == '\0') && (sip->dname[m] == '\0')) {
	            break ;
		}
	    }
	} /* end for */

	if (prmaps[i].prname != NULL) {
	    if ((rs = mknpath1(rbuf,rlen,prmaps[i].dname)) >= 0) {
	        len = rs ;
	        if ((rs = subinfo_dir(sip,rbuf,DMODE)) == 0) {
		    len = 0 ;
		}
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_prmap) */


static int subinfo_home(SUBINFO *sip,char rbuf[],int rlen)
{
	const int	hlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	const char	*hd ;
	char		hbuf[MAXPATHLEN+1] = { 0 } ;

	if ((hd = getenv(VARHOME)) == NULL) {
	    if ((rs = getuserhome(hbuf,hlen,"-")) >= 0) {
		hd = hbuf ;
	    }
	} /* end if */

	if ((rs >= 0) && (hd != NULL) && hbuf[0]) {
	    cchar	*addon = HOMEBASEDNAME ;
	    if ((rs = mknpath3(rbuf,rlen,hd,addon,sip->dname)) >= 0) {
	        len = rs ;
	        if ((rs = subinfo_dir(sip,rbuf,DMODE)) == 0) {
		    len = 0 ;
		}
	    }
	} /* end if (got a HOME directory) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_home) */


static int subinfo_bases(SUBINFO *sip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		i ;
	int		len = 0 ;

	for (i = 0 ; basednames[i] != NULL ; i += 1) {
	    if ((rs = dirsearch(basednames[i],sip->dname)) > 0) {
	        if ((rs = mknpath2(rbuf,rlen,basednames[i],sip->dname)) >= 0) {
		    len = rs ;
		    if ((rs = subinfo_dir(sip,rbuf,DMODE)) == 0) {
	                len = 0 ;
		    }
	        }
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	    if (rs != 0) break ;
	} /* end for */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_bases) */


/* search a directory for an entry */
static int dirsearch(cchar *basedname,cchar *username)
{
	FSDIR		dir ;
	FSDIR_ENT	ds ;
	int		rs ;
	int		rs1 ;
	int		f_found = FALSE ;

	if ((rs = fsdir_open(&dir,basedname)) >= 0) {
	    cchar	*fnp ;
	    while ((rs = fsdir_read(&dir,&ds)) > 0) {
		fnp = ds.name ;
		if (fnp[0] != '.') {
		    f_found = (strcmp(fnp,username) == 0) ;
		    if (f_found) break ;
		}
	    } /* end while */
	    rs1 = fsdir_close(&dir) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */

	return (rs >= 0) ? f_found : rs ;
}
/* end subroutine (dirsearch) */


