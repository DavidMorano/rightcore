/* devpermfile */

/* read in a file of parameters */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFIELD	1		/* extra debug print-outs */


/* revision history:

	= 2000-02-05, David A­D­ Morano
        Some code for this subroutine was taken from something that did
        something similar to what we are doing here. The rest was originally
        written for LevoSim.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object reads in the parameter file and makes the parameter pairs
        available thought a key search.


*******************************************************************************/


#define	DEVPERMFILE_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<regexpr.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecobj.h>
#include	<storebuf.h>
#include	<fsdir.h>
#include	<char.h>
#include	<buffer.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"devpermfile.h"


/* local defines */

#define	DEVPERMFILE_ICHECKTIME	2	/* file check interval (seconds) */
#define	DEVPERMFILE_ICHANGETIME	2	/* wait change interval (seconds) */

#define	DEVPERMFILE_KEY		struct devpermfile_k
#define	DEVPERMFILE_IE		struct devpermfile_ie

#define	DEVPERMFILE_KA		sizeof(mode_t)
#define	DEVPERMFILE_BO(v)		\
	((DEVPERMFILE_KA - ((v) % DEVPERMFILE_KA)) % DEVPERMFILE_KA)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	getpwd(char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	cfnumi(const char *,int,int *) ;
extern int	sfbasename(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct devpermfile_k {
	const char	*console ;
	int		conlen ;
	int		count ;
} ;

struct devpermfile_ie {
	const char	*dev ;		/* device string */
	mode_t		dmode ;
	int		ci ;		/* console index */
	int		devlen ;	/* device length */
} ;


/* forward references */

static int	devpermfile_finishkeys(DEVPERMFILE *) ;
static int	devpermfile_finishentries(DEVPERMFILE *) ;
static int	devpermfile_fileparse(DEVPERMFILE *,const char *) ;
static int	devpermfile_fileparseline(DEVPERMFILE *,const char *,int) ;
static int	devpermfile_keyadd(DEVPERMFILE *,const char *,int) ;
static int	devpermfile_keydel(DEVPERMFILE *,int) ;
static int	devpermfile_keyincr(DEVPERMFILE *,int) ;
static int	devpermfile_keydecr(DEVPERMFILE *,int) ;
static int	devpermfile_entexpand(DEVPERMFILE *,int,mode_t,
			const char *,int) ;
static int	devpermfile_ententer(DEVPERMFILE *,int,mode_t,
			const char *,int) ;
static int	devpermfile_entdir(DEVPERMFILE *,int,mode_t,
			const char *,int) ;
static int	devpermfile_entadd(DEVPERMFILE *,DEVPERMFILE_IE *) ;
static int	devpermfile_keymat(DEVPERMFILE *,DEVPERMFILE_IE *,
			const char *) ;

static int	key_start(DEVPERMFILE_KEY *,const char *,int) ;
static int	key_incr(DEVPERMFILE_KEY *) ;
static int	key_decr(DEVPERMFILE_KEY *) ;
static int	key_count(DEVPERMFILE_KEY *) ;
static int	key_mat(DEVPERMFILE_KEY *,const char *,int) ;
static int	key_finish(DEVPERMFILE_KEY *) ;
static int	key_fake(DEVPERMFILE_KEY *,const char *,int) ;

static int	ientry_start(DEVPERMFILE_IE *,int,mode_t,const char *,int) ;
static int	ientry_ci(DEVPERMFILE_IE *) ;
static int	ientry_finish(DEVPERMFILE_IE *) ;

static int	entry_load(DEVPERMFILE_ENT *,char *,int,DEVPERMFILE_IE *) ;

static int	mkcatfile(char *,int,const char *) ;
static int	mkdirfile(char *,const char *,int,const char *,int) ;

static int	vkeymat() ;

#ifdef	COMMENT
static int	vcmpentry() ;
#endif

static mode_t	modeparse(const char *,int) ;


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
	0x09, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int devpermfile_open(op,fname)
DEVPERMFILE	*op ;
const char	fname[] ;
{
	int	rs = SR_OK ;
	int	size ;
	int	opts ;

	const char	*cp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(DEVPERMFILE)) ;
	op->intcheck = DEVPERMFILE_ICHECKTIME ;
	op->intchange = DEVPERMFILE_ICHANGETIME ;
	op->ti_check = time(NULL) ;

	{
	    int		fl = -1 ;
	    char	tmpfname[MAXPATHLEN + 1] ;
	    if (fname[0] != '/') {
		int	pwdlen ;
		if ((rs = getpwd(tmpfname,MAXPATHLEN)) >= 0) {
		    pwdlen = rs ;
		    rs = mkcatfile(tmpfname,pwdlen,fname) ;
		    fl = rs ;
		    fname = tmpfname ;
		}
	    }
	    if (rs >= 0) {
	        rs = uc_mallocstrw(fname,fl,&cp) ;
	        op->fname = cp ;
	    }
	}
	if (rs < 0)
	    goto bad0 ;

/* initialize */

	opts = VECOBJ_OSTATIONARY ;
	size = sizeof(DEVPERMFILE_KEY) ;
	rs = vecobj_start(&op->keys,size,10,opts) ;
	if (rs < 0)
	    goto bad1 ;

/* keep this not-sorted so that the original order is maintained */

	opts = 0 ;
	size = sizeof(DEVPERMFILE_IE) ;
	rs = vecobj_start(&op->entries,size,10,opts) ;
	if (rs < 0)
	    goto bad2 ;

#if	CF_DEBUGS
	debugprintf("devpermfile_open: about to parse, file=%s\n",fname) ;
#endif

	rs = devpermfile_fileparse(op,fname) ;
	    if (rs < 0)
	        goto bad4 ;

	op->magic = DEVPERMFILE_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("devpermfile_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* handle bad things */
bad4:
	devpermfile_finishentries(op) ;

bad3:
	vecobj_finish(&op->entries) ;

bad2:
	vecobj_finish(&op->keys) ;

bad1:
	if (op->fname != NULL) {
	    uc_free(op->fname) ;
	    op->fname = NULL ;
	}

bad0:
	op->magic = 0 ;
	goto ret0 ;
}
/* end subroutine (devpermfile_open) */


/* free up the resources occupied by a DEVPERMFILE list object */
int devpermfile_close(op)
DEVPERMFILE	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DEVPERMFILE_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("devpermfile_close: ent\n") ;
#endif

/* secondary items */

	rs1 = devpermfile_finishentries(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = devpermfile_finishkeys(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->keys) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (devpermfile_close) */


/* cursor manipulations */
int devpermfile_curbegin(op,cp)
DEVPERMFILE	*op ;
DEVPERMFILE_CUR	*cp ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DEVPERMFILE_MAGIC)
	    return SR_NOTOPEN ;

	op->ccount += 1 ;
	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (devpermfile_curbegin) */


int devpermfile_curend(op,cp)
DEVPERMFILE	*op ;
DEVPERMFILE_CUR	*cp ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DEVPERMFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (op->ccount > 0) op->ccount -= 1 ;
	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (devpermfile_curend) */


/* search the parameters for a match */
int devpermfile_fetch(op,key,curp,ep,ebuf,elen)
DEVPERMFILE	*op ;
const char	key[] ;
DEVPERMFILE_CUR	*curp ;
DEVPERMFILE_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	DEVPERMFILE_IE		*iep ;

	VECOBJ	*elp ;

	int	rs ;
	int	i, j ;
	int	el = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DEVPERMFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (key == NULL)
	    return SR_FAULT ;

	if (key[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("devpermfile_fetch: ent key=%s\n",key) ;
#endif

	if (curp == NULL)
	    return SR_FAULT ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (ebuf == NULL)
	    return SR_FAULT ;

/* loop forward */

	elp = &op->entries ;
	    j = (curp->i < 0) ? 0 : (curp->i + 1) ;
	    for (i = j ; (rs = vecobj_get(elp,i,&iep)) >= 0 ; i += 1) {
	        if (iep == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("devpermfile_fetch: loop i=%d\n",i) ;
#endif

	        if (devpermfile_keymat(op,iep,key))
		    break ;

	    } /* end for (looping through entries) */

	if (rs >= 0) {
	    rs = entry_load(ep,ebuf,elen,iep) ;
	    if (rs >= 0)
	        curp->i = i ;
	}

#if	CF_DEBUGS
	debugprintf("devpermfile_fetch: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (devpermfile_fetch) */


/* enumerate the entries */
int devpermfile_enum(op,curp,ep,ebuf,elen)
DEVPERMFILE	*op ;
DEVPERMFILE_CUR	*curp ;
DEVPERMFILE_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	DEVPERMFILE_IE	*iep ;

	VECOBJ	*elp ;

	int	rs ;
	int	i, j ;
	int	el = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DEVPERMFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	if ((ep == NULL) || (ebuf == NULL))
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("devpermfile_enum: ent curi=%d\n",curp->i) ;
#endif

	elp = &op->entries ;
	i = (curp->i < 0) ? 0 : (curp->i + 1) ;
	for (j = i ; (rs = vecobj_get(elp,j,&iep)) >= 0 ; j += 1) {

	    if (iep != NULL)
	        break ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("devpermfile_enum: vecobj_get() j=%d rs=%d\n",j,rs) ;
#endif

	if ((rs >= 0) && (iep != NULL)) {
	    rs = entry_load(ep,ebuf,elen,iep) ;
	    el = rs ;
	    if (rs >= 0)
	        curp->i = j ;
	}

#if	CF_DEBUGS
	debugprintf("devpermfile_enum: ret rs=%d el=%u\n",rs,el) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (devpermfile_enum) */


int devpermfile_checkint(op,intcheck)
DEVPERMFILE	*op ;
int		intcheck ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != DEVPERMFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (intcheck < 1) intcheck = 1 ;
	op->intcheck = intcheck ;
	return SR_OK ;
}
/* end subroutine (devpermfile_checkint) */


/* check if the parameter file has changed */
int devpermfile_check(op,daytime)
DEVPERMFILE	*op ;
time_t		daytime ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		f_changed = FALSE ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DEVPERMFILE_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0)
	    daytime = time(NULL) ;

/* should we even check? */

	if ((daytime - op->ti_check) <= op->intcheck)
	    goto ret0 ;

	op->ti_check = daytime ;

/* check the files */

	rs = uc_stat(op->fname,&sb) ;
	if (rs < 0)
	    goto bad1 ;

	f = f || (sb.st_mtime > op->ti_mod) ;
	f = f || (sb.st_size != op->fsize) ;
	if (f && ((daytime - sb.st_mtime) >= op->intchange)) {

	    op->ti_mod = sb.st_mtime ;
	    op->fsize = sb.st_size ;
	    f_changed = TRUE ;
	    devpermfile_finishentries(op) ;

	    rs = devpermfile_fileparse(op,op->fname) ;

	} /* end if */

bad1:
ret0:

#if	CF_DEBUGS
	debugprintf("devpermfile_check: ret rs=%d changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (devpermfile_check) */


/* private subroutines */


/* parse a parameter file */
static int devpermfile_fileparse(op,fname)
DEVPERMFILE	*op ;
const char	fname[] ;
{
	struct ustat	sb ;

	bfile		loadfile, *lfp = &loadfile ;

	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	len ;
	int	cl ;
	int	c = 0 ;

	const char	*cp ;

	char	lbuf[LINEBUFLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	rs = bopen(lfp,op->fname,"r",0664) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("devpermfile_fileparse: bopen() rs=%d\n",rs) ;
#endif

	rs = bcontrol(lfp,BC_STAT,&sb) ;
	if (rs < 0)
	    goto ret1 ;

	op->ti_mod = sb.st_mtime ;
	op->fsize = sb.st_size ;

#if	CF_DEBUGS
	debugprintf("devpermfile_fileparse: start processing\n") ;
#endif

	while ((rs = breadlines(lfp,lbuf,llen,NULL)) > 0) {

	    len = rs ;
	    if (len == 1) continue ;	/* blank line */

	    if (lbuf[len - 1] != '\n') {
	        int	ch ;
	        while ((ch = bgetc(lfp)) >= 0) {
	            if (ch == '\n')
	                break ;
	        }
	        continue ;
	    }

	    cp = lbuf ;
	    cl = len ;
	    lbuf[--cl] = '\0' ;
	    while (CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        cl -= 1 ;
	    }

	    if ((*cp == '\0') || (*cp == '#')) continue ;

#if	CF_DEBUGS && CF_DEBUGSFIELD
	    debugprintf("devpermfile_fileparse: line> %t\n",
	        cp,cl) ;
#endif

	    rs = devpermfile_fileparseline(op,cp,cl) ;
	    c += rs ;

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	if (rs < 0) devpermfile_finishentries(op) ;

ret1:
	bclose(lfp) ;

ret0:

#if	CF_DEBUGS
	debugprintf("devpermfile_fileparse: ret rs=%d added=%d\n",
	    rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (devpermfile_fileparse) */


static int devpermfile_fileparseline(op,lp,ll)
DEVPERMFILE	*op ;
const char	*lp ;
int		ll ;
{
	FIELD	fsb ;

	mode_t	dmode ;

	int	rs ;
	int	fl ;
	int	kl, ml ;
	int	ci = -1 ;
	int	c = 0 ;
	int	f_key = FALSE ;
	int	f_mode = FALSE ;

	const char	*fp ;
	const char	*kp, *mp ;


	rs = field_start(&fsb,lp,ll) ;
	if (rs < 0)
	    goto ret0 ;

/* get the "console" field */

	fl = field_get(&fsb,kterms,&fp) ;
	if (fl <= 0)
	    goto ret1 ;

#if	CF_DEBUGS && CF_DEBUGSFIELD
	debugprintf("devpermfile_fileparseline: key=>%t<\n",
	    fp,fl) ;
#endif

	kp = fp ;
	kl = fl ;

/* get the "mode" field */

	fl = field_get(&fsb,kterms,&fp) ;
	if (fl <= 0)
	    goto ret1 ;

	mp = fp ;
	ml = fl ;

/* get "devices" */

	    while ((rs >= 0) && 
	        ((fl = field_get(&fsb,aterms,&fp)) >= 0)) {

#if	CF_DEBUGS && CF_DEBUGSFIELD
	        debugprintf("devpermfile_fileparseline: "
	            "field value=>%t<\n",
	            fp,fl) ;
#endif

	        if (fl > 0) {

			if (! f_key) {
			f_key = TRUE ;
			rs = devpermfile_keyadd(op,kp,kl) ;
			ci = rs ;
			if (rs == SR_OVERFLOW) break ;
			}

		if ((rs >= 0) && (! f_mode)) {
			f_mode = TRUE ;
			dmode = modeparse(mp,ml) ; /* cannot fail */
		}

		if (rs >= 0) {
			rs = devpermfile_entexpand(op,ci,dmode,fp,fl) ;
			c += rs ;
		} /* end if */

	        } /* end if (non-empty device field) */

	        if (fsb.term == '#') break ;
		if (rs < 0) break ;
	    } /* end while (fields) */

	if (f_key && (c == 0) && (ci >= 0))
	    devpermfile_keydel(op,ci) ;

ret1:
	field_finish(&fsb) ;

ret0:

#if	CF_DEBUGS && CF_DEBUGSFIELD
	debugprintf("devpermfile_fileparseline: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (devpermfile_fileparseline) */


/* add an entry to something */
static int devpermfile_entadd(op,iep)
DEVPERMFILE	*op ;
DEVPERMFILE_IE	*iep ;
{
	int	rs ;


	rs = vecobj_add(&op->entries,iep) ;

#if	CF_DEBUGS 
	debugprintf("devpermfile_entadd: vecobj_add() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (devpermfile_entadd) */


/* free up ALL of the keys */
static int devpermfile_finishkeys(op)
DEVPERMFILE	*op ;
{
	DEVPERMFILE_KEY	*kep ;

	VECOBJ	*klp = &op->keys ;

	int	rs = SR_OK ;
	int	i ;


	for (i = 0 ; vecobj_get(klp,i,&kep) >= 0 ; i += 1) {
	    if (kep == NULL) continue ;
	    key_finish(kep) ;
	    vecobj_del(klp,i--) ;
	} /* end for */

	return rs ;
}
/* end subroutine (devpermfile_finishkeys) */


/* free up ALL of the entries in this DEVPERMFILE list */
static int devpermfile_finishentries(op)
DEVPERMFILE	*op ;
{
	DEVPERMFILE_IE	*iep ;

	VECOBJ	*elp = &op->entries ;

	int	rs = SR_OK ;
	int	i ;


	for (i = 0 ; vecobj_get(elp,i,&iep) >= 0 ; i += 1) {
	    if (iep == NULL) continue ;
	    devpermfile_keydecr(op,iep->ci) ;
	    ientry_finish(iep) ;
	    vecobj_del(elp,i--) ;
	} /* end for */

	return rs ;
}
/* end subroutine (devpermfile_finishentries) */


static int devpermfile_keyadd(op,kp,kl)
DEVPERMFILE	*op ;
const char	*kp ;
int		kl ;
{
	DEVPERMFILE_KEY	k ;

	VECOBJ	*klp = &op->keys ;

	int	rs ;
	int	rs1 ;
	int	cl ;
	int	ci = 0 ;

	char	tmpfname[MAXPATHLEN + 1] ;


	rs = pathclean(tmpfname,kp,kl) ;
	cl = rs ;
	if (rs < 0)
	    goto ret0 ;

	key_fake(&k,tmpfname,cl) ;

	rs1 = vecobj_search(klp,&k,vkeymat,NULL) ;
	ci = rs1 ;
	if (rs1 == SR_NOTFOUND) {
	    rs = key_start(&k,tmpfname,cl) ;
	    if (rs >= 0) {
	        rs = vecobj_add(&op->keys,&k) ;
	        ci = rs ;
	        if (rs < 0) key_finish(&k) ;
	    }
	}

ret0:
	return (rs >= 0) ? ci : rs ;
}
/* end subroutine (devpermfile_keyadd) */


static int devpermfile_keydel(op,ci)
DEVPERMFILE	*op ;
int		ci ;
{
	DEVPERMFILE_KEY	*kep ;

	VECOBJ	*klp = &op->keys ;

	int	rs1 ;


	rs1 = vecobj_get(klp,ci,&kep) ;
	if (rs1 >= 0) {
	    int	c = key_count(kep) ;
	    if (c == 0) {
	        key_finish(kep) ;
	        vecobj_del(klp,ci) ;
	    }
	}

	return SR_OK ;
}
/* end subroutine (devpermfile_keydel) */


static int devpermfile_keyincr(op,ci)
DEVPERMFILE	*op ;
int		ci ;
{
	DEVPERMFILE_KEY	*kep ;

	VECOBJ	*klp = &op->keys ;

	int	rs ;


	rs = vecobj_get(klp,ci,&kep) ;
	if (rs >= 0)
	    key_incr(kep) ;

	return rs ;
}
/* end subroutine (devpermfile_keyincr) */


static int devpermfile_keydecr(op,ci)
DEVPERMFILE	*op ;
int		ci ;
{
	DEVPERMFILE_KEY	*kep ;

	VECOBJ	*klp = &op->keys ;

	int	rs ;


	rs = vecobj_get(klp,ci,&kep) ;
	if (rs >= 0) {
	    int	c = key_decr(kep) ;
	    if (c == 0) {
		key_finish(kep) ;
		vecobj_del(klp,ci) ;
	    }
	}

	return rs ;
}
/* end subroutine (devpermfile_keydecr) */


static int devpermfile_entexpand(op,ci,dmode,dp,dl)
DEVPERMFILE	*op ;
int		ci ;
mode_t		dmode ;
const char	*dp ;
int		dl ;
{
	int	rs = SR_OK ;
	int	bl ;
	int	c = 0 ;

	const char	*bp ;


	if (dl < 0) dl = strlen(dp) ;

	bl = sfbasename(dp,dl,&bp) ;

	if (bl > 0) {
	    if ((bl == 1) && (bp[0] == '*')) {
	        rs = devpermfile_entdir(op,ci,dmode,dp,(bp-1-dp)) ;
	        c += rs ;
	    } else {
	        rs = devpermfile_ententer(op,ci,dmode,dp,dl) ;
	        c += rs ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (devpermfile_entexpand) */


static int devpermfile_ententer(op,ci,dmode,dp,dl)
DEVPERMFILE	*op ;
int		ci ;
mode_t		dmode ;
const char	*dp ;
int		dl ;
{
	DEVPERMFILE_IE	ie ;

	int	rs ;
	int	c = 0 ;


	rs = ientry_start(&ie,ci,dmode,dp,dl) ;
	if (rs >= 0) {
	    rs = devpermfile_keyincr(op,ci) ;
	    if (rs >= 0) {
		c += 1 ;
		rs = devpermfile_entadd(op,&ie) ;
		if (rs < 0)
	 	    devpermfile_keydecr(op,ci) ;
	    }
	    if (rs < 0)
		ientry_finish(&ie) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (devpermfile_ententer) */


static int devpermfile_entdir(op,ci,dmode,dp,dl)
DEVPERMFILE	*op ;
int		ci ;
mode_t		dmode ;
const char	*dp ;
int		dl ;
{
	NULSTR		ns ;

	FSDIR		d ;

	FSDIR_SLOT	ds ;

	int	rs ;
	int	rs1 ;
	int	c = 0 ;

	const char	*dname ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if ((rs = nulstr_start(&ns,dp,dl,&dname)) >= 0) {

	    if (fsdir_open(&d,dname) >= 0) {
	        int	dnl ;

	        while ((dnl = fsdir_read(&d,&ds)) > 0) {
		    const char *dnp = ds.name ;

		    if (dnp[0] == '.') continue ;
		    rs1 = mkdirfile(tmpfname,dp,dl,dnp,dnl) ;
		    if (rs1 >= 0) {
	                rs = devpermfile_ententer(op,ci,dmode,tmpfname,rs1) ;
	                c += rs ;
	 	    }

	        } /* end while (reading) */

	        fsdir_close(&d) ;
	    } /* end if (fsdir_open) */

	    nulstr_finish(&ns) ;
	} /* end if (nullstr) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (devpermfile_entdir) */


static int devpermfile_keymat(op,iep,key)
DEVPERMFILE	*op ;
DEVPERMFILE_IE	*iep ;
const char	key[] ;
{
	DEVPERMFILE_KEY	*kep ;

	int	rs1 ;
	int	conlen = strlen(key) ;
	int	ci = iep->ci ;
	int	f = FALSE ;


	ci = ientry_ci(iep) ;

	rs1 = vecobj_get(&op->keys,ci,&kep) ;
	if (rs1 >= 0)
	    f = key_mat(kep,key,conlen) ;

	return f ;
}
/* end subroutine (devpermfile_keymat) */


static int key_start(kep,console,conlen)
DEVPERMFILE_KEY	*kep ;
const char	*console ;
int		conlen ;
{
	int	rs ;

	const char	*cp ;


	if (kep == NULL)
	    return SR_FAULT ;

	if (console == NULL)
	    return SR_FAULT ;

	if (console[0] == '\0')
	    return SR_INVALID ;

	if (conlen < 0) conlen = strlen(console) ;

	memset(kep,0,sizeof(DEVPERMFILE_KEY)) ;
	rs = uc_mallocstrw(console,conlen,&cp) ;
	if (rs >= 0) {
	    kep->console = cp ;
	    kep->conlen = conlen ;
	}

ret0:
	return rs ;
}
/* end subroutine (key_start) */


static int key_incr(kep)
DEVPERMFILE_KEY	*kep ;
{


	if (kep == NULL)
	    return SR_FAULT ;

	kep->count += 1 ;
	return kep->count ;
}
/* end subroutine (key_incr) */


static int key_decr(kep)
DEVPERMFILE_KEY	*kep ;
{


	if (kep == NULL)
	    return SR_FAULT ;

	if (kep->count > 0) kep->count -= 1 ;
	return kep->count ;
}
/* end subroutine (key_decr) */


static int key_count(kep)
DEVPERMFILE_KEY	*kep ;
{


	if (kep == NULL)
	    return SR_FAULT ;

	return kep->count ;
}
/* end subroutine (key_count) */


static int key_mat(kep,key,keylen)
DEVPERMFILE_KEY	*kep ;
const char	key[] ;
int		keylen ;
{
	int	cl = kep->conlen ;
	int	f ;

	const char	*cp = kep->console ;


	if (keylen < 0) keylen = strlen(key) ;

	f = (keylen == cl) ;
	f = f && (strncmp(cp,key,cl) == 0) ;
	return f ;
}
/* end subroutine (key_mat) */


static int key_finish(kep)
DEVPERMFILE_KEY	*kep ;
{
	int	rs = SR_OK ;


	if (kep == NULL)
	    return SR_FAULT ;

	if (kep->console != NULL) {
	    uc_free(kep->console) ;
	    kep->console = NULL ;
	}

	return rs ;
}
/* end subroutine (key_finish) */


static int key_fake(kep,key,keylen)
DEVPERMFILE_KEY	*kep ;
const char	key[] ;
int		keylen ;
{


	if (keylen < 0) keylen = strlen(key) ;

	kep->console = key ;
	kep->conlen = keylen ;
	return keylen ;
}
/* end subroutine (key_fake) */


static int ientry_start(iep,ci,m,dp,dl)
DEVPERMFILE_IE	*iep ;
int		ci ;
mode_t		m ;
const char	dp[] ;
int		dl ;
{
	int	rs ;

	const char	*cp ;


	if (iep == NULL)
	    return SR_FAULT ;

	if (ci < 0)
	    return SR_INVALID ;

	if (dp == NULL)
	    return SR_FAULT ;

	memset(iep,0,sizeof(DEVPERMFILE_IE)) ;
	iep->ci = ci ;
	iep->dmode = m ;
	if (dl < 0) dl = strlen(dp) ;

	rs = uc_mallocstrw(dp,dl,&cp) ;
	if (rs < 0)
	    goto ret0 ;

	iep->dev = cp ;
	iep->devlen = dl ;

ret0:

#if	CF_DEBUGS
	debugprintf("devpermfile/ientry_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ientry_start) */


static int ientry_finish(iep)
DEVPERMFILE_IE	*iep ;
{


	if (iep == NULL)
	    return SR_FAULT ;

	if (iep->dev != NULL) {
	    uc_free(iep->dev) ;
	    iep->dev = NULL ;
	}

	iep->devlen = 0 ;
	return SR_OK ;
}
/* end subroutine (ientry_finish) */


static int ientry_ci(iep)
DEVPERMFILE_IE	*iep ;
{


	if (iep == NULL)
	    return SR_FAULT ;

	return iep->ci ;
}
/* end subroutine (ientry_ci) */


static int entry_load(ep,ebuf,elen,iep)
DEVPERMFILE_ENT	*ep ;
char			ebuf[] ;
int			elen ;
DEVPERMFILE_IE		*iep ;
{
	int	rs = SR_OK ;
	int	bo ;
	int	ilen ;
	int	el = 0 ;

	char	*bp ;


	if (ep == NULL)
	    return SR_FAULT ;

	if (ebuf == NULL)
	    return SR_FAULT ;

	if (elen <= 0)
	    return SR_OVERFLOW ;

	if (iep == NULL)
	    return SR_FAULT ;

	memset(ep,0,sizeof(DEVPERMFILE_ENT)) ;
	ep->devmode = iep->dmode ;
	ep->devlen = iep->devlen ;

	bp = ebuf ;
	bo = DEVPERMFILE_BO((ulong) ebuf) ;
	bp += bo ;
	elen -= bo ;

#if	CF_DEBUGS
	    debugprintf("entry_load: bo=%u\n",bo) ;
#endif

	if (rs >= 0) {
	    ilen = (iep->devlen+1) ;
	    if (ilen < elen) {
		ep->dev = bp ;
	        bp = strwcpy(bp,iep->dev,iep->devlen) + 1 ;
		elen -= ilen ;
	    } else
	        rs = SR_OVERFLOW ;
	}

	el = (bp - ebuf) ;

#if	CF_DEBUGS
	debugprintf("entry_load: ret rs=%d el=%u\n",rs,el) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (entry_load) */


static int mkcatfile(char *buf,int i,const char *fname)
{
	const int	blen = MAXPATHLEN ;

	int	rs = SR_OK ;


	if ((rs >= 0) && (i > 0) && (buf[i-1] != '/')) {
	    rs = storebuf_char(buf,blen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(buf,blen,i,fname,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkfile) */


static int mkdirfile(buf,dp,dl,dnp,dnl)
char		buf[] ;
const char	*dp ;
int		dl ;
const char	*dnp ;
int		dnl ;
{
	const int	blen = MAXPATHLEN ;

	int	rs = SR_OK ;
	int	i = 0 ;


	if (rs >= 0) {
	    rs = storebuf_strw(buf,blen,i,dp,dl) ;
	    i += rs ;
	}

	if ((rs >= 0) && (i > 0) && (buf[i-1] != '/')) {
	    rs = storebuf_char(buf,blen,i,'/') ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(buf,blen,i,dnp,dnl) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkdirfile) */


static int vkeymat(e1pp,e2pp)
DEVPERMFILE_KEY	**e1pp, **e2pp ;
{
	DEVPERMFILE_KEY	*e1p = *e1pp ;
	DEVPERMFILE_KEY	*e2p = *e2pp ;

	int	rc ;


	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	rc = (e1p->console[0] - e2p->console[0]) ;
	if (rc == 0)
	    rc = strcmp(e1p->console,e2p->console) ;

	return rc ;
}
/* end subroutine (vkeymat) */


#ifdef	COMMENT

static int vcmpentry(e1pp,e2pp)
DEVPERMFILE_ENT	**e1pp, **e2pp ;
{
	DEVPERMFILE_ENT	*e1p = *e1pp ;
	DEVPERMFILE_ENT	*e2p = *e2pp ;

	int	rc ;


	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	rc = (e1p->console[0] - e2p->console[0]) ;
	if (rc == 0)
	    rc = strcmp(e1p->key,e2p->key) ;

	return rc ;
}
/* end subroutine (vcmpentry) */

#endif /* COMMENT */


static mode_t modeparse(const char *mp,int ml)
{
	mode_t	m = 0600 ;


	if (ml < 0) ml = strlen(mp) ;

	if (ml > 0) {
		int	rs1 ;
		int	v ;
		rs1 = cfnumi(mp,ml,&v) ;
		if (rs1 >= 0) {
			if (v > 0777) v = 0777 ;
			m = v ;
		}
	}

	return m ;
}
/* end subroutine (modeparse) */



