/* wdt */

/* walk directory tree */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_ACCESS	0		/* call 'access(2)'? */


/* revision history:

	= 1998-05-17, David A­D­ Morano

	This was made to get around some 'ftw(3c)' limitations.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is similar to the 'ftw(3c)' subroutine in that it
        "walks" a directory hierarchy.

	Note that unlike 'ftw(3c)' this subroutine is NOT recursive!

	Synopsis:

	int wdt(basedir,wopts,uf,uarg)
	char		basedir[] ;
	int		wopts ;
	int		(*uf)() ;
	void		*uarg ;

	Arguments:

	basedir		directory at top of tree
	wopts		options for usage (FOLLOW links or not)
	uf		user function to call
	uarg		user argument (usually a pointer)

	Returns:

	>=0		OK
	<0		error

	Description:

	The user-defined subroutie that we call.

	Synopsis:

	int checkname(cchar *name,struct ustat *sbp,void *uarg)

	Arguments:

	name		name (string)
	sbp		pointer to STAT structure
	uarg		user-supplied argument

	Returns:

	<0		error and exit enumeration with given error
	==0		continue
	>0		exit enumberation w/ count


*******************************************************************************/


#define	WDT_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fifostr.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"wdt.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	hasNotDots(cchar *,int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* local structures */

struct subinfo {
	FIFOSTR		fs ;
	int		wopts ;
	int		f_exit ;
	int		(*uf)() ;
	void		*uarg ;
} ;


/* forward references */

static int subinfo_start(SUBINFO *,int,int (*)(),void *) ;
static int subinfo_finish(SUBINFO *) ;
static int subinfo_procdir(SUBINFO *,char *,int) ;
static int subinfo_procdirents(SUBINFO *,char *,int) ;
static int subinfo_procname(SUBINFO *,cchar *,int) ;
static int subinfo_procout(SUBINFO *,cchar *,struct ustat *) ;


/* local variables */


/* exported subroutines */


int wdt(basedir,wopts,uf,uarg)
const char	basedir[] ;
int		wopts ;
int		(*uf)() ;
void		*uarg ;
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("wdt: ent\n") ;
#endif

	if (basedir == NULL) return SR_FAULT ;
	if (uf == NULL) return SR_FAULT ;

	if (basedir[0] == '\0') return SR_INVALID ;

/* go */

	if ((rs = subinfo_start(sip,wopts,uf,uarg)) >= 0) {
	    char	dbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath1(dbuf,basedir)) >= 0) {
	        if ((rs = subinfo_procdir(sip,dbuf,rs)) >= 0) {
	            const int	dlen = MAXPATHLEN ;
	            c += rs ;
	            while ((rs >= 0) && (! sip->f_exit)) {
	                rs1 = fifostr_remove(&sip->fs,dbuf,dlen) ;
	                if (rs1 == SR_NOTFOUND) break ;
	                rs = rs1 ;
	                if (rs >= 0) {
	                    rs = subinfo_procdir(sip,dbuf,rs1) ;
	                    c += rs ;
	                }
	            } /* end while */
	        } /* end if (subinfo_procdir) */
	    } /* end if (mkpath) */
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("wdt: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (wdt) */


/* private subroutines */


static int subinfo_start(SUBINFO *sip,int wopts,int (*uf)(),void *uarg)
{
	int		rs ;
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->wopts = wopts ;
	sip->uf = uf ;
	sip->uarg = uarg ;
	rs = fifostr_start(&sip->fs) ;
	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = fifostr_finish(&sip->fs) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_procdir(SUBINFO *sip,char *dbuf,int dlen)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("wdt/procdir: ent d=%t\n",dbuf,dlen) ;
#endif
	if ((rs = subinfo_procdirents(sip,dbuf,dlen)) >= 0) {
	    struct ustat	sb ;
	    c += rs ;
	    if ((rs = uc_stat(dbuf,&sb)) >= 0) {
	        rs = subinfo_procout(sip,dbuf,&sb) ;
		c += rs ;
	    }
	} /* end if (subinfo_procdirents) */
#if	CF_DEBUGS
	debugprintf("wdt/procdir: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_procdir) */


static int subinfo_procdirents(SUBINFO *sip,char *dbuf,int dlen)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("wdt/procdirents: ent d=%t\n",dbuf,dlen) ;
#endif
	if ((rs = fsdir_open(&d,dbuf)) >= 0) {
	    while ((rs = fsdir_read(&d,&de)) > 0) {
	        if (hasNotDots(de.name,-1)) {
	            if ((rs = pathadd(dbuf,dlen,de.name)) >= 0) {
	                rs = subinfo_procname(sip,dbuf,rs) ;
	                c += rs ;
	            }
	        }
	        if (sip->f_exit) break ;
	        if (rs < 0) break ;
	    } /* end while */
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
	dbuf[dlen] = '\0' ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_procdirents) */


static int subinfo_procname(SUBINFO *sip,cchar *dbuf,int dlen)
{
	struct ustat	sb ;
	int		rs ;
#if	CF_DEBUGS
	debugprintf("wdt/procname: ent d=%t\n",dbuf,dlen) ;
#endif
	if ((rs = lstat(dbuf,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        rs = fifostr_add(&sip->fs,dbuf,dlen) ;
	    } else if (S_ISLNK(sb.st_mode)) {
	        if (sip->wopts & WDT_MFOLLOW) {
	            struct ustat	sbb ;
	            if ((rs = u_stat(dbuf,&sbb)) >= 0) {
	                if (S_ISDIR(sbb.st_mode)) {
	                    rs = fifostr_add(&sip->fs,dbuf,dlen) ;
	                } else {
	                    rs = subinfo_procout(sip,dbuf,&sbb) ;
	                }
	            } else if (isNotAccess(rs)) {
	                memset(&sb,0,sizeof(struct ustat)) ;
	                rs = subinfo_procout(sip,dbuf,&sb) ;
	            }
	        } else {
	            rs = subinfo_procout(sip,dbuf,&sb) ;
	        }
	    } else {
	            rs = subinfo_procout(sip,dbuf,&sb) ;
	    }
	} else if (rs == SR_NOENT) {
	    memset(&sb,0,sizeof(struct ustat)) ;
	    rs = subinfo_procout(sip,dbuf,&sb) ;
	}
#if	CF_DEBUGS
	debugprintf("wdt/procname: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_procname) */


static int subinfo_procout(SUBINFO *sip,cchar *dbuf,struct ustat *sbp)
{
	int		rs = SR_OK ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("wdt/procout: ent d=%s\n",dbuf) ;
#endif
	if (sip->uf != NULL) {
	    if ((rs = (*sip->uf)(dbuf,sbp,sip->uarg)) >= 0) {
		c = 1 ;
	        if (rs > 0) sip->f_exit = TRUE ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("wdt/procout: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (subinfo_procout) */


