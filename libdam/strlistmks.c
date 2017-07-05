/* strlistmks */

/* make a STRLIST database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FIRSTHASH	0		/* arrange for first-attempt hashing */
#define	CF_MINMOD	1		/* ensure minimum file mode */
#define	CF_LATE		0		/* late open */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a STRLIST database file.

	Synopsis:

	int strlistmks_open(op,dbname,oflags,om,n)
	STRLISTMKS	*op ;
	const char	dbname[] ;
	int		oflags ;
	mode_t		om ;
	int		n ;

	Arguments:

	op		object pointer
	dbname		name of (path-to) DB
	oflags		open-flags
	om		open-mode
	n		starting estimate of numbers of variables

	Returns:

	>=0		OK
	<0		error code


	Notes:

	= possible returns to an open attempt

	- OK (creating)
	- already exists
	- doesn't exist but is in progress
	- exists and is in progress

	= open-flags

			if DB exits	if NDB exists	returns
	___________________________________________________________________

	-		no		no		SR_OK (created)
	-		no		yes		SR_INPROGRESS
	-		yes		no		SR_OK
	-		yes		yes		SR_INPROGRESS

	O_CREAT|O_EXCL	no		no		SR_OK (created)
	O_CREAT|O_EXCL	no		yes		SR_INPROGRESS
	O_CREAT|O_EXCL	yes		no		SR_EXIST
	O_CREAT|O_EXCL	yes		yes		SR_INPROGRESS

	O_CREAT		x		x		SR_OK (created)


*******************************************************************************/


#define	STRLISTMKS_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<strtab.h>
#include	<filebuf.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"strlistmks.h"
#include	"strlisthdr.h"


/* local defines */

#define	STRLISTMKS_SIZEMULT	4
#define	STRLISTMKS_NSKIP	5
#define	STRLISTMKS_INDPERMS	0664

#undef	RECTAB
#define	RECTAB		struct strlistmks_rectab

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	120
#endif

#define	BUFLEN		(sizeof(STRLISTHDR) + 128)

