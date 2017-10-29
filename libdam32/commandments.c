/* commandments */

/* COMMANDMENTS object implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGLINE	0		/* special debug-line subroutine */
#define	CF_EMPTYTERM	0		/* terminate entry on empty line */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the COMMANDMENTS datbase.


*******************************************************************************/


#define	COMMANDMENTS_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<nulstr.h>
#include	<sbuf.h>
#include	<char.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"commandments.h"
#include	"cmimk.h"
#include	"cmi.h"


/* local defines */

#define	COMMANDMENTS_SN		"commandments"
#define	COMMANDMENTS_DBDNAME	"share/commandments"
#define	COMMANDMENTS_TMPDNAME	"/var/tmp"
#define	COMMANDMENTS_NLINES	40

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	MKENT		struct mkent
#define	MKENT_LINE	struct mkent_line

#define	TO_CHECK	4


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfprogroot(cchar *,int,cchar **) ;
extern int	sfrootname(cchar *,int,cchar **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	mkdirs(char *,mode_t) ;
extern int	removes(cchar *) ;
extern int	prmktmpdir(cchar *,char *,cchar *,cchar *,mode_t) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	isdigitlatin(int) ;
extern int	isOneOf(const int *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* local structures */

struct mkent_line {
	uint		loff ;
	uint		llen ;
} ;

struct mkent {
	MKENT_LINE	*lines ;
	uint		eoff ;
	uint		elen ;
	int		e, i ;
	ushort		cn ;
} ;

static const int	rsold[] = {
	SR_STALE,
	0
} ;


/* forward references */

static int	commandments_argsbegin(COMMANDMENTS *,cchar *,cchar *) ;
static int	commandments_argsend(COMMANDMENTS *) ;

static int	commandments_findbegin(COMMANDMENTS *,cchar *,cchar *) ;
static int	commandments_findend(COMMANDMENTS *) ;
static int	commandments_tmpcheck(COMMANDMENTS *,char *,USTAT *,cchar *) ;
static int	commandments_tmpcopy(COMMANDMENTS *,char *,cchar *,char *) ;

static int	commandments_fileloadbegin(COMMANDMENTS *,cchar *) ;
static int	commandments_fileloadend(COMMANDMENTS *) ;
static int	commandments_dbmapbegin(COMMANDMENTS *,time_t) ;
static int	commandments_dbmapend(COMMANDMENTS *) ;
static int	commandments_dbproc(COMMANDMENTS *,CMIMK *) ;
static int	commandments_checkupdate(COMMANDMENTS *,time_t) ;
static int	commandments_loadbuf(COMMANDMENTS *,CMI_ENT *,char *,int rlen) ;

static int	commandments_userhome(COMMANDMENTS *) ;
static int	commandments_usridname(COMMANDMENTS *,char *) ;
static int	commandments_sysidname(COMMANDMENTS *,char *) ;

static int	commandments_idxbegin(COMMANDMENTS *,cchar *) ;
static int	commandments_idxend(COMMANDMENTS *) ;
static int	commandments_idxmkname(COMMANDMENTS *,char *,cchar *) ;
static int	commandments_idxopencheck(COMMANDMENTS *,cchar *) ;
static int	commandments_idxmk(COMMANDMENTS *,cchar *) ;
static int	commandments_idxmapbegin(COMMANDMENTS *,cchar *) ;
static int	commandments_idxmapend(COMMANDMENTS *) ;
static int	commandments_chownpr(COMMANDMENTS *,cchar *) ;
static int	commandments_ids(COMMANDMENTS *) ;

static int	mkent_start(MKENT *,int,uint,uint) ;
static int	mkent_add(MKENT *,uint,uint) ;
static int	mkent_finish(MKENT *) ;

static int	cmimkent_start(CMIMK_ENT *,MKENT *) ;
static int	cmimkent_finish(CMIMK_ENT *) ;

static int	isempty(const char *,int) ;
static int	isstart(const char *,int,int *,int *) ;
static int	hasourdig(const char *,int) ;

#if	CF_DEBUGS && CF_DEBUGLINE
static int	linenlen(cchar *,int,int) ;
#endif

static int	isNotOurAccess(int) ;
static int	isStale(int) ;


/* exported variables */

COMMANDMENTS_OBJ	commandments = {
	"commandments",
	sizeof(COMMANDMENTS),
	sizeof(COMMANDMENTS_CUR)
} ;


/* local variables */


/* exported subroutines */


int commandments_open(COMMANDMENTS *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("commandments_open: ent pr=%s dbname=%s\n",pr,dbname) ;
#endif

	if ((dbname == NULL) || (dbname[0] == '\0')) {
	    dbname = COMMANDMENTS_DBNAME ;
	}

	memset(op,0,sizeof(COMMANDMENTS)) ;
	op->uid = -1 ;
	op->uid_pr = -1 ;
	op->gid_pr = -1 ;

	if ((rs = commandments_argsbegin(op,pr,dbname)) >= 0) {
	    if ((rs = commandments_findbegin(op,pr,dbname)) >= 0) {
	        if ((rs = commandments_fileloadbegin(op,dbname)) >= 0) {
		    c = op->nents ;
		    op->magic = COMMANDMENTS_MAGIC ;
	        } /* end if (commandments_fileloadbegin) */
	        if (rs < 0)
		    commandments_findend(op) ;
	    } /* end if (commandments_findbegin) */
	    if (rs < 0)
		commandments_argsend(op) ;
	} /* end if (commandments_argsbegin) */

#if	CF_DEBUGS
	debugprintf("commandments_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandments_open) */


int commandments_close(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

	rs1 = commandments_fileloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	if (op->uhome != NULL) {
	    rs1 = uc_free(op->uhome) ;
	    if (rs >= 0) rs = rs1 ;
	    op->uhome = NULL ;
	}

	rs1 = commandments_argsend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (commandments_close) */


int commandments_audit(COMMANDMENTS *op)
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->f.idx) {
	    rs = cmi_audit(&op->idx) ;
	}

	return rs ;
}
/* end subroutine (commandments_audit) */


int commandments_count(COMMANDMENTS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

	if (op->f.idx) {
	    CMI		*cip = &op->idx ;
	    rs = cmi_count(cip) ;
	} else {
	    rs = SR_BUGCHECK ;
	}

	return rs ;
}
/* end subroutine (commandments_count) */


int commandments_max(COMMANDMENTS *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("commandments_max: ret rs=%d\n",op->maxent) ;
#endif

	return op->maxent ;
}
/* end subroutine (commandments_max) */


int commandments_read(COMMANDMENTS *op,char *vbuf,int vlen,uint cn)
{
	CMI_ENT		viv ;
	CMI_LINE	lines[COMMANDMENTS_NLINES+1] ;
	time_t		dt = 0 ;
	const int	nlines = COMMANDMENTS_NLINES ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("commandments_read: ent cn=%u\n",cn) ;
#endif

/* check for update */

	if (op->ncursors == 0) {
	    rs = commandments_checkupdate(op,dt) ;
	}

#if	CF_DEBUGS
	debugprintf("commandments_read: mid1 rs=%u\n",rs) ;
#endif

	if (rs >= 0) {
	    const int	lsize = ((nlines+1) * sizeof(CMI_LINE)) ;
	    char	*lb = (char *) lines ;
	    if ((rs = cmi_read(&op->idx,&viv,lb,lsize,cn)) >= 0) {
	        if (vbuf != NULL) {
	            rs = commandments_loadbuf(op,&viv,vbuf,vlen) ;
		    len = rs ;
		}
	    }
	}

#if	CF_DEBUGS
	debugprintf("commandments_read: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (commandments_read) */


int commandments_curbegin(COMMANDMENTS *op,COMMANDMENTS_CUR *curp)
{
	int		rs ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if ((rs = cmi_curbegin(&op->idx,&curp->vicur)) >= 0) {
	    op->ncursors += 1 ;
	}

	return rs ;
}
/* end subroutine (commandments_curbegin) */


int commandments_curend(COMMANDMENTS *op,COMMANDMENTS_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	rs1 = cmi_curend(&op->idx,&curp->vicur) ;
	if (rs >= 0) rs = rs1 ;

	if (op->ncursors > 0) {
	    op->ncursors -= 1 ;
	}

	return rs ;
}
/* end subroutine (commandments_curend) */



int commandments_enum(op,curp,ep,vbuf,vlen)
COMMANDMENTS	*op ;
COMMANDMENTS_C	*curp ;
COMMANDMENTS_E	*ep ;
char		*vbuf ;
int		vlen ;
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		len = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENTS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

#ifdef	COMMENT
	if (vbuf == NULL) return SR_FAULT ;
#endif

/* check for update */

	if (op->ncursors == 0) {
	    rs = commandments_checkupdate(op,dt) ;
	}

	if (rs >= 0) {
	    CMI_CUR	*bcurp = &curp->vicur ;
	    CMI_ENT	viv ;
	    CMI_LINE	lines[COMMANDMENTS_NLINES + 1] ;
	    const int	ls = ((COMMANDMENTS_NLINES + 1) * sizeof(CMI_LINE)) ;
	    if ((rs = cmi_enum(&op->idx,bcurp,&viv,(char *) lines,ls)) >= 0) {
	        if (vbuf != NULL) {
	            rs = commandments_loadbuf(op,&viv,vbuf,vlen) ;
		    len = rs ;
		}
	        if ((rs >= 0) && (ep != NULL)) {
		    memset(ep,0,sizeof(COMMANDMENTS_E)) ;
		    ep->cn = viv.cn ;
	        }
	    } /* end if (cmi_enum) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("commandments_enum: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (commandments_enum) */


int commandments_get(COMMANDMENTS *op,int cn,char *rbuf,int rlen)
{
	return commandments_read(op,rbuf,rlen,cn) ;
}
/* end subroutine (commandments_get) */


/* private subroutines */


static int commandments_argsbegin(COMMANDMENTS *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;
	size += (strlen(pr)+1) ;
	size += (strlen(dbname)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->pr = bp ;
	    bp = (strwcpy(bp,pr,-1)+1) ;
	    op->dbname = bp ;
	    bp = (strwcpy(bp,dbname,-1)+1) ;
	}
	return rs ;
}
/* end subroutine (commandments_argsbegin) */


static int commandments_argsend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->a  != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}
	return rs ;
}
/* end subroutine (commandments_argsend) */


static int commandments_findbegin(COMMANDMENTS *op,cchar *pr,cchar *dbname)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("commandments_findbegin: ent db=%s\n",dbname) ;
#endif
	if ((rs = commandments_userhome(op)) >= 0) {
	    cchar	*cname = COMMANDMENTS_DBDNAME ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath3(tbuf,op->uhome,cname,dbname)) >= 0) {
	        USTAT	sb ;
		int	tl = rs ;
		if ((rs = uc_stat(tbuf,&sb)) >= 0) {
		    if (S_ISREG(sb.st_mode)) {
			op->f.user = TRUE ;
			op->ti_db = sb.st_mtime ;
			op->size_db = sb.st_size ;
		    } else {
			rs = SR_ISDIR ;
		    }
		} else if (isNotOurAccess(rs)) {
	   	    if ((rs = mkpath3(tbuf,pr,cname,dbname)) >= 0) {
			tl = rs ;
			if ((rs = uc_stat(tbuf,&sb)) >= 0) {
		            if (S_ISREG(sb.st_mode)) {
				op->ti_db = sb.st_mtime ;
				op->size_db = sb.st_size ;
				rs = commandments_tmpcheck(op,tbuf,&sb,dbname) ;
				tl = rs ;
			    } else {
				rs = SR_ISDIR ;
			    }
			}
		    }
		} /* end if (user or system) */
		if (rs >= 0) {
		    cchar	*cp ;
		    if ((rs = uc_mallocstrw(tbuf,tl,&cp)) >= 0) {
		        op->fname = cp ;
		    }
		} /* end if (ok) */
	    } /* end if (mkpath) */
	} /* end if (commandments_userhome) */
#if	CF_DEBUGS
	debugprintf("commandments_findbegin: ret rs=%d\n",rs) ;
	debugprintf("commandments_findbegin: ret fname=%s\n",op->fname) ;
#endif
	return rs ;
}
/* end subroutine (commandments_findbegin) */


static int commandments_findend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}
	return rs ;
}
/* end subroutine (commandments_findend) */


