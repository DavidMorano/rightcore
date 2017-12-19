/* paramfile */

/* read in a file of parameters */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFIELD	1		/* extra debug print-outs */


/* revision history:

	= 2000-02-17, David A­D­ Morano
	Some code for this subroutine was taken from something that did
	something similar to what we are doing here.  The rest was originally
	written for LevoSim.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object reads in the parameter file and makes the parameter pairs
	available thought a key search.


*******************************************************************************/


#define	PARAMFILE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<char.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"paramfile.h"


/* local defines */

#define	PARAMFILE_FILE		struct paramfile_file

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	getpwd(char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct paramfile_file {
	const char	*fname ;
	time_t		mtime ;
	dev_t		dev ;
	uino_t		ino ;
	int		size ;
} ;


/* forward references */

int		paramfile_fileadd(PARAMFILE *,const char *) ;

static int	paramfile_bufbegin(PARAMFILE *,int) ;
static int	paramfile_bufend(PARAMFILE *) ;

static int	paramfile_filefins(PARAMFILE *) ;
static int	paramfile_fileparse(PARAMFILE *,int) ;
static int	paramfile_fileparser(PARAMFILE *,int,bfile *) ;
static int	paramfile_fileparseline(PARAMFILE *,int,const char *,int) ;
static int	paramfile_filedump(PARAMFILE *,int) ;
static int	paramfile_filedel(PARAMFILE *,int) ;
static int	paramfile_entadd(PARAMFILE *,PARAMFILE_ENT *) ;
static int	paramfile_entfins(PARAMFILE *) ;
static int	paramfile_entsub(PARAMFILE *,PARAMFILE_ENT *,
			const char **) ;

static int	paramfile_envbegin(PARAMFILE *) ;
static int	paramfile_envload(PARAMFILE *) ;
static int	paramfile_envend(PARAMFILE *) ;

static int	paramfile_defbegin(PARAMFILE *,VECSTR *) ;
static int	paramfile_defload(PARAMFILE *,VECSTR *) ;
static int	paramfile_defend(PARAMFILE *) ;

static int	paramfile_entrels(PARAMFILE *) ;

static int	file_start(struct paramfile_file *,const char *) ;
static int	file_finish(struct paramfile_file *) ;

static int	entry_start(PARAMFILE_ENT *,int,const char *,int,
			VECSTR *,int) ;
static int	entry_release(PARAMFILE_ENT *) ;
static int	entry_finish(PARAMFILE_ENT *) ;

static int	vcmpentry() ;


/* local variables */

/* key field terminators (pound, equal, colon, and all white space) */
static const unsigned char 	kterms[32] = {
	0x00, 0x1B, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

/* argument field terminators (pound and all white space) */
static const unsigned char 	aterms[32] = {
	0x00, 0x1B, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int paramfile_open(PARAMFILE *op,cchar **envv,cchar *fname)
{
	int		rs = SR_OK ;
	int		size ;
	int		opts ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(PARAMFILE)) ;
	op->envv = envv ;
	op->intcheck = PARAMFILE_INTCHECK ;
	op->ti_check = time(NULL) ;

/* initialize */

	opts = VECOBJ_OSTATIONARY ;
	size = sizeof(PARAMFILE_FILE) ;
	if ((rs = vecobj_start(&op->files,size,10,opts)) >= 0) {
	    opts = (VECOBJ_PNOHOLES | VECOBJ_OORDERED) ;
	    size = sizeof(PARAMFILE_ENT) ;
	    if ((rs = vecobj_start(&op->entries,size,10,opts)) >= 0) {
	        if (op->envv != NULL) {
	            rs = paramfile_envbegin(op) ;
	        }
	        if (rs >= 0) {
	            op->magic = PARAMFILE_MAGIC ;
	            if (fname != NULL) {
	                rs = paramfile_fileadd(op,fname) ;
	            }
	        }
	        if (rs < 0) {
	            paramfile_envend(op) ;
	            vecobj_finish(&op->entries) ;
	            op->magic = 0 ;
	        }
	    }
	    if (rs < 0)
	        vecobj_finish(&op->files) ;
	} /* end if (had an initial file to load) */

#if	CF_DEBUGS
	debugprintf("paramfile_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (paramfile_open) */


/* free up the resources occupied by a PARAMFILE list object */
int paramfile_close(PARAMFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("paramfile_close: ent\n") ;
#endif

/* secondary items */

	rs1 = paramfile_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = paramfile_filefins(op) ;
	if (rs >= 0) rs = rs1 ;

/* primary items */

	if (op->f.definit) {
	    op->f.definit = FALSE ;
	    rs1 = varsub_finish(&op->d) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = paramfile_envend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (paramfile_close) */


/* load up the defines for substitution purposes */
int paramfile_setdefines(PARAMFILE *op,VECSTR *dvp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (dvp == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

/* delete any existing ones */

	rs1 = paramfile_defend(op) ;
	if (rs >= 0) rs = rs1 ;

/* clear substitutions */

	rs1 = paramfile_entrels(op) ;
	if (rs >= 0) rs = rs1 ;

/* load the new ones */

	rs1 = paramfile_defbegin(op,dvp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (paramfile_setdefines) */


/* add a file to the list of files */
int paramfile_fileadd(PARAMFILE *op,cchar *fname)
{
	int		rs = SR_OK ;
	const char	*fnp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("paramfile_fileadd: fname=%s\n",fname) ;
#endif

	fnp = fname ;
	if (fname[0] != '/') {
	    char	pwd[MAXPATHLEN + 1] ;
	    fnp = tmpfname ;
	    if ((rs = getpwd(pwd,MAXPATHLEN)) >= 0)
	        rs = mkpath2(tmpfname,pwd,fname) ;
	} /* end if (rooting file) */

#if	CF_DEBUGS
	debugprintf("paramfile_fileadd: fnp=%s\n",fnp) ;
#endif

	if (rs >= 0) {
	    PARAMFILE_FILE	fe ;
	    if ((rs = file_start(&fe,fnp)) >= 0) {
	        if ((rs = vecobj_add(&op->files,&fe)) >= 0) {
	            int	fi = rs ;
	            rs = paramfile_fileparse(op,fi) ;
	            if (rs < 0)
	                paramfile_filedel(op,fi) ;
	        }
	        if (rs < 0)
	            file_finish(&fe) ;
	    } /* end if (file-start) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("paramfile_fileadd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (paramfile_fileadd) */


/* cursor manipulations */
int paramfile_curbegin(PARAMFILE *op,PARAMFILE_CUR *cp)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (paramfile_curbegin) */


int paramfile_curend(PARAMFILE *op,PARAMFILE_CUR *cp)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (paramfile_curend) */


/* search the parameters for a match */
int paramfile_fetch(PARAMFILE *op,cchar *key,PARAMFILE_CUR *curp,
	char *vbuf,int vlen)
{
	PARAMFILE_ENT	ke, *pep ;
	VECOBJ		*slp ;
	int		rs ;
	int		i, j ;
	int		vl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

	if (key[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("paramfile_fetch: ent key=%s\n",key) ;
#endif

/* create a key entry */

	ke.fi = -1 ;
	ke.key = (const char *) key ;

/* go! */

	slp = &op->entries ;
	if (curp == NULL) {
	    void	*p ;

#if	CF_DEBUGS
	    debugprintf("paramfile_fetch: NULL cursor\n") ;
#endif

	    rs = vecobj_search(slp,&ke,vcmpentry,&p) ;
	    pep = p ;

	} else {

#if	CF_DEBUGS
	    debugprintf("paramfile_fetch: non-NULL cursor\n") ;
#endif

	    j = (curp->i < 0) ? 0 : (curp->i + 1) ;
	    for (i = j ; (rs = vecobj_get(slp,i,&pep)) >= 0 ; i += 1) {
	        if (pep == NULL) continue ;
	        if (pep->key == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("paramfile_fetch: got entry kl=%d key=%s\n",
	            pep->klen,pep->key) ;
#endif

	        if (strcmp(key,pep->key) == 0)
	            break ;

	    } /* end for (looping through entries) */

	    if (rs >= 0)
	        curp->i = i ;

	} /* end if */

/* OK, if we have one, let's perform the substitutions on it */

	if ((rs >= 0) && (pep != NULL)) {
	    const char	*vp ;

	    rs = paramfile_entsub(op,pep,&vp) ;
	    vl = rs ;
	    if ((rs >= 0) && (vbuf != NULL))
	        rs = sncpy1(vbuf,vlen,vp) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("paramfile_fetch: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (paramfile_fetch) */


/* enumerate the entries */
int paramfile_enum(op,curp,ep,ebuf,elen)
PARAMFILE	*op ;
PARAMFILE_CUR	*curp ;
PARAMFILE_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	PARAMFILE_ENT	*pep ;
	VECOBJ		*slp ;
	int		rs ;
	int		i, j ;
	int		kl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if ((ep == NULL) || (ebuf == NULL)) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("paramfile_enum: ent curi=%d\n",curp->i) ;
#endif

	slp = &op->entries ;
	i = (curp->i < 0) ? 0 : (curp->i + 1) ;
	for (j = i ; (rs = vecobj_get(slp,j,&pep)) >= 0 ; j += 1) {
	    if (pep != NULL) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("paramfile_enum: vecobj_get() j=%d rs=%d\n",j,rs) ;
#endif

	if ((rs >= 0) && (pep != NULL)) {
	    const char	*vp ;
	    char	*bp = ebuf ;

	    if ((rs = paramfile_entsub(op,pep,&vp)) >= 0) {
	        int	vl = rs ;

	        kl = pep->klen ;
	        memset(ep,0,sizeof(PARAMFILE_ENT)) ;

	        if ((elen < 0) || (elen >= (kl + vl + 2))) {

	            ep->key = bp ;
	            ep->klen = pep->klen ;
	            bp = strwcpy(bp,pep->key,pep->klen) + 1 ;

	            ep->value = bp ;
	            ep->vlen = vl ;
	            bp = strwcpy(bp,vp,vl) + 1 ;

	        } else
	            rs = SR_OVERFLOW ;

	    } /* end if (entry-sub) */

	} /* end if (got one) */

	if (rs >= 0)
	    curp->i = j ;

#if	CF_DEBUGS
	debugprintf("paramfile_enum: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (paramfile_enum) */


int paramfile_checkint(PARAMFILE *op,int intcheck)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

	if (intcheck < 1) intcheck = 1 ;
	op->intcheck = intcheck ;
	return SR_OK ;
}
/* end subroutine (paramfile_checkint) */


/* check if the parameter file has changed */
int paramfile_check(PARAMFILE *op,time_t dt)
{
	const int	to = PARAMFILE_INTCHANGE ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c_changed = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PARAMFILE_MAGIC) return SR_NOTOPEN ;

	if (dt == 0)
	    dt = time(NULL) ;

/* should we even check? */

	if ((dt - op->ti_check) > op->intcheck) {
	    struct ustat	sb ;
	    VECOBJ		*flp = &op->files ;
	    PARAMFILE_FILE	*fep ;
	    int			i ;

/* check the files */

	    op->ti_check = dt ;
	    for (i = 0 ; vecobj_get(flp,i,&fep) >= 0 ; i += 1) {
	        if (fep != NULL) {

	        rs1 = u_stat(fep->fname,&sb) ;
	        if ((rs1 >= 0) &&
	            (sb.st_mtime > fep->mtime) &&
	            ((dt - sb.st_mtime) >= to)) {

#if	CF_DEBUGS
	            debugprintf("paramfile_check: file=%d changed\n",i) ;
	            debugprintf("paramfile_check: freeing file entries\n") ;
#endif

	            paramfile_filedump(op,i) ;

#if	CF_DEBUGS
	            debugprintf("paramfile_check: parsing the file again\n") ;
#endif

	            c_changed += 1 ;
	            rs = paramfile_fileparse(op,i) ;

#if	CF_DEBUGS
	            debugprintf("paramfile_check: "
			"paramfile_fileparse() rs=%d\n", rs) ;
#endif

	        } /* end if */

	    }
	    if (rs < 0) break ;
	} /* end for */

	} /* end if (needed) */

#if	CF_DEBUGS
	debugprintf("paramfile_check: ret rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (paramfile_check) */


/* private subroutines */


static int paramfile_bufbegin(PARAMFILE *op,int llen)
{
	int		size ;
	int		rs ;
	char		*bp ;
	if (llen < 0) llen = LINEBUFLEN ;
	size = (2*(llen+1)) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->lbuf = bp ;
	    op->llen = llen ;
	    bp += (llen+1) ;
	    op->fbuf = bp ;
	    op->flen = llen ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (paramfile_bufbegin) */


static int paramfile_bufend(PARAMFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}
	return rs ;
}
/* end subroutine (paramfile_bufend) */


/* parse a parameter file */
static int paramfile_fileparse(PARAMFILE *op,int fi)
{
	PARAMFILE_FILE	*fep = NULL ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("paramfile_fileparse: ent fi=%u\n",fi) ;
#endif

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        bfile	lfile, *lfp = &lfile ;
	        if ((rs = bopen(lfp,fep->fname,"r",0664)) >= 0) {
	            struct ustat	sb ;
	            if ((rs = bcontrol(lfp,BC_STAT,&sb)) >= 0) {
	                if (fep->mtime <  sb.st_mtime) {

	                    fep->dev = sb.st_dev ;
	                    fep->ino = sb.st_ino ;
	                    fep->mtime = sb.st_mtime ;
	                    fep->size = sb.st_size ;

	                    rs = paramfile_fileparser(op,fi,lfp) ;
	                    c = rs ;

	                } /* end if (needed update) */
	            } /* end if (bcontrol) */
	            rs1 = bclose(lfp) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (bfile) */
	    } else 
	        rs = SR_NOTFOUND ;
	} /* end if (vecobj_get) */

#if	CF_DEBUGS
	debugprintf("paramfile_fileparse: bfile rs=%d\n",rs) ;
#endif

	if (rs < 0) paramfile_filedump(op,fi) ;

#if	CF_DEBUGS
	debugprintf("paramfile_fileparse: ret rs=%d added=%u\n",
	    rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramfile_fileparse) */


static int paramfile_fileparser(PARAMFILE *op,int fi,bfile *lfp)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = paramfile_bufbegin(op,llen)) >= 0) {
	    int		len ;
	    int		cl ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    const char	*cp ;
	    char	*lbuf = op->lbuf ;

	    while ((rs = breadlines(lfp,lbuf,llen,NULL)) > 0) {
	        len = rs ;

	        if (len == 1) continue ;	/* blank line */
	        f_eol = (lbuf[len-1] == '\n') ;
	        if (f_eol) len -= 1 ;
	        lbuf[len] = '\0' ;

		if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	            if (f_bol && (*cp != '#')) {
	                rs = paramfile_fileparseline(op,fi,cp,cl) ;
	                c += rs ;
		    }
	        }

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rs1 = paramfile_bufend(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramfile_buf) */

#if	CF_DEBUGS
	debugprintf("paramfile_fileparser: ret rs=%d added=%u\n",
	    rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramfile_fileparser) */


static int paramfile_fileparseline(PARAMFILE *op,int fi,cchar *lp,int ll)
{
	FIELD		fsb ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    int		kl ;
	    const char	*kp ;
	    if ((kl = field_get(&fsb,kterms,&kp)) > 0) {
	        VECSTR	vals ;
	        if ((rs = vecstr_start(&vals,10,0)) >= 0) {
	            PARAMFILE_ENT	pe ;
	            const int	flen = op->flen ;
	            int		fl ;
	            int		vl = 0 ;
	            char	*fbuf = op->fbuf ;
	            while ((fl = field_sharg(&fsb,aterms,fbuf,flen)) >= 0) {
	                vl += (fl + 1) ;
	                rs = vecstr_add(&vals,fbuf,fl) ;
	                if (fsb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while (fields) */

#if	CF_DEBUGS && CF_DEBUGSFIELD
	            debugprintf("paramfile_fileparseline: rs=%d\n",rs) ;
	            debugprintf("paramfile_fileparseline: vl=%u\n",vl) ;
#endif

	            if ((rs = entry_start(&pe,fi,kp,kl,&vals,vl)) >= 0) {
	                c += 1 ;
	                rs = paramfile_entadd(op,&pe) ;

#if	CF_DEBUGS && CF_DEBUGSFIELD
	                debugprintf("paramfile_fileparseline: "
	                    "_addentry() rs=%d\n",rs) ;
#endif

	                if (rs < 0)
	                    entry_finish(&pe) ;

	            } /* end if (entry_start) */

	            rs1 = vecstr_finish(&vals) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (vecstr) */
	    } /* end if (non-zero key) */
	    field_finish(&fsb) ;
	} /* end if (field) */

#if	CF_DEBUGS && CF_DEBUGSFIELD
	debugprintf("paramfile_fileparseline: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramfile_fileparseline) */


/* add an entry to something */
static int paramfile_entadd(PARAMFILE *op,PARAMFILE_ENT *pep)
{
	int		rs ;

	rs = vecobj_add(&op->entries,pep) ;

#if	CF_DEBUGS 
	debugprintf("paramfile_entadd: vecobj_add() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (paramfile_entadd) */


/* perform the variable substitution on an entry */
static int paramfile_entsub(PARAMFILE *op,PARAMFILE_ENT *pep,cchar **vpp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		vl = 0 ;
	const char	*v = NULL ;

#if	CF_DEBUGS
	debugprintf("paramfile_entsub: ent\n") ;
#endif

	if (pep == NULL) return SR_FAULT ;

	if (pep->oval != NULL) {

#if	CF_DEBUGS
	debugprintf("paramfile_entsub: oval=>%s<\n",pep->oval) ;
#endif

	vl = pep->vlen ;
	if (pep->value == NULL) {
	    BUFFER	b ;
	    const int	start = MAX(pep->vlen,USERNAMELEN) ;
	    int		f_exptry = FALSE ;
	    const char	*vp ;

	    if ((rs = buffer_start(&b,start)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("paramfile_entsub: processing\n") ;
#endif

	        if (op->f.definit) {

#if	CF_DEBUGS
	            debugprintf("paramfile_entsub: have defines\n") ;
	            debugprintf("paramfile_entsub: 1 varsub_expandbuf()\n") ;
#endif

	            f_exptry = TRUE ;
	            rs = varsub_expandbuf(&op->d,&b,pep->oval,pep->olen) ;
	            vl = rs ;

#if	CF_DEBUGS
	            debugprintf("paramfile_entsub: "
	                "1 varsub_expand() rs=%d\n",
	                rs) ;
#endif

	        } /* end if */

#if	CF_DEBUGS
	        debugprintf("paramfile_entsub: try 1 rs=%d\n",rs) ;
#endif

	        if ((! f_exptry) || (rs == SR_NOTFOUND)) {

#if	CF_DEBUGS
	            debugprintf("paramfile_entsub: trying 2\n") ;
#endif

	            rs = SR_OK ;
	            if (! op->f.envload)
	                rs = paramfile_envload(op) ;

#if	CF_DEBUGS
	            debugprintf("paramfile_entsub: "
	                "2 _envload() rs=%d\n",
	                rs) ;
#endif

	            if (rs >= 0) {
	                f_exptry = TRUE ;
	                rs = varsub_expandbuf(&op->e,&b,pep->oval,pep->olen) ;
	                vl = rs ;
	            }

#if	CF_DEBUGS
	            debugprintf("paramfile_entsub: "
	                "2 varsub_expandbuf() rs=%d\n",
	                rs) ;
#endif

	        } /* end if (second try) */

	        if ((rs >= 0) && (! f_exptry)) rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	        debugprintf("paramfile_entsub: store or not rs=%d vl=%u\n",
	            rs,vl) ;
#endif

	        if (rs >= 0) {

	            if ((rs = buffer_get(&b,&vp)) >= 0) {
	                int	f ;

	                f = (vl != pep->olen) ;
	                f = f || (strncmp(vp,pep->oval,vl) != 0) ;
	                if (f) {
	                    char	*bp ;

	                    if ((rs = uc_malloc((vl + 1),&bp)) >= 0) {
	                        strwcpy(bp,vp,vl) ;
	                        pep->value = bp ;
	                        pep->vlen = vl ;
	                        v = bp ;
	                    }

	                } else {
	                    pep->value = pep->oval ;
	                    pep->vlen = pep->olen ;
	                    v = pep->value ;
	                }

	            } /* end if (buffer_get) */

	        } else
	            v = pep->oval ;

	        rs1 = buffer_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (buffer) */

	} else {
	    v = pep->value ;
	}

	} /* end if (non-null) */

	if ((rs >= 0) && (vpp != NULL)) {
	    *vpp = v ;
	}

#if	CF_DEBUGS
	debugprintf("paramfile_entsub: ret rs=%d vl=%u v=>%s<\n",rs,vl,v) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (paramfile_entsub) */


/* free up all of the files in this PARAMFILE list */
static int paramfile_filefins(PARAMFILE *op)
{
	PARAMFILE_FILE	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;
	    rs1 = file_finish(fep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (paramfile_filefins) */


/* free up ALL of the entries in this PARAMFILE list */
static int paramfile_entfins(PARAMFILE *op)
{
	PARAMFILE_ENT	*pep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->entries,i,&pep) >= 0 ; i += 1) {
	    if (pep == NULL) continue ;
	    rs1 = entry_finish(pep) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (paramfile_entfins) */


/* free up all of the entries in this PARAMFILE list associated w/ a file */
static int paramfile_filedump(PARAMFILE *op,int fi)
{
	PARAMFILE_ENT	*pep ;
	VECOBJ		*slp = &op->entries ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("paramfile_filedump: fi=%d\n",fi) ;
#endif

	for (i = 0 ; vecobj_get(slp,i,&pep) >= 0 ; i += 1) {
	    if (pep == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("paramfile_filedump: i=%d fi=%d\n",i,pep->fi) ;
#endif

	    if ((pep->fi == fi) || (fi < 0)) {

#if	CF_DEBUGS
	        debugprintf("paramfile_filedump: key=%s\n",pep->key) ;
#endif

	        rs1 = entry_finish(pep) ;
	        if (rs >= 0) rs = rs1 ;

	        c += 1 ;
	        rs1 = vecobj_del(slp,i--) ;
	        if (rs >= 0) rs = rs1 ;

	    } /* end if (got one) */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("paramfile_filedump: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (paramfile_filedump) */


static int paramfile_filedel(PARAMFILE *op,int fi)
{
	PARAMFILE_FILE	*fep ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        rs1 = file_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_del(&op->files,fi) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end if (get) */

	return rs ;
}
/* end subroutine (paramfile_filedel) */


static int paramfile_envbegin(PARAMFILE *op)
{
	int		rs = SR_OK ;

	if (! op->f.envinit) {
	    const int	opts = 0 ;
	    rs = varsub_start(&op->e,opts) ;
	    op->f.envinit = (rs >= 0) ;
	} /* end if */

	return rs ;
}
/* end subroutine (paramfile_envbegin) */


static int paramfile_envend(PARAMFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.envinit) {
	    op->f.envinit = FALSE ;
	    rs1 = varsub_finish(&op->e) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (paramfile_envend) */


static int paramfile_envload(PARAMFILE *op)
{
	int		rs = SR_OK ;

	if ((op->envv != NULL) && (! op->f.envload)) {
	    op->f.envload = TRUE ;
	    if (! op->f.envinit) {
	        rs = paramfile_envbegin(op) ;
	    }
	    if (rs >= 0) {
	        rs = varsub_addva(&op->e,op->envv) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutines (paramfile_envload) */


static int paramfile_defbegin(PARAMFILE *op,VECSTR *dvp)
{
	int		rs = SR_OK ;

	if (! op->f.definit) {
	    const int	opts = VARSUB_OBADNOKEY ;
	    rs = varsub_start(&op->d,opts) ;
	    op->f.definit = (rs >= 0) ;
	}

	if ((rs >= 0) && (dvp != NULL)) {
	    rs = paramfile_defload(op,dvp) ;
	}

	return rs ;
}
/* end subroutines (paramfile_defbegin) */


static int paramfile_defend(PARAMFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.definit) {
	    op->f.definit = FALSE ;
	    rs1 = varsub_finish(&op->d) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (paramfile_defend) */


/* load up something */
static int paramfile_defload(PARAMFILE *op,VECSTR *dvp)
{
	int		rs = SR_OK ;

	if (dvp != NULL) {
	    int		i ;
	    const char	*tp, *cp ;
	    for (i = 0 ; vecstr_get(dvp,i,&cp) >= 0 ; i += 1) {
	        if (cp != NULL) {
	            if ((tp = strchr(cp,'=')) != NULL) {
	                rs = varsub_add(&op->d,cp,(tp - cp),(tp + 1),-1) ;
	            } else {
	                rs = varsub_add(&op->d,cp,-1,NULL,0) ;
		    }
		}
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if */

	return rs ;
}
/* end subroutine (paramfile_defload) */


static int paramfile_entrels(PARAMFILE *op)
{
	PARAMFILE_ENT	*pep ;
	VECOBJ		*slp = &op->entries ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(slp,i,&pep) >= 0 ; i += 1) {
	    if (pep != NULL) {
	        rs1 = entry_release(pep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (paramfile_entrels) */


static int file_start(PARAMFILE_FILE *fep,cchar *fname)
{
	int		rs ;
	const char	*cp ;

	if (fname == NULL) return SR_FAULT ;

	memset(fep,0,sizeof(PARAMFILE_FILE)) ;

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    fep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(PARAMFILE_FILE *fep)
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


static int entry_start(pep,fi,kp,kl,vsp,olen)
PARAMFILE_ENT	*pep ;
int		fi ;
const char	kp[] ;
int		kl ;
VECSTR		*vsp ;
int		olen ;
{
	int		rs = SR_OK ;
	int		size ;
	int		i ;
	const char	*vp ;
	char		*mp ;

	if (pep == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	memset(pep,0,sizeof(PARAMFILE_ENT)) ;

	pep->fi = fi ;
	if (kl < 0) kl = strlen(kp) ;

	pep->klen = kl ;
	pep->olen = olen ;

#if	CF_DEBUGS
	debugprintf("paramfile/entry_start: kl=%d olen=%d\n",kl,olen) ;
#endif

	size = (kl + 1 + olen + 1) + 10 ;
	if ((rs = uc_malloc(size,&mp)) >= 0) {

	    pep->key = mp ;
	    mp = strwcpy(mp,kp,kl) + 1 ;

#if	CF_DEBUGS
	    debugprintf("paramfile/entry_start: key=>%s<\n",pep->key) ;
#endif

	    pep->oval = mp ;
	    *mp = '\0' ;
	    for (i = 0 ; vecstr_get(vsp,i,&vp) >= 0 ; i += 1) {
	        if (i > 0) mp[-1] = CH_FS ;
	        mp = strwcpy(mp,vp,-1) + 1 ;
	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("paramfile/entry_start: oval=>%s<\n",pep->oval) ;
#endif

	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("paramfile/entry_start: ret rs=%d size=%u\n",rs,size) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(PARAMFILE_ENT *pep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	pep->fi = -1 ;
	if (pep->value != NULL) {
	    if (pep->oval != pep->value) {
	        rs1 = uc_free(pep->value) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    pep->value = NULL ;
	}

	if (pep->key != NULL) {
	    rs1 = uc_free(pep->key) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->key = NULL ;
	}

	pep->oval = NULL ;
	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_release(PARAMFILE_ENT *pep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pep == NULL) return SR_FAULT ;

	if ((pep->oval != pep->value) && (pep->value != NULL)) {
	    rs1 = uc_free(pep->value) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->value = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_release) */


static int vcmpentry(PARAMFILE_ENT **e1pp,PARAMFILE_ENT **e2pp)
{
	PARAMFILE_ENT	*e1p = *e1pp ;
	PARAMFILE_ENT	*e2p = *e2pp ;
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            if ((rc = (e1p->key[0] - e2p->key[0])) == 0) {
	                rc = strcmp(e1p->key,e2p->key) ;
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmpentry) */


