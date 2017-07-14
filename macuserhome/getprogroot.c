/* getprogroot */

/* get the program root directory */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1995-05-01, David A­D­ Morano
	This subroutine is originally written.

	= 1998-03-10, David A­D­ Morano
	I added some comments.

*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine tries to find (get) a program root directory from a
        given program name and a number of given possible supplied program root
        directories.

	Synopsis:

	int getprogroot(pr,prnames,prlenp,output,name)
	const char	pr[] ;
	const char	*prnames[] ;
	char		output[] ;
	int		*prlenp ;
	const char	name[] ;

	Arguments:

	pr		PCS program root path
	prnames		list of program-root names
	prlenp		pointer to result variable to take resulting PR length
	output		buffer to receive result
	name		program to find

	Returns:

	<0		program was not found
	0		program was found in present working directory
	>0		found the program path and this is the returned length


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<storebuf.h>
#include	<dirseen.h>
#include	<nulstr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	isNotPresse(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strnnlen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct subinfo {
	IDS		id ;
	DIRSEEN		dirs ;
	uint		f_dirs:1 ;
	int		prlen ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_local(SUBINFO *,char *,const char *,int) ;
static int	subinfo_prs(SUBINFO *,cchar **,char *,cchar *,int) ;
static int	subinfo_pr(SUBINFO *,cchar *,char *,cchar *,int) ;
static int	subinfo_other(SUBINFO *,char *,const char *,int) ;
static int	subinfo_check(SUBINFO *,cchar *,int,char *,cchar *,int) ;
static int	subinfo_dirstat(SUBINFO *,struct ustat *,cchar *,int) ;
static int	subinfo_record(SUBINFO *,struct ustat *,cchar *,int) ;
static int	subinfo_xfile(SUBINFO *,const char *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	mkdfname(char *,const char *,int,const char *,int) ;


/* local variables */


/* exported subroutines */


int getprogroot(pr,prnames,prlenp,output,name)
const char	pr[] ;
const char	*prnames[] ;
int		*prlenp ;
char		output[] ;
const char	name[] ;
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		namelen ;
	int		f_changed = FALSE ;
	int		outlen = 0 ;

	if (name == NULL) return SR_FAULT ;
	if (output == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

	namelen = strlen(name) ;

	while ((namelen > 0) && (name[namelen - 1] == '/')) {
	    f_changed = TRUE ;
	    namelen -= 1 ;
	}

	output[0] = '\0' ;
	if ((rs = subinfo_start(sip)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("getprogroot: pr=%s\n",pr) ;
	    debugprintf("getprogroot: name=%t\n",name,namelen) ;
#endif

	    rs = SR_NOENT ;
	    if (strnchr(name,namelen,'/') == NULL) {

/* check if the PCS root directory exists */

	        if ((rs < 0) && (rs != SR_NOMEM) && (pr != NULL)) {
	            rs = subinfo_pr(sip,pr,output,name,namelen) ;
	            outlen = rs ;
	        }

#if	CF_DEBUGS
	        debugprintf("getprogroot: PR rs=%d\n",rs) ;
#endif

/* check other program roots */

	        if ((rs < 0) && (rs != SR_NOMEM) && (prnames != NULL)) {
	            rs = subinfo_prs(sip,prnames,output,name,namelen) ;
	            outlen = rs ;
	        }

#if	CF_DEBUGS
	        debugprintf("getprogroot: PRS rs=%d\n",rs) ;
#endif

/* search the rest of the execution path */

	        if ((rs < 0) && (rs != SR_NOMEM)) {
	            rs = subinfo_other(sip,output,name,namelen) ;
	            outlen = rs ;
	        }

#if	CF_DEBUGS
	        debugprintf("getprogroot: OTHER rs=%d\n",rs) ;
#endif

	    } else {
	        rs = subinfo_local(sip,output,name,namelen) ;
	        outlen = (f_changed) ? namelen : 0 ;
	    }

	    if (prlenp != NULL) {
	        *prlenp = sip->prlen ;
	    }

	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("getprogroot: ret rs=%d outlen=%u\n",rs,outlen) ;
#endif

	return (rs >= 0) ? outlen : rs ;
}
/* end subroutine (getprogroot) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip)
{
	int		rs ;

	memset(sip,0,sizeof(SUBINFO)) ;

	rs = ids_load(&sip->id) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f_dirs) {
	    rs1 = dirseen_finish(&sip->dirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_other(sip,output,np,nl)
SUBINFO		*sip ;
char		output[] ;
const char	np[] ;
int		nl ;
{
	int		rs = SR_NOENT ;
	int		outlen = 0 ;
	const char	*tp ;
	const char	*sp = getenv(VARPATH) ;

	sip->prlen = 0 ;

	if (sp != NULL) {

	while ((tp = strpbrk(sp,":;")) != NULL) {

	    rs = subinfo_check(sip,sp,(tp - sp),output,np,nl) ;
	    outlen = rs ;

	    sp = (tp + 1) ;
	    if ((rs >= 0) || (rs == SR_NOMEM)) break ;
	} /* end while */

	if ((rs < 0) && (rs != SR_NOMEM) && (sp[0] != '\0')) {
	    rs = subinfo_check(sip,sp,-1,output,np,nl) ;
	    outlen = rs ;
	}

	} /* end if (non-null) */

	return (rs >= 0) ? outlen : rs ;
}
/* end subroutine (subinfo_other) */


static int subinfo_check(sip,d,dlen,output,np,nl)
SUBINFO		*sip ;
const char	d[] ;
int		dlen ;
char		output[] ;
const char	np[] ;
int		nl ;
{
	struct ustat	sb ;
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		outlen = 0 ;

	if (sip->f_dirs) {
	    if ((rs = dirseen_havename(&sip->dirs,d,dlen)) >= 0) {
	        rs = SR_NOENT ;
	    } else if (rs == rsn) {
		rs = SR_OK ;
	    }
	}

	if (rs >= 0) {
	    if ((rs = subinfo_dirstat(sip,&sb,d,dlen)) >= 0) {
	        if ((rs1 = dirseen_havedevino(&sip->dirs,&sb)) >= 0) {
	            rs = SR_NOENT ;
		} else if (rs == rsn) {
	            rs = SR_OK ;
		}
		if ((rs = mkdfname(output,d,dlen,np,nl)) >= 0) {
		    outlen = rs ;
		    if ((rs = subinfo_xfile(sip,output)),isNotAccess(rs)) {
	    		rs = subinfo_record(sip,&sb,d,dlen) ;
		    }
	        }
	    }
	} /* end if (mkdfname) */

	return (rs >= 0) ? outlen : rs ;
}
/* end subroutine (subinfo_check) */


static int subinfo_local(sip,output,np,nl)
SUBINFO		*sip ;
char		output[] ;
const char	np[] ;
int		nl ;
{
	int		rs ;
	int		outlen = 0 ;

	if ((rs = mkpath1w(output,np,nl)) >= 0) {
	    outlen = rs ;
	    rs = subinfo_xfile(sip,output) ;
	}

	return (rs >= 0) ? outlen : rs ;
}
/* end subroutine (subinfo_local) */


static int subinfo_prs(sip,prnames,output,np,nl)
SUBINFO		*sip ;
const char	*prnames[] ;
char		output[] ;
const char	np[] ;
int		nl ;
{
	int		rs ;
	int		rs1 ;
	char		dn[MAXHOSTNAMELEN + 1] ;

	if ((rs = getnodedomain(NULL,dn)) >= 0) {
	    int		i ;
	    char	pr[MAXPATHLEN + 1] ;
	    rs = SR_NOENT ;
	    for (i = 0 ; prnames[i] != NULL ; i += 1) {

	        if ((rs1 = mkpr(pr,MAXPATHLEN,prnames[i],dn)) >= 0) {
	            rs = subinfo_pr(sip,pr,output,np,nl) ;
	        }

	        if ((rs >= 0) || (rs == SR_NOMEM)) break ;
	    } /* end for */
	} /* end if (getnodedomain) */

	return rs ;
}
/* end subroutine (subinfo_prs) */


static int subinfo_pr(sip,pr,output,np,nl)
SUBINFO		*sip ;
const char	pr[] ;
char		output[] ;
const char	np[] ;
int		nl ;
{
	int		rs ;
	int		outlen = 0 ;
	char		dname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("getprogroot: pr=%s\n",pr) ;
#endif

	rs = mkpath2(dname,pr,"bin") ;

	if (rs >= 0) {
	    rs = subinfo_check(sip,dname,-1,output,np,nl) ;
	    outlen = rs ;
	}

	if ((rs < 0) && (rs != SR_NOMEM)) {

	    if ((rs = mkpath2(dname,pr,"sbin")) >= 0) {
	        rs = subinfo_check(sip,dname,-1,output,np,nl) ;
	        outlen = rs ;
	    }

	} /* end if */

	if (rs >= 0)
	    sip->prlen = strlen(pr) ;

	return (rs >= 0) ? outlen : rs ;
}
/* end subroutine (subinfo_pr) */


static int subinfo_dirstat(sip,sbp,d,dlen)
SUBINFO		*sip ;
struct ustat	*sbp ;
const char	d[] ;
int		dlen ;
{
	NULSTR		ns ;
	int		rs ;
	const char	*dnp ;

	if ((rs = nulstr_start(&ns,d,dlen,&dnp)) >= 0) {

	    if ((rs = u_stat(dnp,sbp)) >= 0) {
	        rs = SR_NOTFOUND ;
	        if (S_ISDIR(sbp->st_mode))
	            rs = sperm(&sip->id,sbp,X_OK) ;
	    }

	    nulstr_finish(&ns) ;
	} /* end if (numstr) */

	return rs ;
}
/* end subroutine (subinfo_dirstat) */


static int subinfo_record(sip,sbp,d,dlen)
SUBINFO		*sip ;
struct ustat	*sbp ;
const char	d[] ;
int		dlen ;
{
	int		rs = SR_OK ;

	if (! sip->f_dirs) {
	    rs = dirseen_start(&sip->dirs) ;
	    sip->f_dirs = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = dirseen_add(&sip->dirs,d,dlen,sbp) ;
	}

	return rs ;
}
/* end subroutine (subinfo_record) */


static int subinfo_xfile(sip,name)
SUBINFO		*sip ;
const char	name[] ;
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(name,&sb)) >= 0) {
	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode)) {
	        rs = sperm(&sip->id,&sb,X_OK) ;
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_xfile) */


static int mkdfname(rbuf,d,dlen,np,nl)
char		rbuf[] ;
const char	d[] ;
int		dlen ;
const char	np[] ;
int		nl ;
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,d,dlen) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (rbuf[i - 1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,np,nl) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkdfname) */


