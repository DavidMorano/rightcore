/* mxalias */

/* manage a MXALIAS object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGFILE	0		/* file parsing */
#define	CF_SAFE		1		/* safe mode */
#define	CF_DEVINO	1		/* check device-inode */
#define	CF_FILECHECK	0		/* enable file-checking */
#define	CF_FILESADD	1		/* files-add */
#define	CF_FILESYS	1		/* system-DB */
#define	CF_FILEUSER	1		/* user-DB */


/* revision history:

	= 2003-06-11, David A­D­ Morano
	I snarfed this file from some hardware research use since it seemed be
	a pretty good match for the present algorithm needs.  We'll see how it
	works out! :-|

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module is used to manage a MXALIAS object.


*******************************************************************************/


#define	MXALIAS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"mxalias.h"


/* local defines */

#define	MXALIAS_FILE	struct mxalias_file
#define	MXALIAS_SYSDB	"/etc/mail/mailx.rc"
#define	MXALIAS_USERDB	".mailrc"
#define	MXALIAS_KEYWORD	"alias"

#define	BUFDESC		struct bufdesc

#define	VARUSERNAME	"USERNAME"
#define	VARHOME		"HOME"

#define	TO_APROFILE	(1 * 60)
#define	TO_FILECOME	15		/* timeout for file to "come in" */
#define	TO_LOCK		(5 * 60)
#define	TO_OPEN		(60 * 60)
#define	TO_ACCESS	(1 * 60)
#define	TO_CHECK	5		/* minimum check interval */
#define	TO_FILECHANGED	5		/* DB file check */
#define	TO_FILEOLD	10		/* backing-store check */

#ifndef	ENDIAN
#if	defined(OSNAME_SunOS) && defined(__sparc)
#define	ENDIAN		1
#else
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifndef	ENDIAN
#error	"could not determine endianness of this machine"
#endif
#endif
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	ALIASNAMELEN
#define	ALIASNAMELEN	64
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	KEYBUFLEN	80


