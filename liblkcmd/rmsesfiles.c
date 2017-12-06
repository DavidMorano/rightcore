/* rmsesfiles */

/* remove session files and session directories (as appropriate) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Remove files from a specified directory that are not longer needed.

	Synopsis:

	int rmsesfiles(cchar *dname)

	Arguments:

	dname		directory name (as a string)

	Returns:

	<0		error
	>=0		number of files removed (deleted)


	Notes:

        If a lock file is left by a previous process that was killed, we try to
        "adopt" it so that after we are done, we delete it as the original
        process that created it was supposed to do. In this way, the directory
        is never permanently locked. Our real "lock" is not the existence of the
        the lock file itself, but rather our record lock on the lock file.
        Record locks are automatically deleted with the file-descriptor holding
        the record lock is closed. So a process that is killed while holding the
        record lock, implicitly deletes the record lock on its death.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<cfdec.h>
#include	<ids.h>
#include	<fsdir.h>
#include	<vecpstr.h>
#include	<localmisc.h>


/* local defines */

#define	RMSESFILES_LOCKFILE	"LOCK"
#define	RMDIRSFILES_DIRSIZE	512

#define	OPENSTATE		struct openstate

#define	NDF			"rmsesfiles.deb"


/* external subroutines */

extern int	mkpath1(char *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	removes(cchar *) ;
extern int	isproc(pid_t) ;
extern int	hasNotDots(cchar *,int) ;
extern int	isNotValid(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* global variables */


/* local structures */

struct openstate {
	uint		f_created ;
} ;


/* forward references */

static int	lockbegin(char *,int) ;
static int	lockend(char *,int,int) ;

static int	rmsesdir(IDS *,char *,int) ;
static int	rmdirfiles(char *,int) ;

static int	vecpstr_dirload(vecpstr *,char *,int) ;
static int	vecpstr_dirdel(vecpstr *,char *,int) ;

static int	openstate(OPENSTATE *,cchar *,int,mode_t) ;
static int	rmfile(cchar *) ;

static int	isLocked(int) ;
static int	isNotRunning(cchar *,int) ;


/* local variables */

static const int	rslocked[] = {
	SR_EXISTS,
	SR_AGAIN,
	SR_ACCESS,
	0
} ;


/* exported subroutines */


int rmsesfiles(cchar *dname)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		pbuf[MAXPATHLEN + 1] ;
	if (dname == NULL) return SR_FAULT ;
	if (dname[0] == '\0') return SR_FAULT ;
	if ((rs = mkpath1(pbuf,dname)) >= 0) {
	    SIGBLOCK	s ;
	    const int	plen = rs ;
	    if ((rs = sigblock_start(&s,NULL)) >= 0) {
	        if ((rs = lockbegin(pbuf,plen)) >= 0) {
	            IDS		id ;
	            const int	lfd = rs ;
	            if ((rs = ids_load(&id)) >= 0) {
	                FSDIR		d ;
	                FSDIR_ENT	de ;
	                if ((rs = fsdir_open(&d,pbuf)) >= 0) {
	                    while ((rs = fsdir_read(&d,&de)) > 0) {
	                        cchar	*np = de.name ;
	                        if (hasNotDots(np,rs) && (np[0] == 's')) {
	                            if ((rs = pathadd(pbuf,plen,np)) >= 0) {
	                                rs = rmsesdir(&id,pbuf,rs) ;
	                                c += rs ;
	                            } /* end if (pathadd) */
	                        } /* end (not dots) */
	                        if (rs < 0) break ;
	                    } /* end while */
	                    rs1 = fsdir_close(&d) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (fsdir) */
	                pbuf[plen] = '\0' ;
	                rs1 = ids_release(&id) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (ids) */
	            rs1 = lockend(pbuf,plen,lfd) ;
	            if (rs >= 0) rs = rs1 ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        } else if (isLocked(rs)) {
	            rs = SR_OK ;
	        } /* end if (lockend) */
	        rs1 = sigblock_finish(&s) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sigblock) */
	} /* end if (mkpath) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (rmsesfiles) */


/* local subroutines */


static int lockbegin(char *pbuf,int plen)
{
	int		rs ;
	int		lfd = -1 ;
	cchar		*lfn = RMSESFILES_LOCKFILE ;
	if ((rs = pathadd(pbuf,plen,lfn)) >= 0) {
	    OPENSTATE		ols ;
	    const mode_t	om = 0666 ;
	    const int		of = (O_CREAT|O_RDWR|O_TRUNC) ;
	    if ((rs = openstate(&ols,pbuf,of,om)) >= 0) {
	        lfd = rs ;
	        if (ols.f_created) {
	            rs = u_fchmod(lfd,om) ;
	        }
	        if (rs >= 0) {
	            rs = uc_lockf(lfd,F_TWLOCK,0) ;
	        }
	        if (rs < 0) {
	            if (ols.f_created) {
	                u_unlink(pbuf) ;
	            }
	            u_close(lfd) ;
	        }
	    } /* end if (open) */
	    pbuf[plen] = '\0' ; /* restore */
	} /* end if (pathadd) */
	return (rs >= 0) ? lfd : rs ;
}
/* end subroutine (lockbegin) */


static int lockend(char *pbuf,int plen,int lfd)
{
	int		rs ;
	cchar		*lfn = RMSESFILES_LOCKFILE ;
	if ((rs = pathadd(pbuf,plen,lfn)) >= 0) {
	    u_unlink(pbuf) ;
	    u_close(lfd) ;
	}
	return rs ;
}
/* end subroutine (lockend) */


static int rmsesdir(IDS *idp,char *pbuf,int plen)
{
	USTAT		sb ;
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUGN
	nprintf(NDF,"rmsesdir: ent d=%t\n",pbuf,plen) ;
#endif
	if ((rs = u_stat(pbuf,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        const int	am = (R_OK|W_OK|X_OK) ;
	        if ((rs = sperm(idp,&sb,am)) >= 0) {
#if	CF_DEBUGN
	{
		const int	ulen = USERNAMELEN ;
		char		ubuf[USERNAMELEN+1] ;
		getusername(ubuf,ulen,sb.st_uid) ;
	            nprintf(NDF,"rmsesdir: sperm() rs=%d\n",rs) ;
		nprintf(NDF,"rmsesdir: u=%s\n",ubuf) ;
	}
#endif
	            if ((rs = rmdirfiles(pbuf,plen)) > 0) {
	                int	cl ;
	                cchar	*cp ;
	                c += rs ;
	                if ((cl = sfbasename(pbuf,plen,&cp)) > 0) {
	                    if ((rs = isNotRunning(cp,cl)) > 0) {
	                        rs = removes(pbuf) ;
	                    }
	                }
	            } /* end if (rmdirfiles) */
	        } else if (isNotAccess(rs)) {
#if	CF_DEBUGN
	            nprintf(NDF,"rmsesdir: sperm() NOACCESS rs=%d\n",rs) ;
#endif
	            rs = SR_OK ;
	        }
	    }
	} else if (isNotAccess(rs)) {
	    rs = SR_OK ;
	} /* end if (stat) */
#if	CF_DEBUGN
	nprintf(NDF,"rmsesdir: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (rmsesdir) */


static int rmdirfiles(char *pbuf,int plen)
{
	vecpstr		files ;
	const int	cs = RMDIRSFILES_DIRSIZE ;
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;

	if (pbuf == NULL) return SR_FAULT ;
	if (pbuf[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGN
	nprintf(NDF,"rmdirfiles: ent d=%s\n",pbuf) ;
#endif

	if ((rs = vecpstr_start(&files,0,cs,0)) >= 0) {
	    int		c = 0 ;
	    if ((rs = vecpstr_dirload(&files,pbuf,plen)) > 0) {
	        c = rs ;
	        if ((rs = vecpstr_dirdel(&files,pbuf,plen)) >= 0) {
	            if (c == rs) rc = (rs+1) ;
	        }
	    } else if (rs == 0) {
	        rc = 1 ;
	    }
	    rs1 = vecpstr_finish(&files) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (files) */

#if	CF_DEBUGN
	nprintf(NDF,"rmdirfiles: ret rs=%d rc=%u\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (rmdirfiles) */


/* ARGSUSED */
static int vecpstr_dirload(vecpstr *flp,char *pbuf,int plen)
{
	FSDIR		dir ;
	FSDIR_ENT	de ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUGN
	nprintf(NDF,"vecpstr_dirload: ent\n") ;
#endif
	if ((rs = fsdir_open(&dir,pbuf)) >= 0) {
	    int		nl ;
	    const char	*np ;
	    while ((rs = fsdir_read(&dir,&de)) > 0) {
	        nl = rs ;
	        np = de.name ;
	        if (hasNotDots(np,nl)) {
	            c += 1 ;
	            rs = vecpstr_add(flp,np,nl) ;
	        } /* end if (hasNotDots) */
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = fsdir_close(&dir) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
#if	CF_DEBUGN
	nprintf(NDF,"vecpstr_dirload: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_dirload) */


static int vecpstr_dirdel(vecpstr *flp,char *pbuf,int plen)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	cchar		*np ;
#if	CF_DEBUGN
	nprintf(NDF,"vecpstr_dirdel: ent\n") ;
#endif
	for (i = 0 ; vecpstr_get(flp,i,&np) >= 0 ; i += 1) {
	    if ((np != NULL) && (np[0] != '\0')) {
	        if ((rs = pathadd(pbuf,plen,np)) >= 0) {
#if	CF_DEBUGN
	nprintf(NDF,"vecpstr_dirdel: p=%s\n",pbuf) ;
#endif
	            if (np[0] == 'p') {
	                if ((rs = isNotRunning(np,-1)) > 0) {
	                    rs = rmfile(pbuf) ;
	                    c += rs ;
	                } /* end if (isNotRunning) */
	            } else {
	                rs = rmfile(pbuf) ;
	                c += rs ;
	            } /* end if (type) */
#if	CF_DEBUGN
	nprintf(NDF,"vecpstr_dirdel: c=%u\n",c) ;
#endif
	        } /* end if (pathadd) */
	    }
	    if (rs < 0) break ;
	} /* end for */
	pbuf[plen] = '\0' ; /* restore */
#if	CF_DEBUGN
	nprintf(NDF,"vecpstr_dirdel: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_dirdel) */


static int openstate(OPENSTATE *lsp,cchar *fn,int of,mode_t om)
{
	int		rs ;
	if (lsp == NULL) return SR_FAULT ;
	if (fn == NULL) return SR_FAULT ;
	if (fn[0] == '\0') return SR_INVALID ;
	memset(lsp,0,sizeof(OPENSTATE)) ;
	if ((rs = u_open(fn,(of|O_EXCL),om)) >= 0) {
	    lsp->f_created = TRUE ;
	} else if ((!(of&O_EXCL)) && (rs == SR_EXISTS)) {
	    rs = u_open(fn,of,om) ;
	}
	return rs ;
}
/* end subroutine (openstate) */


static int rmfile(cchar *fn)
{
	int		rs ;
	if ((rs = u_unlink(fn)) >= 0) {
	    rs = 1 ;
	} else if (isNotAccess(rs)) {
	    rs = SR_OK ;
	}
	return rs ;
}
/* end subroutine (rmfile) */


static int isLocked(int rs)
{
	return isOneOf(rslocked,rs) ;
}
/* end subroutine (isLocked) */


static int isNotRunning(cchar *np,int nl)
{
	const int	sch = MKCHAR(np[0]) ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (nl < 0) nl = strlen(np) ;
#if	CF_DEBUGN
	nprintf(NDF,"isNotRunning: n=%t\n",np,nl) ;
#endif
	if ((nl > 1) && ((sch == 'p') || (sch == 's'))) {
	    uint	uv ;
	    np += 1 ;
	    nl -= 1 ;
	    if ((rs = cfdecui(np,nl,&uv)) >= 0) {
	        const pid_t	pid = uv ;
	        if ((rs = isproc(pid)) == 0) {
	            f = TRUE ;
	        }
	    } else if (isNotValid(rs)) {
	        f = TRUE ;
	        rs = SR_OK ;
	    }
	}
#if	CF_DEBUGN
	nprintf(NDF,"isNotRunning: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (isNotRunning) */