#define	FSUF_IND	"si"

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	DEBFNAME	"strlistmks.deb"


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;
extern uint	nextpowtwo(uint) ;

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,const char *,
			const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy6(char *,int,const char *,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf3(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	hasuc(const char *,int) ;
extern int	vstrkeycmp(const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* exported variables */

STRLISTMKS_OBJ	strlistmks = {
	"strlistmks",
	sizeof(STRLISTMKS)
} ;


/* local structures */

struct varentry {
	uint	khash ;
	uint	ri ;
	uint	ki ;
	uint	hi ;
} ;


/* forward references */

static int	strlistmks_filesbegin(STRLISTMKS *) ;
static int	strlistmks_filesend(STRLISTMKS *,int) ;

static int	strlistmks_listbegin(STRLISTMKS *,int) ;
static int	strlistmks_listend(STRLISTMKS *) ;

static int	strlistmks_nfcreate(STRLISTMKS *,const char *) ;
static int	strlistmks_nfcreatecheck(STRLISTMKS *,cchar *,cchar *) ;
static int	strlistmks_nfdestroy(STRLISTMKS *) ;
static int	strlistmks_nfstore(STRLISTMKS *,const char *) ;
static int	strlistmks_fexists(STRLISTMKS *) ;

static int	strlistmks_mkvarfile(STRLISTMKS *) ;
static int	strlistmks_wrvarfile(STRLISTMKS *) ;
static int	strlistmks_mkind(STRLISTMKS *,const char *,uint (*)[3],int) ;
static int	strlistmks_renamefiles(STRLISTMKS *) ;

static int	rectab_start(RECTAB *,int) ;
static int	rectab_add(RECTAB *,uint) ;
static int	rectab_done(RECTAB *) ;
static int	rectab_getvec(RECTAB *,uint (**)[1]) ;
static int	rectab_extend(RECTAB *) ;
static int	rectab_finish(RECTAB *) ;

#ifdef	COMMENT
static int	rectab_count(RECTAB *) ;
#endif

static int	filebuf_writefill(FILEBUF *,const char *,int) ;

static int	indinsert(uint (*rt)[1],uint (*it)[3],int,struct varentry *) ;
static int	hashindex(uint,int) ;


/* local variables */

static const char	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int strlistmks_open(op,dbname,oflags,om,n)
STRLISTMKS	*op ;
const char	dbname[] ;
int		oflags ;
mode_t		om ;
int		n ;
{
	int		rs ;
	const char	*cp ;

#if	CF_DEBUGS && defined(DEBFNAME)
	{
	    int	dfd = debuggetfd() ;
	    nprintf(DEBFNAME,"strlistmks_open: ent dfd=%d\n",dfd) ;
	}
#endif /* DEBFNAME */

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("strlistmks_open: ent dbname=%s\n",dbname) ;
#endif /* CF_DEBUGS */

	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

	if (n < STRLISTMKS_NENTRIES)
	    n = STRLISTMKS_NENTRIES ;

	memset(op,0,sizeof(STRLISTMKS)) ;
	op->om = om ;
	op->nfd = -1 ;
	op->gid = -1 ;

	op->f.creat = (oflags & O_CREAT) ;
	op->f.excl = (oflags & O_EXCL) ;
	op->f.none = (! op->f.creat) && (! op->f.excl) ;

	    if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
		op->dbname = cp ;
		if ((rs = strlistmks_filesbegin(op)) >= 0) {
		    if ((rs = strlistmks_listbegin(op,n)) >= 0) {
			op->magic = STRLISTMKS_MAGIC ;
		    }
		    if (rs < 0)
			strlistmks_filesend(op,FALSE) ;
		} /* end if */
		if (rs < 0) {
	    	    uc_free(op->dbname) ;
	    	    op->dbname = NULL ;
		}
	    } /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("strlistmks_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlistmks_open) */


int strlistmks_close(op)
STRLISTMKS	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_remove = TRUE ;
	int		nvars = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRLISTMKS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("strlistmks_close: nvars=%u\n",op->nstrs) ;
#endif

	nvars = op->nstrs ;
	if (! op->f.abort) {
	    rs1 = strlistmks_mkvarfile(op) ;
	    f_remove = (rs1 < 0) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("strlistmks_close: strlistmks_mkvarfile() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	rs1 = strlistmks_listend(op) ;
	if (! f_remove) f_remove = (rs1 < 0) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs >= 0) && (! op->f.abort)) {
	    rs1 = strlistmks_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("strlistmks_close: strlistmks_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = strlistmks_filesend(op,f_remove) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("strlistmks_close: ret=%d\n",rs) ;
#endif /* CF_DEBUGS */

	op->magic = 0 ;
	return (rs >= 0) ? nvars : rs ;
}
/* end subroutine (strlistmks_close) */


int strlistmks_addvar(op,sp,sl)
STRLISTMKS	*op ;
const char	sp[] ;
int		sl ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != STRLISTMKS_MAGIC) return SR_NOTOPEN ;

	if ((rs = strtab_add(&op->strs,sp,sl)) >= 0) {
	    uint	ki = rs ;
	        if ((rs = rectab_add(&op->rectab,ki)) >= 0) {
	            op->nstrs += 1 ;
	        }
	}

#if	CF_DEBUGS && 0
	debugprintf("strlistmks_add: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlistmks_addvar) */


int strlistmks_abort(op)
STRLISTMKS	*op ;
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRLISTMKS_MAGIC) return SR_NOTOPEN ;

	op->f.abort = TRUE ;
	return SR_OK ;
}
/* end subroutine (strlistmks_abort) */


int strlistmks_chgrp(op,gid)
STRLISTMKS	*op ;
gid_t		gid ;
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRLISTMKS_MAGIC) return SR_NOTOPEN ;

	op->gid = gid ;
	return SR_OK ;
}
/* end subroutine (strlistmks_chgrp) */


/* private subroutines */


static int strlistmks_filesbegin(op)
STRLISTMKS	*op ;
{
	int		rs = SR_INVALID ;
	int		dnl ;
	const char	*dnp ;
	char		tmpdname[MAXPATHLEN + 1] ;

/* check that the parent directory is writable to us */

	if ((dnl = sfdirname(op->dbname,-1,&dnp)) > 0) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(dnp,dnl,&cp)) >= 0) {
	        op->idname = cp ;
	        if (dnl == 0) {
	            rs = getpwd(tmpdname,MAXPATHLEN) ;
	            dnl = rs ;
	        } else
	            rs = mkpath1w(tmpdname,dnp,dnl) ;
	        if (rs >= 0) {
	            int	operm = (X_OK | W_OK) ;
	            rs = perm(tmpdname,-1,-1,NULL,operm) ;
	        }
	        if (rs >= 0) {
	            if ((rs = strlistmks_nfcreate(op,FSUF_IND)) >= 0) {
	                if (op->f.creat && op->f.excl) {
	                    rs = strlistmks_fexists(op) ;
	                }
	                if (rs < 0)
		            strlistmks_nfdestroy(op) ;
	            } /* end if (nfcreate) */
	        }
	        if (rs < 0) {
		    if (op->idname != NULL) {
	    	        uc_free(op->idname) ;
	    	        op->idname = NULL ;
		    }
	        }
	    } /* end if (memory-allocation) */
	} /* end if (sfshrink) */