/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getpwds(struct ustat *,char *,int) ;
extern int	getpwd(char *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	isalphalatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* local structures */

struct mxalias_file {
	const char	*fname ;
	uino_t		ino ;
	dev_t		dev ;
	time_t		mtime ;
	int		size ;
} ;

struct bufdesc {
	const char	*a ;		/* memory allocation */
	char		*lbuf ;
	char		*fbuf ;
	char		*kbuf ;
	int		llen ;
	int		flen ;
	int		klen ;
} ;

/* forward references */

static int	mxalias_username(MXALIAS *,const char *) ;
static int	mxalias_userdname(MXALIAS *) ;
static int	mxalias_mkuserfname(MXALIAS *,char *) ;
static int	mxalias_filesadd(MXALIAS *,time_t) ;
static int	mxalias_fileadd(MXALIAS *,const char *) ;
static int	mxalias_filereg(MXALIAS *,struct ustat *,const char *) ;
static int	mxalias_fileparse(MXALIAS *,int) ;
static int	mxalias_fileparser(MXALIAS *,int,bfile *) ;
static int	mxalias_fileparseline(MXALIAS *,int,BUFDESC *,cchar *,int) ;
static int	mxalias_filedump(MXALIAS *,int) ;
static int	mxalias_filedels(MXALIAS *) ;
static int	mxalias_filedel(MXALIAS *,int) ;
static int	mxalias_entfins(MXALIAS *) ;
static int	mxalias_mkvals(MXALIAS *,MXALIAS_CUR *,vecstr *) ;
static int	mxalias_addvals(MXALIAS *,vecstr *,vecstr *,const char *) ;
static int	mxalias_allocfins(MXALIAS *) ;

static int	mxalias_fileparseline_alias(MXALIAS *,int,BUFDESC *,FIELD *) ;
static int	mxalias_fileparseline_unalias(MXALIAS *,int,BUFDESC *,FIELD *) ;
static int	mxalias_fileparseline_source(MXALIAS *,int,BUFDESC *,FIELD *) ;

#ifdef	COMMENT
static int	mxalias_filealready(MXALIAS *,dev_t,uino_t) ;
#endif

#if	CF_FILECHECK
static int	mxalias_filechecks(MXALIAS *,time_t) ;
#endif

static int	file_start(struct mxalias_file *,struct ustat *,const char *) ;
static int	file_finish(struct mxalias_file *) ;

static int	bufdesc_start(BUFDESC *,int) ;
static int	bufdesc_finish(BUFDESC *) ;

static int	isnotspecial(int) ;
static int	isOurFileType(mode_t) ;

static int	vcmpfe() ;


/* local variables */

static const uchar	kterms[] = {
	0x00, 0x26, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

/* all white space plus comma (',') */
static const uchar	vterms[] = {
	0x00, 0x26, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*keywords[] = {
	"alias",
	"group",
	"unalias",
	"ungroup",
	"source",
	NULL
} ;

enum keywords {
	keyword_alias,
	keyword_group,
	keyword_unalias,
	keyword_ungroup,
	keyword_source,
	keyword_overlast
} ;


/* exported subroutines */


int mxalias_open(MXALIAS *op,cchar *pr,cchar *username)
{
	int		rs ;
	int		c = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mxalias_open: pr=%s u=%s\n",pr,username) ;
#endif

	memset(op,0,sizeof(MXALIAS)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    if ((rs = mxalias_username(op,username)) >= 0) {
	        const int	opts = (VECOBJ_OSTATIONARY | VECOBJ_OREUSE) ;
	        const int	size = sizeof(MXALIAS_FILE) ;
	        if ((rs = vecobj_start(&op->files,size,5,opts)) >= 0) {
	            if ((rs = keyvals_start(&op->entries,5)) >= 0) {
	                time_t	daytime = time(NULL) ;
	                op->ti_access = daytime ;
	                op->ti_check = daytime ;
	                op->magic = MXALIAS_MAGIC ;
#if	CF_FILESADD
	                if ((rs = mxalias_filesadd(op,daytime)) >= 0) {
	                    c = rs ;
	                }
#endif /* CF_FILESADD */
	                if (rs < 0) {
	                    mxalias_entfins(op) ;
	                    keyvals_finish(&op->entries) ;
	                    op->magic = 0 ;
	                }
	            } /* end if (entries) */
	            if (rs < 0) {
	                mxalias_filedels(op) ;
	                vecobj_finish(&op->files) ;
	            }
	        } /* end if (files) */
	    } /* end if (username) */
	    if (rs < 0)
	        mxalias_allocfins(op) ;
	} /* end if (program-root) */

#if	CF_DEBUGS
	debugprintf("mxalias_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_open) */


/* free up this mxalias object */
int mxalias_close(MXALIAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;

	rs1 = mxalias_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = keyvals_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mxalias_filedels(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mxalias_allocfins(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mxalias_close) */


/* get the string count in the table */
int mxalias_audit(MXALIAS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;

	return rs ;
}
/* end subroutine (mxalias_audit) */


/* get the string count in the table */
int mxalias_count(MXALIAS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;

	rs = keyvals_count(&op->entries) ;

	return rs ;
}
/* end subroutine (mxalias_count) */


/* initialize a cursor */
int mxalias_curbegin(MXALIAS *op,MXALIAS_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(MXALIAS_CUR)) ;
	curp->i = -1 ;
	curp->vbuf = NULL ;
	curp->vals = NULL ;
	curp->nvals = 0 ;
	curp->magic = MXALIAS_MAGIC ;

	op->ncursors += 1 ;

	return rs ;
}
/* end subroutine (mxalias_curbegin) */


/* free up a cursor */
int mxalias_curend(MXALIAS *op,MXALIAS_CUR *curp)
{
	const time_t	daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;

	if (op->f.cursoracc)
	    op->ti_access = daytime ;

	curp->i = -1 ;
	curp->nvals = 0 ;
	if (curp->vbuf != NULL) {
	    rs1 = uc_free(curp->vbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->vbuf = NULL ;
	}

	if (curp->vals != NULL) {
	    rs1 = uc_free(curp->vals) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->vals = NULL ;
	}

	if (curp->kvcp != NULL) {
	    rs1 = keyvals_curend(&op->entries,curp->kvcp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(curp->kvcp) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->kvcp = NULL ;
	}

	op->ncursors -= 1 ;

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (mxalias_curend) */


/* enumerate */
int mxalias_enum(MXALIAS *op,MXALIAS_CUR *curp,char *kbuf,int klen,
		char *vbuf,int vlen)
{
	KEYVALS_CUR	*kvcp ;
	int		rs = SR_OK ;
	int		kl = 0 ;
	const char	*kp, *vp ;


#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (curp->magic != MXALIAS_MAGIC) return SR_INVALID ;
	if (op->ncursors == 0) return SR_INVALID ;

	kvcp = curp->kvcp ;
	if (kvcp == NULL) {
	    const int	size = sizeof(KEYVALS_CUR) ;
	    if ((rs = uc_malloc(size,&kvcp)) >= 0) {
	        curp->kvcp = kvcp ;
	        rs = keyvals_curbegin(&op->entries,kvcp) ;
	        if (rs < 0) {
	            uc_free(kvcp) ;
	            curp->kvcp = NULL ;
	        }
	    }
	}

	if (rs >= 0) {
	    if ((rs = keyvals_enum(&op->entries,kvcp,&kp,&vp)) >= 0) {
	        kl = rs ;

#if	CF_DEBUGS
	        debugprintf("mxalias_enum: kl=%d\n",kl) ;
#endif

	        if (kbuf != NULL)
	            rs = sncpy1(kbuf,klen,kp) ;

	        if ((rs >= 0) && (vbuf != NULL))
	            rs = sncpy1(vbuf,vlen,vp) ;

	    } /* end if */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mxalias_enum: ret k=%s\n",kbuf) ;
	debugprintf("mxalias_enum: ret rs=%d kl=%d\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (mxalias_enum) */


/* lookup an entry by key-name */
int mxalias_lookup(MXALIAS *op,MXALIAS_CUR *curp,cchar *kbuf,int klen)
{
	NULSTR		kstr ;
	VECSTR		klist, vlist ;
	int		rs = SR_OK ;
	int		i ;
	int		opts ;
	int		c = 0 ;
	const char	*kp ;
	const char	*cp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (curp->magic != MXALIAS_MAGIC) return SR_INVALID ;
	if (op->ncursors == 0) return SR_INVALID ;
	if (kbuf[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mxalias_lookup: k=%t\n",kbuf,klen) ;
#endif

	if ((rs = nulstr_start(&kstr,kbuf,klen,&kp)) >= 0) {
	    opts = 0 ;
	    if ((rs = vecstr_start(&klist,4,opts)) >= 0) {
	        opts = VECSTR_OSTATIONARY ;
	        if ((rs = vecstr_start(&vlist,4,opts)) >= 0) {
	            if ((rs = mxalias_addvals(op,&klist,&vlist,kp)) >= 0) {
	                c += rs ;
	                for (i = 0 ; vecstr_get(&vlist,i,&cp) >= 0 ; i += 1) {
	                    if ((cp != NULL) && isnotspecial(*cp)) {
	                        rs = mxalias_addvals(op,&klist,&vlist,cp) ;
	                        c += rs ;
	                        if (rs > 0) {
	                            if (c > 0) c -= 1 ;
	                            vecstr_del(&vlist,i) ;
	                        }
	                    } /* end if */
	                    if (rs < 0) break ;
	                } /* end for */
	                if (rs >= 0) {
	                    rs = mxalias_mkvals(op,curp,&vlist) ;
	                    c = rs ;
	                }
	            } /* end if (addvals) */
	            vecstr_finish(&vlist) ;
	        } /* end if (vecstr) */
	        vecstr_finish(&klist) ;
	    } /* end if (vecstr) */
	    nulstr_finish(&kstr) ;
	} /* end if (nulstr) */

#if	CF_DEBUGS
	debugprintf("mxalias_lookup: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_lookup) */


int mxalias_read(MXALIAS *op,MXALIAS_CUR *curp,char *vbuf,int vlen)
{
	int		rs = SR_OK ;
	int		ni ;
	int		vl = 0 ;
	const char	*vp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if (curp->magic != MXALIAS_MAGIC) return SR_INVALID ;

	if (op->ncursors == 0) return SR_INVALID ;

	ni = (curp->i < 0) ? 0 : (curp->i + 1) ;

	if (ni < curp->nvals) {

	    vp = curp->vals[ni] ;
	    if (vbuf != NULL) {
	        rs = sncpy1(vbuf,vlen,vp) ;
	        vl = rs ;
	    } else
	        vl = strlen(vp) ;

	    if (rs >= 0)
	        curp->i = ni ;

	} else
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mxalias_read) */


/* private subroutines */


static int mxalias_username(MXALIAS *op,cchar *username)
{
	int		rs = SR_OK ;
	const char	*cp ;

	if ((username == NULL) || (username[0] == '\0'))
	    username = "-" ;

	if ((rs = uc_mallocstrw(username,-1,&cp)) >= 0) {
	    op->username = cp ;
	}

	return rs ;
}
/* end subroutine (mxalias_username) */


static int mxalias_userdname(MXALIAS *op)
{
	int		rs = SR_OK ;
	int		hl = 0 ;

	if (op->userdname == NULL) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN+1] ;
	    cchar	*un = op->username ;
	    if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	        const char	*cp ;
		hl = rs ;
	        if ((rs = uc_mallocstrw(hbuf,rs,&cp)) >= 0) {
	            op->userdname = cp ;
		}
	    } /* end if (getuserhome) */
	} else {
	    hl = strlen(op->userdname) ;
	} /* end if (null) */

	return (rs >= 0) ? hl : rs ;
}
/* end subroutine (mxalias_userdname) */


/* ARGSUSED */
static int mxalias_filesadd(MXALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	char		fname[MAXPATHLEN + 1] ;

#if	CF_FILESYS
	if (rs >= 0) {
	    if ((rs1 = mkpath1(fname,MXALIAS_SYSDB)) >= 0) {
	        rs = mxalias_fileadd(op,fname) ;
	        c += rs ;
	    }
	} /* end if */
#endif /* CF_FILESYS */

#if	CF_FILEUSER
	if (rs >= 0) {
	    if ((rs1 = mxalias_mkuserfname(op,fname)) >= 0) {
	        rs = mxalias_fileadd(op,fname) ;
	        c += rs ;
	    }
	} /* end if */
#endif /* CF_FILEUSER */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_filesadd) */


/* add a file to the list of files */
int mxalias_fileadd(MXALIAS *op,cchar *atfname)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*np ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (atfname == NULL) return SR_FAULT ;

	if (op->magic != MXALIAS_MAGIC) return SR_NOTOPEN ;

	if (atfname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mxalias_fileadd: fname=%s\n",atfname) ;
#endif

	np = (const char *) atfname ;
	if (atfname[0] != '/') {
	    if ((rs = mxalias_userdname(op)) >= 0) {
	        rs = mkpath2(tmpfname,op->userdname,atfname) ;
	        np = tmpfname ;
	    }
	} /* end if (added PWD) */

	if (rs >= 0) {
	    struct ustat	sb ;
	    if ((u_stat(np,&sb) >= 0) && isOurFileType(sb.st_mode)) {
	        if ((rs = mxalias_filereg(op,&sb,np)) >= 0) {
	            int	fi = rs ;
	            if (rs < INT_MAX) {
	                if ((rs = mxalias_fileparse(op,fi)) >= 0) {
	                    c = rs ;
	                }
	            } /* end if (needed registration) */
	            if (rs < 0)
	                mxalias_filedel(op,fi) ;
	        } /* end if (filereg) */
	    } /* end if (our file type) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("mxalias_fileadd: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_fileadd) */


static int mxalias_filereg(MXALIAS *op,USTAT *sbp,cchar *np)
{
	MXALIAS_FILE	fe ;
	int		rs ;
	int		fi = INT_MAX ;

	if ((rs = file_start(&fe,sbp,np)) >= 0) {
	    const int	nrs = SR_NOTFOUND ;
	    if ((rs = vecobj_search(&op->files,&fe,vcmpfe,NULL)) == nrs) {
	        rs = vecobj_add(&op->files,&fe) ;
	        fi = rs ;
	    }
	    if (rs < 0)
	        file_finish(&fe) ;
	} /* end if (file_start) */

	return (rs >= 0) ? fi : rs ;
}
/* end subroutine (mxalias_filereg) */


#if	CF_FILECHECK

/* check if files have changed */
static int mxalias_filechecks(MXALIAS *op,time_t daytime)
{
	struct ustat	sb ;
	MXALIAS_FILE	*fep ;
	int		rs = SR_OK ;
	int		i ;
	int		c_changed = 0 ;

/* check the files */

#if	CF_DEBUGS
	debugprintf("mxalias_filechecks: about to loop\n",i) ;
#endif

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    if ((u_stat(fep->fname,&sb) >= 0) &&
	        (sb.st_mtime > fep->mtime)) {

#if	CF_DEBUGS
	        debugprintf("mxalias_filechecks: file=%d changed\n",i) ;
	        debugprintf("mxalias_filechecks: freeing file entries\n") ;
#endif

	        c_changed += 1 ;
	        mxalias_filedump(op,i) ;

#if	CF_DEBUGS
	        debugprintf("mxalias_filechecks: parsing the file again\n") ;
#endif

	        rs = mxalias_fileparse(op,i) ;

#if	CF_DEBUGS
	        debugprintf("mxalias_filechecks: _fileparse() rs=%d\n",
	            rs) ;
#endif

	    } /* end if */

	    if (rs < 0) break ;
	} /* end for */

	op->ti_check = daytime ;

#if	CF_DEBUGS
	debugprintf("mxalias_filechecks: ret rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (mxalias_filechecks) */

#endif /* CF_FILECHECK */


#ifdef	COMMENT

static int mxalias_filealready(MXALIAS *op,dev_t dev,uino_t ino)
{
	MXALIAS_FILE	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("mxalias_filealready: searching for dev=%u ino=%u\n",
	    dev,ino) ;
#endif

	for (i = 0 ; (rs1 = vecobj_get(&op->files,i,&fep)) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("mxalias_filealready: got for dev=%u ino=%u\n",
	        fep->dev,fep->ino) ;
#endif

	    if ((fep->dev == dev) && (fep->ino == ino))
	        break ;

	} /* end for */

	if (rs1 >= 0)
	    rs = SR_EXIST ;

#if	CF_DEBUGS
	debugprintf("mxalias_filealready: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mxalias_filealready) */

#endif /* COMMENT */


static int mxalias_fileparse(MXALIAS *op,int fi)
{
	MXALIAS_FILE	*fep ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("mxalias_fileparse: ent fi=%u\n",fi) ;
#endif

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        bfile	mxfile, *lfp = &mxfile ;
	        if ((rs = bopen(lfp,fep->fname,"r",0664)) >= 0) {
	            struct ustat	sb ;
	            if ((rs = bcontrol(lfp,BC_STAT,&sb)) >= 0) {
	                if (! S_ISDIR(sb.st_mode)) {
	                    if (fep->mtime < sb.st_mtime) {

	                        fep->dev = sb.st_dev ;
	                        fep->ino = sb.st_ino ;
	                        fep->mtime = sb.st_mtime ;
	                        fep->size = sb.st_size ;

	                        rs = mxalias_fileparser(op,fi,lfp) ;
	                        c += rs ;

	                    } /* end if (needed update) */
	                } else {
	                    rs = SR_ISDIR ;
			}
	            } /* end if (bcontrol) */
	            rs1 = bclose(lfp) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (bfile) */
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} /* end if (vecobj_get) */

	if (rs < 0) mxalias_filedump(op,fi) ;

#if	CF_DEBUGS
	debugprintf("mxalias_fileparse: ret rs=%d c=%u\n",
	    rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_fileparse) */


static int mxalias_fileparser(MXALIAS *op,int fi,bfile *lfp)
{
	BUFDESC		bd ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("mxalias_fileparser: ent fi=%u\n",fi) ;
#endif

	if ((rs = bufdesc_start(&bd,llen)) >= 0) {
	    int		len ;
	    int		cl ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    const char	*cp ;
	    char	*lbuf = bd.lbuf ;

	    while ((rs = breadline(lfp,lbuf,llen)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;
	        if (f_eol) len -= 1 ;
	        lbuf[len] = '\0' ;

#if	CF_DEBUGS && CF_DEBUGFILE
	        debugprintf("mxalias_fileparse: line>%t<\n",
	            lbuf,strlinelen(lbuf,len,40)) ;
#endif

	        if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	            if (f_bol && (*cp != '#')) {
	                rs = mxalias_fileparseline(op,fi,&bd,cp,cl) ;
	                c += rs ;
	            }
	        }

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading extended lines) */

	    rs1 = bufdesc_finish(&bd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bufdesc) */

#if	CF_DEBUGS
	debugprintf("mxalias_fileparser: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_fileparser) */


static int mxalias_fileparseline(MXALIAS *op,int fi,BUFDESC *bdp,
		cchar *lp,int ll)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    int		ki ;
	    int		fl ;
	    const char	*fp ;
	    if ((fl = field_get(&fsb,kterms,&fp)) > 0) {

	        if ((ki = matstr(keywords,fp,fl)) >= 0) {
	            switch (ki) {
	            case keyword_alias:
	            case keyword_group:
	                rs = mxalias_fileparseline_alias(op,fi,bdp,&fsb) ;
	                c += rs ;
	                break ;
	            case keyword_unalias:
	            case keyword_ungroup:
	                rs = mxalias_fileparseline_unalias(op,fi,bdp,&fsb) ;
	                break ;
	            case keyword_source:
	                rs = mxalias_fileparseline_source(op,fi,bdp,&fsb) ;
	                c += rs ;
	                break ;
	            } /* end switch */
	        } /* end if */

	    } /* end if (field_get) */

	    field_finish(&fsb) ;
	} /* end if (field) */

#if	CF_DEBUGS && CF_DEBUGFILE
	debugprintf("mxalias_fileparseline: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_fileparseline) */


static int mxalias_fileparseline_alias(MXALIAS *op,int fi,BUFDESC *bdp,
		FIELD *fsbp)
{
	const int	flen = bdp->flen ;
	const int	klen = bdp->klen ;
	int		rs = SR_OK ;
	int		fl ;
	int		c_field = 0 ;
	int		c = 0 ;
	const char	*fp ;
	char		*keybuf = bdp->kbuf ;
	char		*fbuf = bdp->fbuf ;

	keybuf[0] = '\0' ;
	while (rs >= 0) {

	    fp = fbuf ;
	    fl = field_sharg(fsbp,vterms,fbuf,flen) ;

	    if ((fl < 0) || ((c_field == 0) && (fl == 0)))
	        break ;

#if	CF_DEBUGS && CF_DEBUGFILE
	    debugprintf("mxalias_fileparseline: cf=%u f=>%t<\n",
	        c_field,fp,fl) ;
#endif

	    if (c_field++ == 0) {
	        strwcpy(keybuf,fp,MIN(fl,klen)) ;
	    } else if (keybuf[0] != '\0') {
	        c += 1 ;
	        rs = keyvals_add(&op->entries,fi,keybuf,fp,fl) ;
	    } /* end if (handling record) */

	    if (fsbp->term == '#') break ;
	} /* end while (fields) */

	if ((rs >= 0) && (c == 0) && (keybuf[0] != '\0')) {
	    c += 1 ;
	    rs = keyvals_add(&op->entries,fi,keybuf,fbuf,0) ;
	}

#if	CF_DEBUGS && CF_DEBUGFILE
	debugprintf("mxalias_fileparseline: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_fileparseline_alias) */


/* ARGSUSED */
static int mxalias_fileparseline_unalias(MXALIAS *op,int fi,BUFDESC *bdp,	
		FIELD *fsbp)
{
	const int	flen = bdp->flen ;
	int		rs = SR_OK ;
	int		fl ;
	char		*fbuf = bdp->fbuf ;

	if ((fl = field_sharg(fsbp,vterms,fbuf,flen)) > 0) {
	    rs = keyvals_delkey(&op->entries,fbuf,fl) ;
	}

	return rs ;
}
/* end subroutine (mxalias_fileparseline_unalias) */


/* ARGSUSED */
static int mxalias_fileparseline_source(MXALIAS *op,int fi,BUFDESC *bdp,
		FIELD *fsbp)
{
	const int	flen = bdp->flen ;
	int		rs = SR_OK ;
	int		c = 0 ;
	char		*fbuf = bdp->fbuf ;

	if (field_sharg(fsbp,vterms,fbuf,flen) > 0) {
	    rs = mxalias_fileadd(op,fbuf) ;
	    c = rs ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_fileparseline_source) */


static int mxalias_filedump(MXALIAS *op,int fi)
{
	int		rs = SR_OK ;

	if (fi >= 0) {

	    rs = keyvals_delset(&op->entries,fi) ;

	} else {
	    const int	n = vecobj_count(&op->files) ;
	    int		i ;

	    for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	        rs = keyvals_delset(&op->entries,i) ;
	    }

	} /* end if */

	return rs ;
}
/* end subroutine (mxalias_filedump) */


static int mxalias_filedel(MXALIAS *op,int fi)
{
	MXALIAS_FILE	*fep ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        rs1 = file_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_del(&op->files,fi) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	}

	return rs ;
}
/* end subroutine (mxalias_filedel) */


/* free up all of the files in this MXALIAS list */
static int mxalias_filedels(MXALIAS *op)
{
	MXALIAS_FILE	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep != NULL) {
	        rs1 = file_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (mxalias_filedels) */


#if	CF_FILECHECK

static int mxalias_filechanged(MXALIAS *op,USTAT *sbp)
{

	if (op->fi.size != sbp->st_size)
	    return 1 ;

	if (op->fi.mtime != sbp->st_mtime)
	    return 1 ;

	if (op->fi.ino != sbp->st_ino)
	    return 1 ;

	if (op->fi.dev != sbp->st_dev)
	    return 1 ;

	return 0 ;
}
/* end subroutine (mxalias_filechanged) */


static int mxalias_fileold(MXALIAS *op,time_t daytime)
{
	int		rs, rs1 ;
	int		f = FALSE ;

	if ((rs = mxalias_aprofile(op,daytime)) >= 0) {
	    struct ustat	sb ;
	    int		i ;
	    const char	*cp ;
	    char	tmpfname[MAXPATHLEN + 1] ;

	    for (i = 0 ; op->aprofile[i] != NULL ; i += 1) {

	        cp = (const char *) op->aprofile[i] ;
	        if (*cp != '/') {
	            cp = tmpfname ;
	            mkpath2(tmpfname,op->pr,op->aprofile[i]) ;
	        }

	        rs1 = u_stat(cp,&sb) ;

	        if ((rs1 >= 0) && (sb.st_mtime > op->fi.mtime))
	            break ;

	    } /* end for */

	    f = (op->aprofile[i] != NULL) ? 1 : 0 ;
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mxalias_fileold) */

#endif /* CF_FILECHECK */


static int mxalias_mkuserfname(MXALIAS *op,char *fname)
{
	int		rs ;
	fname[0] = '\0' ;
	if ((rs = mxalias_userdname(op)) >= 0) {
	    cchar	*homedname = op->userdname ;
	    rs = mkpath2(fname,homedname,MXALIAS_USERDB) ;
	}
	return rs ;
}
/* end subroutine (mxalias_mkuserfname) */


static int mxalias_addvals(MXALIAS *op,vecstr *klp,vecstr *vlp,cchar *kp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (kp[0] != '\0') {
	    KEYVALS_CUR	kcur ;
	    if ((rs = keyvals_curbegin(&op->entries,&kcur)) >= 0) {
	        int	vl ;
	        int	f ;
	        cchar	*vp ;

	        while (rs >= 0) {

	            vl = keyvals_fetch(&op->entries,kp,&kcur,&vp) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;

	            c += 1 ;
	            if ((rs >= 0) && (vl > 0)) {

	                f = (kp[0] == vp[0]) ;
	                f = f && (strncmp(kp,vp,vl) == 0) ;
	                f = f && (kp[vl] == '\0') ;
	                if (! f) {
	                    rs1 = vecstr_findn(klp,vp,vl) ;
	                    f = (rs1 >= 0) ;
	                }
	                if (! f) {
	                    rs = vecstr_adduniq(vlp,vp,vl) ;
			}

	            } /* end if (substantive value) */

	        } /* end while */

	        keyvals_curend(&op->entries,&kcur) ;
	    } /* end if (cursor) */
	    if ((rs >= 0) && (c > 0)) {
	        rs = vecstr_adduniq(klp,kp,-1) ;
	    }
	} /* end if (non-nul) */

#if	CF_DEBUGS
	debugprintf("mxalias_addvals: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_addvals) */


static int mxalias_mkvals(MXALIAS *op,MXALIAS_CUR *curp,vecstr *vlp)
{
	int		rs = SR_OK ;
	int		n ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if ((n = vecstr_count(vlp)) > 0) {
	    void	*p ;
	    int		size = (n + 1) * sizeof(char **) ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        int		i ;
	        const char	*cp ;
	        curp->vals = p ;
	        size = 1 ;
	        for (i = 0 ; vecstr_get(vlp,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                size += (strlen(cp) + 1) ;
		    }
	        } /* end for */
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            char	*bp = p ;
	            curp->vbuf = p ;
	            *bp++ = '\0' ;
	            for (i = 0 ; vecstr_get(vlp,i,&cp) >= 0 ; i += 1) {
	                if (cp != NULL) {
	                    curp->vals[c++] = bp ;
	                    bp = strwcpy(bp,cp,-1) + 1 ;
			}
	            } /* end for */
	            curp->vals[c] = NULL ;
	            curp->nvals = c ;
	        } /* end if (memory-allocation) */
	        if (rs < 0) {
	            uc_free(curp->vals) ;
	            curp->vals = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (non-zero) */

#if	CF_DEBUGS
	debugprintf("mxalias_mkvals: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mxalias_mkvals) */


static int mxalias_allocfins(MXALIAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->pwd != NULL) {
	    rs1 = uc_free(op->pwd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pwd = NULL ;
	}

	if (op->userdname != NULL) {
	    rs1 = uc_free(op->userdname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->userdname = NULL ;
	}

	if (op->username != NULL) {
	    rs1 = uc_free(op->username) ;
	    if (rs >= 0) rs = rs1 ;
	    op->username = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	return rs ;
}
/* end subroutine (mxalias_allocfins) */


static int mxalias_entfins(MXALIAS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (mxalias_entfins) */


/* ARGSUSED */
static int file_start(MXALIAS_FILE *fep,USTAT *sbp,cchar *fname)
{
	int		rs ;
	const char	*cp ;

	if (fname == NULL) return SR_FAULT ;

	memset(fep,0,sizeof(MXALIAS_FILE)) ;

#ifdef	COMMENT
	fep->mtime = sbp->st_mtime ;
	fep->size = sbp->st_size ;
	fep->dev = sbp->st_dev ;
	fep->ino = sbp->st_ino ;
#endif /* COMMENT */

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    fep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(MXALIAS_FILE *fep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (fep == NULL) return SR_FAULT ;

	if (fep->fname != NULL) {
	    rs1 = uc_free(fep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (file_finish) */


static int bufdesc_start(BUFDESC *bdp,int llen)
{
	const int	klen = KEYBUFLEN ;
	int		size ;
	int		rs ;
	char		*bp ;
	if (llen < 0) llen = LINEBUFLEN ;
	size = ((2*(llen+1))+(klen+1)) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    bdp->a = bp ;
	    bdp->lbuf = bp ;
	    bdp->llen = llen ;
	    bp += (llen+1) ;
	    bdp->fbuf = bp ;
	    bdp->flen = llen ;
	    bp += (klen+1) ;
	    bdp->kbuf = bp ;
	    bdp->klen = klen ;
	}
	return rs ;
}
/* end subroutine (bufdesc_start) */


static int bufdesc_finish(BUFDESC *bdp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (bdp->a != NULL) {
	    rs1 = uc_free(bdp->a) ;
	    if (rs >= 0) rs = rs1 ;
	    bdp->a = NULL ;
	}
	return rs ;
}
/* end subroutine (bufdesc_finish) */


static int vcmpfe(MXALIAS_FILE **e1pp,MXALIAS_FILE **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            MXALIAS_FILE	*e1p = *e1pp ;
	            MXALIAS_FILE	*e2p = *e2pp ;
	            if ((rc = (e1p->dev - e2p->dev)) == 0) {
	                rc = (e1p->ino - e2p->ino) ;
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = +1 ;
	}
	return rc ;
}
/* end subroutine (vcmpfe) */


static int isnotspecial(int ch)
{
	int		f ;
	ch &= 255 ;
	f = (ch != '/') && (ch != '|') ;
	return f ;
}
/* end subroutine (isnotspecial) */


static int isOurFileType(mode_t m)
{
	return S_ISREG(m) || S_ISSOCK(m) || S_ISFIFO(m) || S_ISCHR(m) ;
}
/* end subroutine (isOurFileType) */