static int commandments_tmpcheck(COMMANDMENTS *op,char *tbuf,USTAT *sbp,
		cchar *dbname)
{
	const mode_t	dm = 0777 ;
	const int	plen = MAXPATHLEN ;
	int		rs ;
	int		size = 0 ;
	cchar		*vtmp = "/var/tmp" ;
	cchar		*sn = COMMANDMENTS_SN ;
	char		*bp ;
	size += (plen+1) ;
	size += (plen+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    char	*abuf = bp ;
	    char	*dbuf = (bp+(plen+1)) ;
	    if ((rs = prmktmpdir(op->pr,dbuf,vtmp,sn,dm)) >= 0) {
	        if ((rs = mkpath2(abuf,dbuf,dbname)) >= 0) {
		    USTAT	tsb ;
		    if ((rs = uc_stat(abuf,&tsb)) >= 0) {
			if (S_ISREG(tsb.st_mode)) {
			    int	f = FALSE ;
			    f = f || (sbp->st_mtime > tsb.st_mtime) ;
			    f = f || (sbp->st_size != tsb.st_size) ;
			    if (f) {
				rs = commandments_tmpcopy(op,tbuf,abuf,dbuf) ;
			    }
			} else {
			    if ((rs = removes(abuf)) >= 0) {
				rs = commandments_tmpcopy(op,tbuf,abuf,dbuf) ;
			    }
			}
		    } else if (isNotPresent(rs)) {
			rs = commandments_tmpcopy(op,tbuf,abuf,dbuf) ;
		    } /* end if (stat) */
	        } /* end if (mkpath) */
	    } /* end if (prmktmpdir) */
	    uc_free(bp) ;
	} /* end if (m-a-f) */
	return rs ;
}
/* end subroutine (commandments_tmpcheck) */