#if	CF_DEBUGS
	debugprintf("strlistmks_filesbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strlistmks_filesbegin) */


static int strlistmks_filesend(op,f)
STRLISTMKS	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->nfname != NULL) {
	    if (f && (op->nfname[0] != '\0')) {
	        u_unlink(op->nfname) ;
	    }
	    rs1 = uc_free(op->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfname = NULL ;
	}

	if (op->idname != NULL) {
	    rs1 = uc_free(op->idname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idname = NULL ;
	}

	return rs ;
}
/* end subroutine (strlistmks_filesend) */


/* exclusively create this new file */
static int strlistmks_nfcreate(op,fsuf)
STRLISTMKS	*op ;
const char	fsuf[] ;
{
	struct ustat	sb ;
	const int	to_old = TO_OLDFILE ;
	int		rs ;
	int		rs1 ;
	int		nfl ;
	int		oflags = (O_CREAT | O_EXCL | O_WRONLY) ;
	const char	*cp ;
	char		nfname[MAXPATHLEN + 1] ;

	rs = mkfnamesuf3(nfname,op->dbname,fsuf,ENDIANSTR,"n") ;
	nfl = rs ;
	if (rs >= 0) {
	    rs = uc_mallocstrw(nfname,nfl,&cp) ;
	    if (rs >= 0) op->nfname = (char *) cp ;
	}

	if (rs < 0) goto ret0 ;

again:
	rs = u_open(op->nfname,oflags,op->om) ;
	op->nfd = rs ;

#if	CF_LATE
	if (rs >= 0) {
	    u_close(op->nfd) ;
	    op->nfd = -1 ;
	}
#endif /* CF_LATE */

	if (rs == SR_EXIST) {
	    time_t	daytime = time(NULL) ;
	    int		f_inprogress ;
	    rs1 = u_stat(op->nfname,&sb) ;
	    if ((rs1 >= 0) && ((daytime - sb.st_mtime) > to_old)) {
		u_unlink(op->nfname) ;
		goto again ;
	    }
	    op->f.inprogress = TRUE ;
	    f_inprogress = op->f.none ;
	    f_inprogress = f_inprogress || (op->f.creat && op->f.excl) ;
	    rs = (f_inprogress) ? SR_INPROGRESS : SR_OK ;
	} /* end if */

	if (rs >= 0) {
	    op->f.created = TRUE ;
	} else {
	    if (op->nfname != NULL) {
	        uc_free(op->nfname) ;
	        op->nfname = NULL ;
	    }
	}

ret0:
	return rs ;
}
/* end subroutine (txindexmks_nfcreate) */


static int strlistmks_nfcreatecheck(op,fpre,fsuf)
STRLISTMKS	*op ;
const char	fpre[] ;
const char	fsuf[] ;
{
	int		rs = SR_OK ;
	int		oflags ;

#if	CF_DEBUGS
	debugprintf("strlistmks_nfcreatecheck: nfd=%d\n",op->nfd) ;
	debugprintf("strlistmks_nfcreatecheck: f_inprogress=%u\n",
		op->f.inprogress) ;
#endif

	if ((op->nfd < 0) || op->f.inprogress) {
	    if (op->nfd >= 0) {
		u_close(op->nfd) ;
		op->nfd = -1 ;
	    }
	    oflags = O_WRONLY | O_CREAT ;
	    if (op->f.inprogress) {
		char	cname[MAXNAMELEN + 1] ;
		char	infname[MAXPATHLEN + 1] ;
		char	outfname[MAXPATHLEN + 1] ;
		outfname[0] = '\0' ;
		rs = sncpy6(cname,MAXNAMELEN,
			fpre,"XXXXXXXX",".",fsuf,ENDIANSTR,"n") ;
		if (rs >= 0) {
		    if ((op->idname != NULL) && (op->idname[0] != '\0')) {
		        rs = mkpath2(infname,op->idname,cname) ;
		    } else
		        rs = mkpath1(infname,cname) ;
		}
		if (rs >= 0) {
		    rs = opentmpfile(infname,oflags,op->om,outfname) ;
	            op->nfd = rs ;
		    op->f.created = (rs >= 0) ;
		}
		if (rs >= 0)
		    rs = strlistmks_nfstore(op,outfname) ;
		if (rs < 0) {
		    if (outfname[0] != '\0')
			u_unlink(outfname) ;
		}
	    } else {
	        rs = u_open(op->nfname,oflags,op->om) ;
	        op->nfd = rs ;
		op->f.created = (rs >= 0) ;
	    }
	    if (rs < 0) {
		if (op->nfd >= 0) {
		    u_close(op->nfd) ;
		    op->nfd = -1 ;
		}
	        if (op->nfname != NULL) {
		    if (op->nfname[0] != '\0') {
			u_unlink(op->nfname) ;
		    }
	            uc_free(op->nfname) ;
	            op->nfname = NULL ;
		}
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (strlistmks_nfcreatecheck) */


static int strlistmks_nfdestroy(op)
STRLISTMKS	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	if (op->nfname != NULL) {
	    if (op->nfname[0] != '\0') {
		rs1 = u_unlink(op->nfname) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(op->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfname = NULL ;
	}

	return rs ;
}
/* end subroutine (strlistmks_nfdestroy) */


static int strlistmks_nfstore(op,outfname)
STRLISTMKS	*op ;
const char	outfname[] ;
{
	int		rs ;
	const char	*cp ;

	if (op->nfname != NULL) {
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	rs = uc_mallocstrw(outfname,-1,&cp) ;
	if (rs >= 0) op->nfname = (char *) cp ;

	return rs ;
}
/* end subroutine (strlistmks_nfstore) */


static int strlistmks_fexists(op)
STRLISTMKS	*op ;
{
	int		rs = SR_OK ;

	if (op->f.creat && op->f.excl && op->f.inprogress) {
	    const char	*suf = FSUF_IND ;
	    const char	*end = ENDIANSTR ;
	    char	hfname[MAXPATHLEN + 1] ;
	    if ((rs = mkfnamesuf2(hfname,op->dbname,suf,end)) >= 0) {
		struct ustat	sb ;
	        int	rs1 = u_stat(hfname,&sb) ;
	        if (rs1 >= 0) rs = SR_EXIST ;
	    }
	}

	return rs ;
}
/* end subroutine (strlistmks_fexists) */


static int strlistmks_listbegin(op,n)
STRLISTMKS	*op ;
int		n ;
{
	int		rs ;
	int		size ;

	size = (n * STRLISTMKS_SIZEMULT) ;
	if ((rs = strtab_start(&op->strs,size)) >= 0) {
	        rs = rectab_start(&op->rectab,n) ;
	    if (rs < 0)
		strtab_finish(&op->strs) ;
	} /* end if (strtab-keys) */

	return rs ;
}
/* end subroutine (strlistmks_listbegin) */


static int strlistmks_listend(op)
STRLISTMKS	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = rectab_finish(&op->rectab) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = strtab_finish(&op->strs) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (strlistmks_listend) */


static int strlistmks_mkvarfile(op)
STRLISTMKS	*op ;
{
	int		rs = SR_OK ;
	int		rtl ;

	rtl = rectab_done(&op->rectab) ;

	if (rtl == (op->nstrs + 1)) {
	    rs = strlistmks_wrvarfile(op) ;
	} else
	    rs = SR_BUGCHECK ;

	return (rs >= 0) ? op->nstrs : rs ;
}
/* end subroutine (strlistmks_mkvarfile) */


static int strlistmks_wrvarfile(op)
STRLISTMKS	*op ;
{
	STRLISTHDR	hf ;
	FILEBUF		varfile ;
	STRTAB		*ksp = &op->strs ;
	const time_t	daytime = time(NULL) ;
	uint		fileoff = 0 ;
	uint		(*rt)[1] ;
	const int	pagesize = getpagesize() ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		rtl ;
	int		itl ;
	int		size ;
	int		bl ;
	char		buf[BUFLEN + 1] ;

	rtl = rectab_getvec(&op->rectab,&rt) ;

/* open (create) the STRLIST file */

	rs = strlistmks_nfcreatecheck(op,"nv",FSUF_IND) ;
	if (rs < 0)
	    goto ret0 ;

	op->f.viopen = TRUE ;
	size = (pagesize * 4) ;
	rs = filebuf_start(&varfile,op->nfd,0,size,0) ;
	if (rs < 0)
	    goto ret1 ;

/* prepare the file-header */

	memset(&hf,0,sizeof(STRLISTHDR)) ;
	hf.vetu[0] = STRLISTHDR_VERSION ;
	hf.vetu[1] = ENDIAN ;
	hf.vetu[2] = 0 ;
	hf.vetu[3] = 0 ;
	hf.wtime = (uint) daytime ;
	hf.nstrs = op->nstrs ;
	hf.nskip = STRLISTMKS_NSKIP ;

/* create the file-header */

	rs = strlisthdr(&hf,0,buf,BUFLEN) ;
	bl = rs ;
	if (rs < 0)
	    goto ret2 ;

/* write header */

	if (rs >= 0) {
	    rs = filebuf_writefill(&varfile,buf,bl) ;
	    fileoff += rs ;
	}

	if (rs < 0)
	    goto ret2 ;

/* write the record table */

	hf.rtoff = fileoff ;
	hf.rtlen = rtl ;

	size = (rtl + 1) * 2 * sizeof(uint) ;
	rs = filebuf_write(&varfile,rt,size) ;
	fileoff += rs ;

/* make and write out key-string table */

	if (rs >= 0) {
	    char	*kstab = NULL ;

	    size = strtab_strsize(ksp) ;

	    hf.stoff = fileoff ;
	    hf.stlen = size ;

	    if ((rs = uc_malloc(size,&kstab)) >= 0) {

	        rs = strtab_strmk(ksp,kstab,size) ;

/* write out the key-string table */

	        if (rs >= 0) {
	            rs = filebuf_write(&varfile,kstab,size) ;
	            fileoff += rs ;
	        }

/* make and write out the record-index table */

	        if (rs >= 0) {
		    uint	(*indtab)[3] = NULL ;

	            itl = nextpowtwo(rtl) ;

	            hf.itoff = fileoff ;
	            hf.itlen = itl ;

	            size = (itl + 1) * 3 * sizeof(int) ;

	            if ((rs = uc_malloc(size,&indtab)) >= 0) {

			memset(indtab,0,size) ;

#if	CF_DEBUGS
	debugprintf("strlistmks_wrvarfile: _mkind() \n") ;
#endif

	                rs = strlistmks_mkind(op,kstab,indtab,itl) ;

	                if (rs >= 0) {
	                    rs = filebuf_write(&varfile,indtab,size) ;
	                    fileoff += rs ;
	                }

	                uc_free(indtab) ;
	            } /* end if (memory allocation) */

	        } /* end if (record-index table) */

	        uc_free(kstab) ;
	    } /* end if (memory allocation) */

	} /* end if (key-string table) */

/* write out the header -- again! */
ret2:
	filebuf_finish(&varfile) ;

	if (rs >= 0) {

	    hf.fsize = fileoff ;

	    rs = strlisthdr(&hf,0,buf,BUFLEN) ;
	    bl = rs ;
	    if (rs >= 0)
	        rs = u_pwrite(op->nfd,buf,bl,0L) ;

#if	CF_MINMOD
	if (rs >= 0)
	    rs = uc_fminmod(op->nfd,op->om) ;
#endif /* CF_MINMOD */

	    if ((rs >= 0) && (op->gid >= 0)) {
#if	CF_DEBUGS
		debugprintf("strlistmks_wrvarfile: gid=%d\n",op->gid) ;
#endif
		rs = u_fchown(op->nfd,-1,op->gid) ;
	    }

	} /* end if (succeeded?) */

/* we're out of here */
ret1:
	op->f.viopen = FALSE ;
	rs1 = u_close(op->nfd) ;
	if (rs >= 0) rs = rs1 ;
	op->nfd = -1 ;

	if ((rs < 0) && (op->nfname[0] != '\0')) {
	    u_unlink(op->nfname) ;
	    op->nfname[0] = '\0' ;
	}

ret0:
	return rs ;
}
/* end subroutine (strlistmks_wrvarfile) */


/* make an index table of the record table */
int strlistmks_mkind(op,kst,it,il)
STRLISTMKS	*op ;
const char	kst[] ;
uint		(*it)[3] ;
int		il ;
{
	struct varentry	ve ;
	uint		ri, ki, hi ;
	uint		khash ;
	uint		(*rt)[1] ;
	int		rs = SR_OK ;
	int		rtl ;
	int		sc = 0 ;
	const char	*kp ;

	rtl = rectab_getvec(&op->rectab,&rt) ;

#if	CF_DEBUGS
	debugprintf("strlistmks_mkind: rtl=%u\n",rtl) ;
#endif

#if	CF_FIRSTHASH
	{
	    struct varentry	*vep ;

	    VECOBJ	ves ;

	    int	size, opts, i ;


	    size = sizeof(struct varentry) ;
	    opts = VECOBJ_OCOMPACT ;
	    if ((rs = vecobj_start(&ves,size,rtl,opts)) >= 0) {

	    for (ri = 1 ; ri < rtl ; ri += 1) {

	        ki = rt[ri][0] ;
	        kp = kst + ki ;
	        khash = hashelf(kp,-1) ;

	        hi = hashindex(khash,il) ;

	        if (it[hi][0] == 0) {
	            it[hi][0] = ri ;
	            it[hi][1] = (khash & INT_MAX) ;
	            it[hi][2] = 0 ;
	            sc += 1 ;
	        } else {
		    ve.ri = ri ;
		    ve.ki = ki ;
	            ve.khash = chash ;
	            ve.hi = hi ;
	            rs = vecobj_add(&ves,&ve) ;
	        }

	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	    for (i = 0 ; vecobj_get(&ves,i,&vep) >= 0 ; i += 1) {
	        sc += indinsert(rt,it,il,vep) ;
	    } /* end for */
	    }

	    vecobj_finish(&ves) ;
	    } /* end if (ves) */

	}
#else /* CF_FIRSTHASH */
	{
	for (ri = 1 ; ri < rtl ; ri += 1) {

	    ki = rt[ri][0] ;
	    kp = kst + ki ;

#if	CF_DEBUGS
	debugprintf("strlistmks_mkind: ri=%u k=%s\n",ri,
		kp,strnlen(kp,20)) ;
#endif

	    khash = hashelf(kp,-1) ;

	    hi = hashindex(khash,il) ;

	    ve.ri = ri ;
	    ve.ki = ki ;
	    ve.khash = khash ;
	    ve.hi = hi ;
	    sc += indinsert(rt,it,il,&ve) ;

	} /* end for */
	}
#endif /* CF_FIRSTHASH */

	it[il][0] = UINT_MAX ;
	it[il][1] = 0 ;
	it[il][2] = 0 ;

	if (sc < 0)
	    sc = 0 ;

#if	CF_DEBUGS
	debugprintf("strlistmks_mkind: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? sc : rs ;
}
/* end subroutine (strlistmks_mkind) */


static int strlistmks_renamefiles(op)
STRLISTMKS	*op ;
{
	int		rs ;
	const char	*end = ENDIANSTR ;
	char		hashfname[MAXPATHLEN + 1] ;

	if ((rs = mkfnamesuf2(hashfname,op->dbname,FSUF_IND,end)) >= 0) {
	    if ((rs = u_rename(op->nfname,hashfname)) >= 0)
	        op->nfname[0] = '\0' ;
	    if (op->nfname[0] != '\0') {
	        u_unlink(op->nfname) ;
	        op->nfname[0] = '\0' ;
	    }
	}

	return rs ;
}
/* end subroutine (strlistmks_renamefiles) */


static int rectab_start(rtp,n)
RECTAB		*rtp ;
int		n ;
{
	int		rs = SR_OK ;
	int		size ;
	void		*p ;

	if (n < 10)
	    n = 10 ;

	rtp->i = 0 ;
	rtp->n = n ;
	size = (n + 1) * 1 * sizeof(int) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    rtp->rectab = p ;
	    rtp->rectab[0][0] = 0 ;
	    rtp->i = 1 ;
	}

	return rs ;
}
/* end subroutine (rectab_start) */


static int rectab_finish(rtp)
RECTAB		*rtp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (rtp->rectab != NULL) {
	    rs1 = uc_free(rtp->rectab) ;
	    if (rs >= 0) rs = rs1 ;
	    rtp->rectab = NULL ;
	}

	return rs ;
}
/* end subroutine (rectab_finish) */


static int rectab_add(rtp,ki)
RECTAB		*rtp ;
uint		ki ;
{
	int		rs = SR_OK ;
	int		i ;

	i = rtp->i ;
	if ((i + 1) > rtp->n)
	    rs = rectab_extend(rtp) ;

	if (rs >= 0) {
	    rtp->rectab[i][0] = ki ;
	    rtp->i += 1 ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (rectab_add) */


static int rectab_extend(rtp)
RECTAB		*rtp ;
{
	int		rs = SR_OK ;

	if ((rtp->i + 1) > rtp->n) {
	    uint	(*va)[1] ;
	    int		nn ;
	    int		size ;
	    nn = (rtp->n + 1) * 1 ;
	    size = (nn + 1) * 1 * sizeof(int) ;
	    if ((rs = uc_realloc(rtp->rectab,size,&va)) >= 0) {
	        rtp->rectab = va ;
	        rtp->n = nn ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (rectab_extend) */


static int rectab_done(rtp)
RECTAB		*rtp ;
{
	int		i = rtp->i ;

	rtp->rectab[i][0] = UINT_MAX ;
	return i ;
}
/* end subroutine (rectab_done) */


#ifdef	COMMENT

static int rectab_count(rtp)
RECTAB		*rtp ;
{

	return rtp->i ;
}
/* end subroutine (rectab_count) */

#endif /* COMMENT */


static int rectab_getvec(rtp,rpp)
RECTAB		*rtp ;
uint		(**rpp)[1] ;
{

	*rpp = rtp->rectab ;
	return rtp->i ;
}
/* end subroutine (rectab_getvec) */


static int filebuf_writefill(bp,wbuf,wlen)
FILEBUF		*bp ;
const char	wbuf[] ;
int		wlen ;
{
	int		rs ;
	int		r, nzero ;
	int		len ;

	if (wlen < 0)
	    wlen = (strlen(wbuf) + 1) ;

	rs = filebuf_write(bp,wbuf,wlen) ;
	len = rs ;

	r = (wlen & 3) ;
	if ((rs >= 0) && (r > 0)) {
	    nzero = (4 - r) ;
	    if (nzero > 0) {
	        rs = filebuf_write(bp,zerobuf,nzero) ;
	        len += rs ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (filebuf_writefill) */


static int indinsert(rt,it,il,vep)
uint		(*rt)[1] ;
uint		(*it)[3] ;
int		il ;
struct varentry	*vep ;
{
	uint		nhash, chash ;
	uint		ri, ki ;
	uint		lhi, nhi, hi ;
	int		c = 0 ;

	hi = vep->hi ;
	nhash = vep->khash ;
	chash = (nhash & INT_MAX) ;

#if	CF_DEBUGS
	debugprintf("indinsert: ve ri=%u ki=%u khash=%08X hi=%u\n",
		vep->ri,vep->ki,vep->khash,vep->hi) ;
	debugprintf("indinsert: il=%u loop 1\n",il) ;
#endif

/* CONSTCOND */
	while (TRUE) {

#if	CF_DEBUGS
	debugprintf("indinsert: it%u ri=%u nhi=%u\n",
		hi,it[hi][0],it[hi][2]) ;
#endif

	    if (it[hi][0] == 0)
		break ;

	    ri = it[hi][0] ;
	    ki = rt[ri][0] ;
	    if (ki == vep->ki)
		break ;

	    it[hi][1] |= (~ INT_MAX) ;
	    nhash = hashagain(nhash,c++,STRLISTMKS_NSKIP) ;

	    hi = hashindex(nhash,il) ;

#if	CF_DEBUGS
	debugprintf("indinsert: nhash=%08X nhi=%u\n",nhash,hi) ;
#endif

	} /* end while */

	if (it[hi][0] > 0) {

#if	CF_DEBUGS
	debugprintf("indinsert: loop 2\n") ;
#endif

	    lhi = hi ;
	    while ((nhi = it[lhi][2]) > 0)
	        lhi = nhi ;

	    hi = hashindex((lhi + 1),il) ;

#if	CF_DEBUGS
	debugprintf("indinsert: loop 3 lhi=%u\n",lhi) ;
#endif

	    while (it[hi][0] > 0)
	        hi = hashindex((hi + 1),il) ;

	    it[lhi][2] = hi ;

#if	CF_DEBUGS
	debugprintf("indinsert: loop 3 it%u ki=%u nhi=%u\n",lhi,
		it[lhi][0],hi) ;
#endif

	} /* end if (same-key continuation) */

	it[hi][0] = vep->ri ;
	it[hi][1] = chash ;
	it[hi][2] = 0 ;

#if	CF_DEBUGS
	debugprintf("indinsert: ret hi=%u c=%u\n",hi,c) ;
#endif

	return c ;
}
/* end subroutine (indinsert) */


static int hashindex(uint i,int n)
{
	int		hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */


