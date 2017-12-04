/* pwfile */

/* open a 'passwd' formatted file for access */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine (object) module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module manipulates passsword records in a 'passwd' type file.

	No, it is not particularly pretty but such is life often times!


*******************************************************************************/


#define	PWFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<storeitem.h>
#include	<gecos.h>
#include	<pwentry.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"pwfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	DEFENTRIES	10
#define	TO_LOCK		60

#ifndef	F_LOCK
#define	F_ULOCK	0
#define	F_LOCK	1
#define	F_TLOCK	2
#define	F_TEST	3
#endif


/* external subroutines */

extern int	snwcpyhyphen(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecl(const char *,int,long *) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	pwfile_loadbegin(PWFILE *) ;
static int	pwfile_loadend(PWFILE *) ;
static int	pwfile_filefront(PWFILE *) ;
static int	pwfile_filefronter(PWFILE *) ;
static int	pwfile_fileback(PWFILE *) ;

static int	pwentry_start(PWENTRY *) ;
static int	pwentry_finish(PWENTRY *) ;
static int	pwentry_fieldpw(PWENTRY *,int,const char *,int) ;
static int	pwentry_mkextras(PWENTRY *) ;
static int	pwentry_mkcopy(PWENTRY *,PWENTRY *,char *,int) ;

static int	loaditem(const char **,const char *,int) ;


/* local variables */


/* exported subroutines */


int pwfile_open(PWFILE *dbp,cchar *pwfname)
{
	int		rs ;
	const char	*cp ;

	if (dbp == NULL) return SR_FAULT ;
	if (pwfname == NULL) return SR_FAULT ;

	if (pwfname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_mallocstrw(pwfname,-1,&cp)) >= 0) {
	    dbp->fname = cp ;
	    dbp->lfd = -1 ;
	    if ((rs = pwfile_loadbegin(dbp)) >= 0) {
	        memset(&dbp->f,0,sizeof(struct pwfile_flags)) ;
	        dbp->magic = PWFILE_MAGIC ;
	    }
	    if (rs < 0) {
	        uc_free(dbp->fname) ;
	        dbp->fname = NULL ;
	    }
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (pwfile_open) */


/* close the file */
int pwfile_close(PWFILE *dbp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (dbp == NULL) return SR_FAULT ;

	if (dbp->magic != PWFILE_MAGIC) return SR_NOTOPEN ;

	rs1 = pwfile_loadend(dbp) ;
	if (rs >= 0) rs = rs1 ;

	if (dbp->lfd >= 0) {
	    rs1 = u_close(dbp->lfd) ;
	    if (rs >= 0) rs = rs1 ;
	    dbp->lfd = -1 ;
	}

	if (dbp->fname != NULL) {
	    rs1 = uc_free(dbp->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    dbp->fname = NULL ;
	}

	dbp->magic = 0 ;
	return rs ;
}
/* end subroutine (pwfile_close) */


/* enumerate the entries */
int pwfile_enum(PWFILE *dbp,PWFILE_CUR *curp,PWENTRY *uep,char *rbuf,int rlen)
{
	PWENTRY		*ep ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("pwfile_enum: ent &pe %08lX pebuf %08lX\n",
	    uep,rbuf) ;
#endif

	if (dbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (dbp->magic != PWFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("pwfile_enum: about to enum\n") ;
#endif

	while ((rs = vecitem_get(&dbp->alist,curp->i,&ep)) >= 0) {
#if	CF_DEBUGS
	    debugprintf("pwfile_enum: u=%s\n",
	        (ep != NULL) ? ep->username : "NULL") ;
#endif
	    curp->i += 1 ;
	    if (ep != NULL) break ;
	} /* end while */

/* load up the user's structure with the information from our internal one */

#if	CF_DEBUGS
	debugprintf("pwfile_enum: getcopy buflen=%d u=%s\n",
	    buflen,ep->username) ;
	uep->username = (char *) 0x5a5a5a5a ;
#endif

	if (rs >= 0) {
	    rs = pwentry_mkcopy(ep,uep,rbuf,rlen) ;
	} else if (rs == SR_NOTFOUND) {
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	debugprintf("pwfile_enum: &username %08lX\n",uep->username) ;
	debugprintf("pwfile_enum: ret rs=%d u=%s\n",rs,uep->username) ;
#endif

	return rs ;
}
/* end subroutine (pwfile_enum) */


/* fetch the next entry that matches the specified username */
int pwfile_fetchuser(dbp,username,curp,uep,rbuf,rlen)
PWFILE		*dbp ;
const char	username[] ;
PWFILE_CUR	*curp ;
PWENTRY		*uep ;
char		rbuf[] ;
int		rlen ;
{
	struct ustat	sb ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("pwfile_fetchuser: ent w/ \"%s\"\n",username) ;
#endif

	if (dbp == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	if (dbp->magic != PWFILE_MAGIC) return SR_NOTOPEN ;

	if (username[0] == '\0') return SR_FAULT ;

/* has the underlying object changed? */

	if (dbp->lfd < 0) {
	    rs = u_stat(dbp->fname,&sb) ;
	} else {
	    rs = u_fstat(dbp->lfd,&sb) ;
	}

	if (rs >= 0) {
	    HDB_DATUM	key, val ;
	    HDB_CUR	*hcurp = ((curp != NULL) ? &curp->hc : NULL) ;

	    if (sb.st_mtime > dbp->readtime) {
	        pwfile_loadend(dbp) ;
	        pwfile_loadbegin(dbp) ;
	    } /* end if */

/* continue with the query */

	    key.buf = (char *) username ;
	    key.len = strlen(username) ;

#if	CF_DEBUGS
	    debugprintf("pwfile_fetchuser: about to fetch\n") ;
#endif

	    if ((rs = hdb_fetch(&dbp->byuser,key,hcurp,&val)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("pwfile_fetchuser: fetched rs=%d\n",rs) ;
#endif

	        if (uep != NULL) {
	            PWENTRY	*ep = (PWENTRY *) val.buf ;

#if	CF_DEBUGS
	            debugprintf("pwfile_fetchuser: &pe %08lX\n",ep) ;
	            debugprintf("pwfile_fetchuser: u=%s\n",ep->username) ;
#endif

	            rs = pwentry_mkcopy(ep,uep,rbuf,rlen) ;

	        }
	    } /* end if (hdb_fetch) */

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (pwfile_fetchuser) */


/* initialize a cursor for the DB */
int pwfile_curbegin(PWFILE *dbp,PWFILE_CUR *curp)
{
	int		rs ;
	int		f_locked = FALSE ;

	if (dbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (dbp->magic != PWFILE_MAGIC) return SR_NOTOPEN ;

	if (dbp->lfd < 0) {
	    rs = u_open(dbp->fname,O_RDONLY,0666) ;
	    dbp->lfd = rs ;
	} /* end if (it wasn't open) */

	if (rs >= 0) {

	    if (! dbp->f.locked) {

	        if ((rs = lockfile(dbp->lfd,F_RLOCK,0L,0L,TO_LOCK)) < 0) {
	            u_close(dbp->lfd) ;
	            dbp->lfd = -1 ;
	            return rs ;
	        }

	        dbp->f.locked = dbp->f.locked_cur = TRUE ;
	        f_locked = TRUE ;

	    } /* end if (not locked) */

	    curp->i = 0 ;
	    rs = hdb_curbegin(&dbp->byuser,&curp->hc) ;

	    if ((rs < 0) && f_locked) {

	        dbp->f.locked = dbp->f.locked_cur = FALSE ;

#ifdef	COMMENT
	        lockfile(dbp->lfd,F_ULOCK,0L,0L,TO_LOCK) ;
#else
	        u_close(dbp->lfd) ;
	        dbp->lfd = -1 ;
#endif

	    } /* end if (cleanup on error) */

	} /* end if (ok) */

	return rs ;
}
/* end subroutine (pwfile_curbegin) */


/* free up a cursor for the DB */
int pwfile_curend(PWFILE *dbp,PWFILE_CUR *curp)
{
	int		rs ;

	if (dbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (dbp->magic != PWFILE_MAGIC) return SR_NOTOPEN ;

	if (dbp->f.locked_cur && (! dbp->f.locked_explicit)) {
	    dbp->f.locked = FALSE ;
	    lockfile(dbp->lfd,F_ULOCK,0L,0L,TO_LOCK) ;
	}

	dbp->f.locked_cur = FALSE ;
	curp->i = 0 ;
	rs = hdb_curend(&dbp->byuser,&curp->hc) ;

	return rs ;
}
/* end subroutine (pwfile_curend) */


/* lock the DB for read or update */
int pwfile_lock(PWFILE *dbp,int type,int to_lock)
{
	int		rs = SR_OK ;
	int		f_opened = FALSE ;

	if (dbp == NULL) return SR_FAULT ;

	if (dbp->magic != PWFILE_MAGIC) return SR_NOTOPEN ;

	if (dbp->lfd < 0) {
	    rs = u_open(dbp->fname,O_RDONLY,0666) ;
	    dbp->lfd = rs ;
	    f_opened = (rs >= 0) ;
	}

	if (rs >= 0) {
	    switch (type) {
	    case F_ULOCK:
	        if (dbp->f.locked_explicit) {
	            dbp->f.locked = dbp->f.locked_explicit = FALSE ;
	            rs = lockfile(dbp->lfd,type,0L,0L,to_lock) ;
	            if (rs < 0) {
	                u_close(dbp->lfd) ;
	                dbp->lfd = -1 ;
	            }
	        } else {
	            rs = SR_INVALID ;
	        }
	        break ;
	    default:
	    case F_RTEST:
	    case F_WTEST:
	        rs = SR_LOCKED ;
	        if (! dbp->f.locked) {
	            rs = lockfile(dbp->lfd,type,0L,0L,to_lock) ;
#ifdef	COMMENT
	            if (f_opened) {
	                u_close(dbp->lfd) ;
	                dbp->lfd = -1 ;
	            }
#endif /* COMMENT */
	        } /* end if (not locked already) */
	        break ;
	    case F_RLOCK:
	    case F_WLOCK:
	    case F_TRLOCK:
	    case F_TWLOCK:
	        if (! dbp->f.locked) {
	            if ((rs = lockfile(dbp->lfd,type,0L,0L,to_lock)) >= 0) {
	                dbp->f.locked = dbp->f.locked_explicit = TRUE ;
	            }
	        } else {
	            rs = SR_INVALID ;
	        }
	        break ;
	    } /* end switch */
	} /* end if (ok) */

	return (rs >= 0) ? f_opened : rs ;
}
/* end subroutine (pwfile_lock) */


/* private subroutines */


/* load up the database */
static int pwfile_loadbegin(PWFILE *dbp)
{
	int		rs ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("pwfile_loadbegin: ent\n") ;
#endif

	if ((rs = pwfile_filefront(dbp)) >= 0) {
	    HDB_DATUM	key, val ;
	    n = rs ;
	    if ((rs = hdb_start(&dbp->byuser,n,0,NULL,NULL)) >= 0) {
	        VECITEM	*alp = &dbp->alist ;
	        PWENTRY	*ep ;
	        int	i ;
	        for (i = 0 ; vecitem_get(alp,i,&ep) >= 0 ; i += 1) {
	            if (ep != NULL) {
	                key.buf = ep->username ;
	                key.len = strlen(ep->username) ;
	                val.buf = ep ;
	                val.len = sizeof(PWENTRY) ;
	                rs = hdb_store(&dbp->byuser,key,val) ;
	            }
	            if (rs < 0) break ;
	        } /* end for */
	        if (rs < 0) {
	            hdb_finish(&dbp->byuser) ;
	        }
	    }
	    if (rs < 0) {
	        pwfile_fileback(dbp) ;
	    }
	} /* end if (pwfile_filefront) */

#if	CF_DEBUGS
	debugprintf("pwfile_loadbegin: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (pwfile_loadbegin) */


/* unload the data-base */
static int pwfile_loadend(PWFILE *dbp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = hdb_finish(&dbp->byuser) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = pwfile_fileback(dbp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pwfile_loadend) */


/* load the contents of the file into the appropriate structures */
static int pwfile_filefront(PWFILE *dbp)
{
	USTAT		sb ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("pwfile_filefront: ent\n") ;
#endif

	if (dbp->fname[0] == '\0') return SR_NOENTRY ;

	if ((rs = uc_stat(dbp->fname,&sb)) >= 0) {
	    VECITEM	*alp = &dbp->alist ;
	    const int	vo = VECITEM_PNOHOLES ;
	    int		n = ((sb.st_size / 60) + 5) ;
	    if (n < DEFENTRIES) n = DEFENTRIES ;
	    if ((rs = vecitem_start(alp,n,vo)) >= 0) {
	        dbp->readtime = sb.st_mtime ;
	        rs = pwfile_filefronter(dbp) ;
	        if (rs < 0)
	            vecitem_finish(alp) ;
	    }
	} /* end if (uc_stat) */

	return rs ;
}
/* end subroutine (pwfile_filefront) */


static int pwfile_filefronter(PWFILE *dbp)
{
	bfile		pwfile, *fp = &pwfile ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if ((rs = bopen(fp,dbp->fname,"rc",0644)) >= 0) {
	    if (! dbp->f.locked) {
	        rs = bcontrol(fp,BC_LOCKREAD,TO_LOCK) ;
	    }
	    if (rs >= 0) {
	        PWENTRY		entry ;
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN+1] ;
	        while ((rs = breadline(fp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

#if	CF_DEBUGS
	            debugprintf("pwfile_filefront: read> %t\n",lbuf,len) ;
#endif

	            if ((rs = pwentry_start(&entry)) >= 0) {
	                int		fn = 0 ;
	                cchar		*tp ;
	                cchar		*cp = lbuf ;
	                while ((tp = strchr(cp,':')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("pwfile_filefront: %02d field> %t\n",
	                        fn,cp,(tp - cp)) ;
#endif

	                    rs = pwentry_fieldpw(&entry,fn,cp,(tp - cp)) ;

	                    cp = (tp + 1) ;
	                    fn += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	                if ((rs >= 0) && (cp[0] != '\0')) {
	                    rs = pwentry_fieldpw(&entry,fn,cp,-1) ;
	                }

#if	CF_DEBUGS
	                debugprintf("pwfile_filefront: created entry u=%s\n",
	                    entry.username) ;
#endif

/* make any extras fields that we want */

	                if (rs >= 0) {
	                    rs = pwentry_mkextras(&entry) ;
#if	CF_DEBUGS
	                    debugprintf("pwfile_filefront: "
	                        "pwentry_mkextras() rs=%d\n",rs) ;
#endif
	                }

/* add the entry to our list */

	                if (rs >= 0) {
	                    const int	esize = sizeof(PWENTRY) ;
	                    n += 1 ;
	                    rs = vecitem_add(&dbp->alist,&entry,esize) ;
	                }

	                if (rs < 0)
	                    pwentry_finish(&entry) ;
	            } /* end if (initialized new entry) */

	            if (rs < 0) break ;
	        } /* end while (reading file entries) */
	        if (! dbp->f.locked) {
	            bcontrol(fp,BC_UNLOCK,0) ;
	        }
	    } /* end if (ok) */
	    rs1 = bclose(fp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

#if	CF_DEBUGS
	debugprintf("pwfile_filefront: ret rs=%d n=%d\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (pwfile_filefronter) */


/* free up the resources occupied by loading of the file */
static int pwfile_fileback(PWFILE *dbp)
{
	VECITEM		*alp = &dbp->alist ;
	PWENTRY		*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecitem_get(alp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = pwentry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

/* free up the container object itself */

	rs1 = vecitem_finish(alp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (pwfile_fileback) */


#ifdef	COMMENT

An actual 'passwd' entry in the file has the fields (in order):

name
passwd
uid
gid
gecos
dir
shell
lstchg		/* last change (in days) */
min		/* minimum number of days between password changes */
max		/* number of days password is valid */
warn		/* number of days to warn user to change passwd */
inact		/* number of days the login may be inactive */
expire		/* date when the login is no longer valid */
flag		/* currently not being used */

These fields are loaded into the following standard structures:

struct passwd {
	char	*pw_name ;
	char	*pw_passwd ;
	uid_t	pw_uid ;
	gid_t	pw_gid ;
	char	*pw_age ;
	char	*pw_comment ;
	char	*pw_gecos ;
	char	*pw_dir ;
	char	*pw_shell ;
} ;

struct spwd {
	char *sp_namp;	/* user name */
	char *sp_pwdp;	/* user password */
	long sp_lstchg;	/* password lastchanged date */
	long sp_min;	/* minimum number of days between password changes */
	long sp_max;	/* number of days password is valid */
	long sp_warn;	/* number of days to warn user to change passwd */
	long sp_inact;	/* number of days the login may be inactive */
	long sp_expire;	/* date when the login is no longer valid */
	unsigned long sp_flag;	/* currently not being used */
} ;

A typical 'passwd' file entry looks like:

dam:
gWI5lTXfCPT6g:
201:
309:
d.a.morano:
/home/dam:
/bin/ksh:
10990:
-1:
-1:
-1:
-1:
:
0

#endif /* COMMENT */


/* initialize a 'passwd' entry */
static int pwentry_start(PWENTRY *ep)
{

	memset(ep,0,sizeof(PWENTRY)) ;
	ep->lstchg = -1 ;

	return SR_OK ;
}
/* end subroutine (pwentry_start) */


static int pwentry_fieldpw(PWENTRY *ep,int fn,cchar *s,int slen)
{
	int		v ;
	const char	*mp = NULL ;

	if (slen < 0)
	    slen = strlen(s) ;

#if	CF_DEBUGS
	debugprintf("pwentry_field: %02d %t\n",fn,s,slen) ;
#endif

	switch (fn) {
	case 0:
	    ep->username = mp = mallocstrw(s,slen) ;
	    break ;
	case 1:
	    ep->password = mp = mallocstrw(s,slen) ;
	    break ;
	case 2:
	    mp = s ;
	    ep->uid = -1 ;
	    if ((slen > 0) && (cfdeci(s,slen,&v) >= 0)) ep->uid = v ;
	    break ;
	case 3:
	    mp = s ;
	    ep->gid = -1 ;
	    if ((slen > 0) && (cfdeci(s,slen,&v) >= 0)) ep->gid = v ;
	    break ;
	case 4:
	    ep->gecos = mp = mallocstrw(s,slen) ;
	    break ;
	case 5:
	    ep->dir = mp = mallocstrw(s,slen) ;
	    break ;
	case 6:
	    ep->shell = mp = mallocstrw(s,slen) ;
	    break ;
	case 7:
	    mp = s ;
	    ep->lstchg = -1 ;
	    if (slen > 0) cfdecl(s,slen,&ep->lstchg) ;
	    break ;
	case 8:
	    mp = s ;
	    ep->min = -1 ;
	    if (slen > 0) cfdecl(s,slen,&ep->min) ;
	    break ;
	case 9:
	    mp = s ;
	    ep->max = -1 ;
	    if (slen > 0) cfdecl(s,slen,&ep->max) ;
	    break ;
	case 10:
	    mp = s ;
	    ep->warn = -1 ;
	    if (slen > 0) cfdecl(s,slen,&ep->warn) ;
	    break ;
	case 11:
	    mp = s ;
	    ep->inact = -1 ;
	    if (slen > 0) cfdecl(s,slen,&ep->inact) ;
	    break ;
	case 12:
	    mp = s ;
	    ep->expire = -1 ;
	    if (slen > 0) cfdecl(s,slen,&ep->expire) ;
	    break ;
	case 13:
	    mp = s ;
	    if (slen > 0) cfdecl(s,slen,(long *) &ep->flag) ;
	    break ;
	default:
	    mp = s ;
	    break ;
	} /* end switch */

	return ((mp == NULL) ? SR_NOMEM : SR_OK) ;
}
/* end subroutine (pwentry_fieldpw) */


/* make the extra entries that we have all grown accustomed to */
static int pwentry_mkextras(PWENTRY *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->gecos != NULL) {
	    GECOS	g ;
	    if ((rs = gecos_start(&g,ep->gecos,-1)) >= 0) {
	        int	i ;
	        int	vl ;
	        cchar	*vp ;
	        for (i = 0 ; i < gecosval_overlast ; i += 1) {
	            if ((vl = gecos_getval(&g,i,&vp)) >= 0) {
	                if (vp != NULL) {
	                    void	*p ;

#if	CF_DEBUGS
	                    debugprintf("pwfile/pwentry_mkextras: "
	                        "i=%d v=>%t<\n",
	                        i,vp,strlinelen(vp,vl,40)) ;
#endif
	                    switch (i) {
	                    case gecosval_organization:
	                        rs = loaditem(&ep->organization,vp,vl) ;
	                        break ;
	                    case gecosval_realname:
	                        if ((rs = uc_malloc((vl+1),&p)) >= 0) {
	                            char	*nbuf = (char *) p ;
	                            if (strnchr(vp,vl,'_') != NULL) {
	                                rs = snwcpyhyphen(nbuf,-1,vp,vl) ;
	                                vp = nbuf ;
	                            }
	                            if (rs >= 0) {
	                                rs = loaditem(&ep->realname,vp,vl) ;
	                            }
	                            uc_free(p) ;
	                        } /* end if (m-a-f) */
	                        break ;
	                    case gecosval_account:
	                        rs = loaditem(&ep->account,vp,vl) ;
	                        break ;
	                    case gecosval_bin:
	                        rs = loaditem(&ep->bin,vp,vl) ;
	                        break ;
	                    case gecosval_office:
	                        rs = loaditem(&ep->office,vp,vl) ;
	                        break ;
	                    case gecosval_wphone:
	                        rs = loaditem(&ep->wphone,vp,vl) ;
	                        break ;
	                    case gecosval_hphone:
	                        rs = loaditem(&ep->hphone,vp,vl) ;
	                        break ;
	                    case gecosval_printer:
	                        rs = loaditem(&ep->printer,vp,vl) ;
	                        break ;
	                    } /* end switch */

	                } /* end if (non-NULL) */
	            } /* end if (gecos-getval) */
	            if (rs < 0) break ;
	        } /* end for */
	        rs1 = gecos_finish(&g) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (gecos) */
	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (pwentry_mkextras) */


/* load a user entry from an internal one */
static int pwentry_mkcopy(PWENTRY *ep,PWENTRY *uep,char *rbuf,int rlen)
{
	STOREITEM	ubuf ;
	int		rs ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("pwfile_getcopy: ent, &pe %08lX buflen=%d\n",uep,rlen) ;
#endif

	memcpy(uep,ep,sizeof(PWENTRY)) ;

	if ((rs = storeitem_start(&ubuf,rbuf,rlen)) >= 0) {

	    if (ep->username != NULL) {
	        storeitem_strw(&ubuf,ep->username,-1,&uep->username) ;
	    }

	    if (ep->password != NULL)
	        storeitem_strw(&ubuf,ep->password,-1,&uep->password) ;

#ifdef	OPTIONAL
	    uep->uid = ep->uid ;
	    uep->gid = ep->gid ;
#endif

	    if (ep->gecos != NULL)
	        storeitem_strw(&ubuf,ep->gecos,-1,&uep->gecos) ;

/* the standard AT&T stuff */

	    if (ep->organization != NULL)
	        storeitem_strw(&ubuf,ep->organization,-1,&uep->organization) ;

	    if (ep->realname != NULL)
	        storeitem_strw(&ubuf,ep->realname,-1,&uep->realname) ;

	    if (ep->account != NULL)
	        storeitem_strw(&ubuf,ep->account,-1,&uep->account) ;

	    if (ep->bin != NULL)
	        storeitem_strw(&ubuf,ep->bin,-1,&uep->bin) ;

/* the finger information stuff */

	    if (ep->office != NULL)
	        storeitem_strw(&ubuf,ep->office,-1,&uep->office) ;

	    if (ep->wphone != NULL)
	        storeitem_strw(&ubuf,ep->wphone,-1,&uep->wphone) ;

	    if (ep->hphone != NULL)
	        storeitem_strw(&ubuf,ep->hphone,-1,&uep->hphone) ;

	    if (ep->printer != NULL)
	        storeitem_strw(&ubuf,ep->printer,-1,&uep->printer) ;

/* the remaining standard password stuff */

	    if (ep->dir != NULL)
	        storeitem_strw(&ubuf,ep->dir,-1,&uep->dir) ;

	    if (ep->shell != NULL)
	        storeitem_strw(&ubuf,ep->shell,-1,&uep->shell) ;

/* done */

	    len = storeitem_finish(&ubuf) ;
	    if (rs >= 0) rs = len ;
	} /* end if (storeitem) */

#if	CF_DEBUGS
	debugprintf("pwfile_getcopy: ret &username %08lX u=%s\n",
	    uep->username,uep->username) ;
	debugprintf("pwfile_getcopy: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (pwentry_mkcopy) */


/* free up a 'passwd' entry */
static int pwentry_finish(PWENTRY *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

/* the regulars */

	if (ep->username != NULL) {
	    rs1 = uc_free(ep->username) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->password != NULL) {
	    rs1 = uc_free(ep->password) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->gecos != NULL) {
	    rs1 = uc_free(ep->gecos) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->dir != NULL) {
	    rs1 = uc_free(ep->dir) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->shell != NULL) {
	    rs1 = uc_free(ep->shell) ;
	    if (rs >= 0) rs = rs1 ;
	}

/* the AT&T standard extras */

	if (ep->organization != NULL) {
	    rs1 = uc_free(ep->organization) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->realname != NULL) {
	    rs1 = uc_free(ep->realname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->account != NULL) {
	    rs1 = uc_free(ep->account) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->bin != NULL) {
	    rs1 = uc_free(ep->bin) ;
	    if (rs >= 0) rs = rs1 ;
	}

/* the sometimes pseudo-standard finger information fields */

	if (ep->office != NULL) {
	    rs1 = uc_free(ep->office) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->wphone != NULL) {
	    rs1 = uc_free(ep->wphone) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->hphone != NULL) {
	    rs1 = uc_free(ep->hphone) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (ep->printer != NULL) {
	    rs1 = uc_free(ep->printer) ;
	    if (rs >= 0) rs = rs1 ;
	}

	memset(ep,0,sizeof(PWENTRY)) ;

	return rs ;
}
/* end subroutine (pwentry_finish) */


static int loaditem(cchar **rpp,cchar *vp,int vl)
{
	int		rs = SR_FAULT ;

	if (vp != NULL) {
	    rs = uc_mallocstrw(vp,vl,rpp) ;
	}

#if	CF_DEBUGS
	debugprintf("pwfile/loaditem: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (loaditem) */