static int commandments_tmpcopy(COMMANDMENTS *op,char *tbuf,
		cchar *abuf, char *dbuf)
{
	const int	xlen = MAXPATHLEN ;
	int		rs ;
	char		*xbuf ;
	if ((rs = uc_malloc((xlen+1),&xbuf)) >= 0) {
	    const int	dlen = strlen(dbuf) ;
	    cchar	*ft = "cmdXXXXXXXXXXX" ;
	    if ((rs = pathadd(dbuf,dlen,ft)) >= 0) {
		const mode_t	om = 0664 ;
		const int	of = O_RDWR ;
	        if ((rs = opentmpfile(dbuf,of,om,xbuf)) >= 0) {
		    const int	fd = rs ;
		    rs = uc_writefile(fd,tbuf) ;
		    if ((rs < 0) && (xbuf[0] != '\0')) {
			u_unlink(xbuf) ;
		    }
		    u_close(fd) ;
		} /* end if (open-file) */
		if (rs >= 0) {
		        struct utimbuf	ut ;
			ut.actime = op->ti_db ;
			ut.modtime = op->ti_db ;
			if ((rs = uc_utime(xbuf,&ut)) >= 0) {
			    if ((rs = u_rename(xbuf,abuf)) >= 0) {
			        xbuf[0] = '\0' ;
			        rs = mkpath1(tbuf,abuf) ;
			    }
			} /* end if (uc_utime) */
		} /* end if (ok) */
		dbuf[dlen] = '\0' ;
	    } /* end if (pathadd) */
	    uc_free(xbuf) ;
	} /* end if (m-a-f) */
	return rs ;
}
/* end subroutine (commandments_tmpcopy) */


static int commandments_fileloadbegin(COMMANDMENTS *op,cchar *dbname)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("commandments_fileloadbegin: ent dbname=%s\n",dbname) ;
#endif

	if ((rs = commandments_dbmapbegin(op,dt)) >= 0) {
	    rs = commandments_idxbegin(op,dbname) ;
	    c = rs ;
	    if (rs < 0)
	        commandments_dbmapend(op) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("commandments_fileloadbegin: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandments_fileloadbegin) */


static int commandments_fileloadend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = commandments_idxend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = commandments_dbmapend(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (commandments_fileloadend) */


static int commandments_idxbegin(COMMANDMENTS *op,cchar *dbname)
{
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;
#if	CF_DEBUGS
	debugprintf("commandments_idxbegin: ent dbname=%s\n",dbname) ;
#endif
	if ((rs = commandments_idxmkname(op,tbuf,dbname)) >= 0) {
#if	CF_DEBUGS
	    debugprintf("commandments_idxbegin: tbuf=%s\n",tbuf) ;
#endif
	    rs = commandments_idxmapbegin(op,tbuf) ;
	} /* end if (commandments_idxmkname) */
#if	CF_DEBUGS
	debugprintf("commandments_idxbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (commandments_idxbegin) */


static int commandments_idxend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = commandments_idxmapend(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (commandments_idxend) */


static int commandments_idxmkname(COMMANDMENTS *op,char *tbuf,cchar *dbname)
{
	int		rs = SR_OK ;
	int		pl = 0 ;
#if	CF_DEBUGS
	debugprintf("commandments_idxmkname: ent dbname=%s\n",dbname) ;
	debugprintf("commandments_idxmkname: f_user=%u\n",op->f.user) ;
#endif
	tbuf[0] = '\0' ;
	    if (op->f.user) {
	        if ((rs = commandments_userhome(op)) >= 0) {
		    if ((rs = commandments_usridname(op,tbuf)) >= 0) {
		        rs = pathadd(tbuf,rs,dbname) ;
			pl = rs ;
		    }
	        }
	    } else {
		    if ((rs = commandments_sysidname(op,tbuf)) >= 0) {
		        rs = pathadd(tbuf,rs,dbname) ;
			pl = rs ;
		    }
	    }
#if	CF_DEBUGS
	debugprintf("commandments_idxmkname: ret rs=%d pl=%u\n",rs,pl) ;
	debugprintf("commandments_idxmkname: ret tbuf=%s\n",tbuf) ;
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (commandments_idxmkname) */


static int commandments_idxopencheck(COMMANDMENTS *op,cchar *dbname)
{
	CMI		*cip = &op->idx ;
	int		rs ;
	int		nents = 0 ;

#if	CF_DEBUGS
	debugprintf("commandments_idxopencheck: ent dbname=%s\n",dbname) ;
#endif

	if ((rs = cmi_open(cip,dbname)) >= 0) {
	    CMI_INFO	cinfo ;

#if	CF_DEBUGS
	    debugprintf("commandments_idxopencheck: cmi_open() rs=%d\n",rs) ;
#endif

	    if ((rs = cmi_info(cip,&cinfo)) >= 0) {
		int	f = TRUE ;
#if	CF_DEBUGS
		{
		    char	wbuf[TIMEBUFLEN+1] ;
		    timestr_logz(op->ti_db,wbuf) ;
	            debugprintf("commandments_idxopencheck: ti_db=%s\n",wbuf) ;
		    timestr_logz(cinfo.idxctime,wbuf) ;
	            debugprintf("commandments_idxopencheck: c.time=%s\n",wbuf) ;
	            debugprintf("commandments_idxopencheck: size_db=%u\n",
			op->size_db) ;
	            debugprintf("commandments_idxopencheck: size_info=%u\n",
			cinfo.dbsize) ;
		}
#endif /* CF_DEBUGS */
	        f = f && (cinfo.idxctime >= op->ti_db) ;
	        f = f && (cinfo.dbsize == op->size_db) ;
		if (f) {
	            op->f.idx = TRUE ;
		    op->nents = cinfo.nents ;
		    op->maxent = cinfo.maxent ;
		    op->ti_idx = cinfo.idxmtime ;
		    nents = cinfo.nents ;
		} else {
	            rs = SR_STALE ;
	        }
	        if (rs < 0)
	            cmi_close(cip) ;
	    } /* end if (cmi_open) */

	} /* end if (cmi_open) */

#if	CF_DEBUGS
	debugprintf("commandments_idxopencheck: ret rs=%d n=%u\n",
	    rs,nents) ;
#endif

	return (rs >= 0) ? nents : rs ;
}
/* end subroutine (commandments_idxopencheck) */


static int commandments_idxmk(COMMANDMENTS *op,cchar *tbuf)
{
	CMIMK		mk ;
	const mode_t	om = 0664 ;
	const int	of = 0 ;
	int		rs ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("commandments_idxmk: ent tbuf=%s\n",tbuf) ;
#endif
	if ((rs = cmimk_open(&mk,tbuf,of,om)) >= 0) {
	    if ((rs = cmimk_setdb(&mk,op->size_db,op->ti_db)) >= 0) {
		rs = commandments_dbproc(op,&mk) ;
	    }
	    rs1 = cmimk_close(&mk) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cmimk) */
#if	CF_DEBUGS
	debugprintf("commandments_idxmk: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (commandments_idxmk) */


static int commandments_idxmapbegin(COMMANDMENTS *op,cchar *tbuf)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("commandments_idxmapbegin: ent tbuf=%s\n",tbuf) ;
#endif
	if ((rs = commandments_idxopencheck(op,tbuf)),isStale(rs)) {
#if	CF_DEBUGS
	    debugprintf("commandments_idxmapbegin: isStale rs=%d n=%u\n",rs) ;
#endif
	    if ((rs = commandments_idxmk(op,tbuf)) >= 0) {
		rs = commandments_idxopencheck(op,tbuf) ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("commandments_idxmapbegin: ret rs=%d n=%u\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (commandments_idxmapbegin) */


static int commandments_idxmapend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->f.idx) {
	    CMI		*cip = &op->idx ;
	    rs1 = cmi_close(cip) ;
	    if (rs >= 0) rs = rs1 ;
	    op->f.idx = FALSE ;
	}
	return rs ;
}
/* end subroutine (commandments_idxmapend) */


static int commandments_usridname(COMMANDMENTS *op,char *tbuf)
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	if (tbuf[0] == '\0') {
	    cchar	*vd = "var" ;
	    cchar	*sn = COMMANDMENTS_SN ;
	    if ((rs = mkpath3(tbuf,op->uhome,vd,sn)) >= 0) {
		USTAT		sb ;
		const int	rsn = SR_NOTFOUND ;
	        rl = rs ;
		if ((rs = uc_stat(tbuf,&sb)) == rsn) {
	            const mode_t	dm = 0775 ;
	            if ((rs = mkdirs(tbuf,dm)) >= 0) {
			if ((rs = uc_minmod(tbuf,dm)) >= 0) {
	    	                const int	n = _PC_CHOWN_RESTRICTED ;
	    	                if ((rs = u_pathconf(tbuf,n,NULL)) == 0) {
		                    rs = commandments_chownpr(op,tbuf) ;
		                }
			} /* end if (uc_chmod) */
		    } /* end if (mkdirs) */
		} /* end if (uc_stat) */
	    } /* end if (mkpath) */
	} else {
	    rl = strlen(tbuf) ;
	}
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (commandments_usridname) */


static int commandments_sysidname(COMMANDMENTS *op,char *tbuf)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		rl = 0 ;
#if	CF_DEBUGS
	debugprintf("commandments_sysidname: ent pr=%s\n",op->pr) ;
#endif
	if (tbuf[0] == '\0') {
	    int		prnl ;
	    cchar	*prnp ;
	    if ((rs = sfrootname(op->pr,-1,&prnp)) > 0) {
		NULSTR	spr ;
		cchar	*prn ;
		prnl = rs ;
		if ((rs = nulstr_start(&spr,prnp,prnl,&prn)) >= 0) {
	            cchar	*tmpdname = COMMANDMENTS_TMPDNAME ;
	            cchar	*sn = COMMANDMENTS_SN ;
	            if ((rs = mkpath3(tbuf,tmpdname,prn,sn)) >= 0) {
		        USTAT		sb ;
		        const int	rsn = SR_NOTFOUND ;
	                rl = rs ;
		        if ((rs = uc_stat(tbuf,&sb)) == rsn) {
	                    const mode_t	dm = 0777 ;
	                    if ((rs = mkdirs(tbuf,dm)) >= 0) {
				if ((rs = uc_chmod(tbuf,dm)) >= 0) {
	    	                    const int	n = _PC_CHOWN_RESTRICTED ;
	    	                    if ((rs = u_pathconf(tbuf,n,NULL)) == 0) {
		                        rs = commandments_chownpr(op,tbuf) ;
				    }
		                } /* end if (uc_chmod) */
		            } /* end if (mkdirs) */
		        } /* end if (stat) */
	            } /* end if (mkpath) */
		    rs1 = nulstr_finish(&spr) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (nulstr) */
	    } else {
		rs = SR_NOENT ;
	    }
	} else {
	    rl = strlen(tbuf) ;
	}
#if	CF_DEBUGS
	debugprintf("commandments_sysidname: ret rs=%d rl=%u\n",rs,rl) ;
#endif
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (commandments_sysidname) */


static int commandments_dbmapbegin(COMMANDMENTS *op,time_t dt)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("commandments_dbmapbegin: ent fn=%s\n",op->fname) ;
#endif

	if ((rs = u_open(op->fname,O_RDONLY,0666)) >= 0) {
	    USTAT	sb ;
	    const int	fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
	            size_t	ms = (size_t) sb.st_size ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*md ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                op->data_db = md ;
	                op->size_db = ms ;
	                op->ti_map = dt ;
	            } /* end if (u_mmap) */
	        } else
	            rs = SR_NOTSUP ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (file) */

#if	CF_DEBUGS
	debugprintf("commandments_dbmapbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandments_dbmapbegin) */


static int commandments_dbmapend(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->data_db != NULL) {
	    rs1 = u_munmap(op->data_db,op->size_db) ;
	    if (rs >= 0) rs = rs1 ;
	    op->data_db = NULL ;
	}

	return rs ;
}
/* end subroutine (commandments_dbmapend) */


static int commandments_dbproc(COMMANDMENTS *op,CMIMK *cmp)
{
	MKENT		e ;
	uint		foff = 0 ;
	int		rs = SR_OK ;
	int		ml = (op->size_db & INT_MAX) ;
	int		ll ;
	int		si ;
	int		len ;
	int		n = 0 ;
	int		c = 0 ;
	int		f_ent = FALSE ;
	const char	*tp, *lp ;
	const char	*mp = op->data_db ;

#if	CF_DEBUGS
	debugprintf("commandments_dbproc: ent ml=%d\n",ml) ;
#endif

	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;
	    lp = mp ;
	    ll = (len - 1) ;

	    if (! isempty(lp,ll)) {

	        if ((tp = strnchr(lp,ll,'#')) != NULL) {
	            ll = (tp - lp) ;
		}

	        if (isstart(lp,ll,&n,&si)) {

	            if (f_ent) {
			CMIMK_ENT	ce ;
	                c += 1 ;
		        if ((rs = cmimkent_start(&ce,&e)) >= 0) {
	                    rs = cmimk_add(cmp,&ce) ;
			    cmimkent_finish(&ce) ;
			}
			f_ent = FALSE ;
	                mkent_finish(&e) ;
	            } /* end if (entry) */

	            if (rs >= 0) {
			const uint	soff = (foff+si) ;
			const int	slen = (ll-si) ;
	                if ((rs = mkent_start(&e,n,soff,slen)) >= 0) {
	                    f_ent = TRUE ;
			}
	            } /* end if (ok) */

	        } else {

	            if (f_ent) {
	                rs = mkent_add(&e,foff,ll) ;
		    }

	        } /* end if (entry start of add) */

	    } else {

#if	CF_EMPTYTERM
	        if (f_ent) {
	    	    CMIMK_ENT	ce ;
	    	    c += 1 ;
	    	    if ((rs = cmimkent_start(&ce,&e)) >= 0) {
			rs = cmimk_add(cmp,&ce) ;
			cmimkent_finish(&ce) ;
	    	    }
	    	    f_ent = FALSE ;
	    	    mkent_finish(&e) ;
	        }
#else
	        rs = SR_OK ;
#endif /* CF_EMPTYTERM */

	    } /* end if (not empty) */

	    foff += len ;
	    ml -= len ;
	    mp += len ;

	    if (rs < 0) break ;
	} /* end while (readling lines) */

	if ((rs >= 0) && f_ent) {
	    CMIMK_ENT	ce ;
	    c += 1 ;
	    if ((rs = cmimkent_start(&ce,&e)) >= 0) {
	        rs = cmimk_add(cmp,&ce) ;
	        cmimkent_finish(&ce) ;
	    }
	    f_ent = FALSE ;
	    mkent_finish(&e) ;
	} /* end if (entry) */

	if (f_ent) {
	    mkent_finish(&e) ;
	}

#if	CF_DEBUGS
	debugprintf("commandments_dbproc: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandments_dbproc) */


static int commandments_checkupdate(COMMANDMENTS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op->ncursors == 0) {
	    struct ustat	sb ;
	    if (dt == 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	        op->ti_lastcheck = dt ;
	        if ((rs = u_stat(op->fname,&sb)) >= 0) {
	            f = f || (sb.st_mtime > op->ti_db) ;
		    f = f || (sb.st_mtime > op->ti_map) ;
	            if (f) {
			cchar	*dbname = op->dbname ;
	                commandments_fileloadend(op) ;
	                rs = commandments_fileloadbegin(op,dbname) ;
	            } /* end if (update) */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        } /* end if (stat) */
	    } /* end if (timed-out) */
	} /* end if (no cursors out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (commandments_checkupdate) */


static int commandments_loadbuf(COMMANDMENTS *op,CMI_ENT *vivp,
		char *rbuf,int rlen)
{
	SBUF		b ;
	int		rs ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("commandments_loadbuf: ent eoff=%u elen=%u\n",
		vivp->eoff,vivp->elen) ;
#endif

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    CMI_LINE	*lines = vivp->lines ;
	    const int	nlines = vivp->nlines ;
	    int		i ;
	    int		ll ;
	    const char	*lp ;

	    for (i = 0 ; i < nlines ; i += 1) {

	        if (i > 0)
		    sbuf_char(&b,' ') ;

	        lp = (op->data_db + lines[i].loff) ;
	        ll = lines[i].llen ;
	        rs = sbuf_strw(&b,lp,ll) ;

	        if (rs < 0) break ;
	    } /* end for */

	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */

	return (rs >= 0) ? len : rs ;
} 
/* end subroutine (commandments_loadbuf) */


static int commandments_userhome(COMMANDMENTS *op)
{
	int		rs ;
	if (op->uhome == NULL) {
	    const int	hlen = MAXPATHLEN ;
	    char	*hbuf ;
	    if ((rs = uc_malloc((hlen+1),&hbuf)) >= 0) {
	        if ((rs = getuserhome(hbuf,hlen,"-")) >= 0) {
		    cchar	*uh ;
		    if ((rs = uc_mallocstrw(hbuf,rs,&uh)) >= 0) {
		        op->uhome = uh ;
		    }
	        } /* end if (getuserhome) */
		uc_free(hbuf) ;
	    } /* end if (m-a-f) */
	} else {
	    rs = strlen(op->uhome) ;
	}
	return rs ;
}
/* end subroutine (commandments_userhome) */


static int commandments_chownpr(COMMANDMENTS *op,cchar *tbuf)
{
	int		rs ;
	if (op->uid < 0) op->uid = getuid() ;
	if ((rs = commandments_ids(op)) >= 0) {
	    const uid_t	uid_pr = op->uid_pr ;
	    const gid_t	gid_pr = op->gid_pr ;
	    rs = u_chown(tbuf,uid_pr,gid_pr) ;
	} /* end if (commandments_ids) */
	return rs ;
}
/* end subroutine (commandments_chownpr) */


static int commandments_ids(COMMANDMENTS *op)
{
	int		rs = SR_OK ;
	if (! op->f.ids) {
	    USTAT	sb ;
	    op->f.ids = TRUE ;
	    if ((rs = uc_stat(op->pr,&sb)) >= 0) {
		op->uid_pr = sb.st_uid ;
		op->gid_pr = sb.st_gid ;
	    }
	} /* end if (needed) */
	return rs ;
}
/* end subroutine (commandments_ids) */


static int mkent_start(MKENT *ep,int cn,uint eoff,uint elen)
{
	const int	ne = CMIMK_NE ; /* use their value for starters */
	int		rs = SR_OK ;
	int		size ;
	void		*p ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkent_start: ent cn=%u\n",cn) ;
#endif

	memset(ep,0,sizeof(MKENT)) ;
	ep->cn = cn ;
	ep->eoff = eoff ;
	ep->elen = elen ;

	size = (ne * sizeof(CMIMK_LINE)) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    ep->lines = p ;
	    ep->e = ne ;
	    ep->i += 1 ;
	    {
	        CMIMK_LINE *elp = p ;
	        elp->loff = eoff ;
	        elp->llen = elen ;
	    } /* end block (first line) */
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("mkent_start: ret rs=%d i=%u\n",rs,ep->i) ;
#endif

	return rs ;
}
/* end subroutine (mkent_start) */


static int mkent_finish(MKENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkent_finish: ent e=%u i=%u\n",ep->e,ep->i) ;
	debugprintf("mkent_finish: i=%u\n",ep->i) ;
#endif

	if (ep->lines != NULL) {
	    rs1 = uc_free(ep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->lines = NULL ;
	}

	return rs ;
}
/* end subroutine (mkent_finish) */


static int mkent_add(MKENT *ep,uint eoff,uint elen)
{
	MKENT_LINE	*elp ;
	int		rs = SR_OK ;
	int		ne ;
	int		size ;

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkent_add: ent e=%u i=%u\n",ep->e,ep->i) ;
#endif

	if (ep->e <= 0) return SR_NOTOPEN ;

	if ((ep->i < 0) || (ep->i > ep->e)) return SR_BADFMT ;

	if (ep->i == ep->e) {
	    ne = (ep->e + CMIMK_NE) ;
	    size = (ne * sizeof(MKENT_LINE)) ;
	    if ((rs = uc_realloc(ep->lines,size,&elp)) >= 0) {
	        ep->lines = elp ;
		ep->e = ne ;
	    }
	}

	if (rs >= 0) {
	    elp = (ep->lines + ep->i) ;
	    elp->loff = eoff ;
	    elp->llen = elen ;
	    ep->i += 1 ;
	}

	return rs ;
}
/* end subroutine (mkent_add) */


static int cmimkent_start(CMIMK_ENT *bvep,MKENT *ep)
{
	uint		nlines = 0 ;
	int		rs = SR_OK ;

	if (ep == NULL) return SR_FAULT ;

	memset(bvep,0,sizeof(CMIMK_ENT)) ;
	bvep->cn = ep->cn ;
	bvep->eoff = ep->eoff ;
	bvep->elen = ep->elen ;
	bvep->lines = NULL ;

	nlines = ep->i ;

#if	CF_DEBUGS
	debugprintf("cmimkent_start: nlines=%u\n",nlines) ;
#endif

	if (nlines <= USHORT_MAX) {
	    CMIMK_LINE	*lines ;
	    int		size ;
	    bvep->nlines = nlines ;
	    size = ((nlines + 1) * sizeof(CMIMK_LINE)) ;
	    if ((rs = uc_malloc(size,&lines)) >= 0) {
	        int	i ;
	        bvep->lines = lines ;
	        for (i = 0 ; i < nlines ; i += 1) {
	            lines[i].loff = ep->lines[i].loff ;
	            lines[i].llen = ep->lines[i].llen ;
	        }
	        lines[i].loff = 0 ;
	        lines[i].llen = 0 ;
	    } /* end if (memory-allocation) */
	} else {
	    rs = SR_TOOBIG ;
	}

#if	CF_DEBUGS
	debugprintf("cmimkent_start: ret rs=%d nlines=%u\n",rs,nlines) ;
#endif

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (cmimkent_start) */


static int cmimkent_finish(CMIMK_ENT *bvep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (bvep == NULL) return SR_FAULT ;

	if (bvep->lines != NULL) {
	    rs1 = uc_free(bvep->lines) ;
	    if (rs >= 0) rs = rs1 ;
	    bvep->lines = NULL ;
	}

	return rs ;
}
/* end subroutine (cmimkent_finish) */


static int isempty(cchar *lp,int ll)
{
	int		cl ;
	int		f = FALSE ;
	const char	*cp ;

	f = f || (ll == 0) ;
	f = f || (lp[0] == '#') ;
	if ((! f) && CHAR_ISWHITE(*lp)) {
	    cl = sfskipwhite(lp,ll,&cp) ;
	    f = f || (cl == 0) ;
	    f = f || (cp[0] == '#') ;
	}

	return f ;
}
/* end subroutine (isempty) */


static int isstart(cchar *lp,int ll,int *np,int *sip)
{
	int		cl ;
	int		f = FALSE ;
	const char	*tp, *cp ;

	*np = -1 ;
	*sip = 0 ;
	if ((tp = strnchr(lp,ll,'.')) != NULL) {

	    cp = lp ;
	    cl = (tp - lp) ;
	    f = hasourdig(cp,cl) && (cfdeci(cp,cl,np) >= 0) ;

	    if (f) {
	        *sip = ((tp + 1) - lp) ;
	    }

	} /* end if */

	return f ;
}
/* end subroutine (isstart) */


static int hasourdig(cchar *sp,int sl)
{
	int		cl ;
	int		f = FALSE ;
	const char	*cp ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    f = TRUE ;
	    while (cl && *cp) {
	        f = isdigitlatin(*cp) ;
		if (!f) break ;
	        cp += 1 ;
	        cl -= 1 ;
	    } /* end while */
	} /* end if */

	return f ;
}
/* end subroutine (hasourdig) */


#if	CF_DEBUGS && CF_DEBUGLINE
static int linenlen(cchar *lp,int ll,int ml)
{
	int		len = INT_MAX ;
	const char	*tp ;
	if (lp == NULL) return 0 ;
	if (ll > 0) len = MIN(len,ll) ;
	if (ml > 0) len = MIN(len,ml) ;
	if ((tp = strnchr(lp,len,'\n')) != NULL) {
	    len = (tp - lp) ;
	}
	return len ;
}
/* end subroutine (linenlen) */
#endif /* CF_DEBUGS */


int isNotOurAccess(int rs)
{
	int		f = FALSE ;
	if (rs < 0) {
	    f = f || isNotPresent(rs) ;
	    f = f || (rs == SR_ISDIR) ;
	}
	return f ;
}
/* end subroutine (isNotOurAccess) */


static int isStale(int rs)
{
	int		f = FALSE ;
	if (rs < 0) {
	    f = f || isNotPresent(rs) ;
	    f = f || isOneOf(rsold,rs) ;
	}
	return f ;
}
/* end subroutine (isStale) */


