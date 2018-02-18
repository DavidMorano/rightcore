/* kbdinfo */

/* keyboard-information database access */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2009-01-20, David A­D­ Morano
	This was written from sratch.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object provides access to a keyboard-information database. This
        information is used in creating the name of a key pressed on a
        particular keyboard.

        Notes: There is much to be desired with the implementation of this
        object. We may have to revisit this object if we ever need to get some
        enhanced function out of function keys.


*******************************************************************************/


#define	KBDINFO_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<bfile.h>
#include	<field.h>
#include	<localmisc.h>

#include	"keysymer.h"
#include	"kbdinfo.h"


/* local defines */

#define	KBDINFO_KEYNAMELEN	60

#undef	ENTRYINFO
#define	ENTRYINFO		struct entryinfo


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfnumi(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasuc(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyblanks(char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strncpylc(char *,const char *,int) ;
extern char	*strncpyuc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strnrpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct entryinfo {
	const char	*fp ;
	int		fl ;
} ;


/* forward references */

static int kbdinfo_parse(KBDINFO *,const char *) ;
static int kbdinfo_parseline(KBDINFO *,const char *,int) ;
static int kbdinfo_process(KBDINFO *,ENTRYINFO *,int) ;
static int kbdinfo_store(KBDINFO *,int,const char *,int,ENTRYINFO *,int) ;
static int kbdinfo_kefins(KBDINFO *) ;

static int ke_start(KBDINFO_KE *,int,const char *,int,ENTRYINFO *,int) ;
static int ke_finish(KBDINFO_KE *) ;

static int vcmpfind(const void *,const void *) ;


/* local variables */

static const char	*keytypes[KBDINFO_TOVERLAST + 1] = {
	"reg",
	"esc",
	"csi",
	"dcs",
	"pf",
	"fkey",
	NULL
} ;

static const uchar	kterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int kbdinfo_open(KBDINFO *op,KEYSYMER *ksp,cchar *fname)
{
	int		rs = SR_OK ;
	int		size ;
	int		opts ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("kbdinfo_open: fname=%s\n",fname) ;
#endif

	memset(op,0,sizeof(KBDINFO)) ;
	op->ksp = ksp ;

	opts = VECOBJ_OSORTED ;
	size = sizeof(KBDINFO_KE) ;
	for (i = 0 ; (rs >= 0) && (i < KBDINFO_TOVERLAST) ; i += 1) {
	    rs = vecobj_start(&op->types[i],size,4,opts) ;
	} /* end for */
	if (rs >= 0) {
	    if ((rs = kbdinfo_parse(op,fname)) >= 0) {
		op->magic = KBDINFO_MAGIC ;
	    }
	    if (rs < 0) {
		kbdinfo_kefins(op) ;
		for (i = 0 ; i < KBDINFO_TOVERLAST ; i += 1) {
	    	    vecobj_finish(&op->types[i]) ;
		}
	    }
	} else {
	    int	j ;
	    for (j = 0 ; j < i ; j += 1) {
	        vecobj_finish(&op->types[j]) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("kbdinfo_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kbdinfo_open) */


int kbdinfo_close(KBDINFO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KBDINFO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("kbdinfo_close: ent\n") ;
#endif

	rs1 = kbdinfo_kefins(op) ;
	if (rs >= 0) rs = rs1 ;

	for (i = 0 ; i < KBDINFO_TOVERLAST ; i += 1) {
	    rs1 = vecobj_finish(&op->types[i]) ;
	    if (rs >= 0) rs = rs1 ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (kbdinfo_close) */


int kbdinfo_count(KBDINFO *op)
{
	int		rs = SR_OK ;
	int		i ;
	int		count = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KBDINFO_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; (rs >= 0) && (i < KBDINFO_TOVERLAST) ; i += 1) {
	    rs = vecobj_count(op->types + i) ;
	    count += rs ;
	} /* end for */

	return (rs >= 0) ? count : rs ;
}
/* end subroutine (kbdinfo_count) */


int kbdinfo_lookup(KBDINFO *op,char *ksbuf,int kslen,TERMCMD *cmdp)
{
	KBDINFO_KE	te, *ep ;
	const int	nps = 4 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ktype ;
	int		keynum = 0 ;
	short		params[4] ;

	if (op == NULL) return SR_FAULT ;
	if (cmdp == NULL) return SR_FAULT ;

	if (op->magic != KBDINFO_MAGIC) return SR_NOTOPEN ;

	ktype = (cmdp->name == '~') ? KBDINFO_TFKEY : cmdp->type ;

	memset(&te,0,sizeof(KBDINFO_KE)) ;
	te.type = ktype ;
	te.name = cmdp->name ;
	te.p = params ;
	if (cmdp->p[0] >= 0) {
	    const int	n = MIN(nps,TERMCMD_NP) ;
	    int	i ;
	    for (i = 0 ; i < n ; i += 1) {
	        te.p[i] = cmdp->p[i] ;
		if (cmdp->p[i] < 0) break ;
	    }
	    te.nparams = i ;
	}

#if	CF_DEBUGS
	debugprintf("kbdinfo_lookup: ktype=%d\n",te.type) ;
	debugprintf("kbdinfo_lookup: kname=%d\n",te.name) ;
	debugprintf("kbdinfo_lookup: nparams=%d\n",te.nparams) ;
	debugprintf("kbdinfo_lookup: p0=%hd\n",te.p[0]) ;
#endif

	rs = vecobj_search(&op->types[ktype],&te,vcmpfind,&ep) ;

#if	CF_DEBUGS
	debugprintf("kbdinfo_lookup: vecobj_search() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (ep != NULL)) {

#if	CF_DEBUGS
	    debugprintf("kbdinfo_lookup: keyname=%s\n",ep->keyname) ;
#endif

	    if (ep->keynum < 0) {
	        if (op->ksp != NULL) {
	            rs1 = keysymer_lookup(op->ksp,ep->keyname,-1) ;

#if	CF_DEBUGS
	            debugprintf("kbdinfo_lookup: keysymer_lookup() rs1=%d\n",
	                rs1) ;
#endif

	            if (rs1 >= 0) {
	                ep->keynum = rs1 ;
	                keynum = rs1 ;
	            }
	        }
	    } else
	        keynum = ep->keynum ;

	    if (ksbuf != NULL) {
		rs = sncpy1(ksbuf,kslen,ep->keyname) ;
	    }

	} else {

	    if (ksbuf != NULL) ksbuf[0] = '\0' ;

	} /* end if */

	return (rs >= 0) ? keynum : rs ;
}
/* end subroutine (kbdinfo_lookup) */


int kbdinfo_curbegin(KBDINFO *op,KBDINFO_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KBDINFO_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(KBDINFO_CUR)) ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (kbdinfo_curbegin) */


int kbdinfo_curend(KBDINFO *op,KBDINFO_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KBDINFO_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(KBDINFO_CUR)) ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (kbdinfo_curend) */


int kbdinfo_enum(KBDINFO *op,KBDINFO_CUR *curp,KBDINFO_KE **rpp)
{
	int		rs = SR_OK ;
	int		i, j ;
	int		nl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if (op->magic != KBDINFO_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(KBDINFO_CUR)) ;

	i = curp->i ;
	if (i < 0) {
	    i = 0 ;
	    j = 0 ;
	} else
	    j = (curp->j + 1) ;

	if (i < KBDINFO_TOVERLAST) {
	    rs = vecobj_get((op->types + i),j,rpp) ;
	    if ((rs == SR_NOTFOUND) && (i < KBDINFO_TOVERLAST)) {
	        i += 1 ;
	        j = 0 ;
	        rs = vecobj_get((op->types + i),j,rpp) ;
	    }
	}

	if ((rs >= 0) && (*rpp != NULL)) {
	    curp->i = i ;
	    curp->j = j ;
	    if ((*rpp)->keyname != NULL) {
	        nl = strlen((*rpp)->keyname) ;
	    }
	}

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (kbdinfo_enum) */


/* private subroutines */


static int kbdinfo_parse(KBDINFO *op,cchar *fname)
{
	bfile		dfile, *dfp = &dfile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("kbdinfo_parse: fname=%s\n",fname) ;
#endif

	if (fname == NULL) return SR_FAULT ;

	if ((rs = bopen(dfp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		sl ;
	    cchar	*sp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(dfp,lbuf,llen)) > 0) {
	        len = rs ;

		if (lbuf[len-1] == '\n') len -= 1 ;
		lbuf[len] = '\0' ;

		if ((sl = sfskipwhite(lbuf,len,&sp)) > 0) {
	            if (sp[0] != '#') {
	                if ((rs = kbdinfo_parseline(op,sp,sl)) > 0) {
	                    c += 1 ;
			}
		    }
	        }

	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = bclose(dfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (open-file) */

#if	CF_DEBUGS
	debugprintf("kbdinfo_parse: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (kbdinfo_parse) */


static int kbdinfo_parseline(KBDINFO *op,cchar *lp,int ll)
{
	ENTRYINFO	eis[20] ;
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("kbdinfo_parseline: l=>%t<\n",
		lp,strlinelen(lp,ll,40)) ;
#endif

	memset(eis,0,sizeof(ENTRYINFO)*nelem(eis)) ;

	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    const int	n = nelem(eis) ;
	    int		i = 0 ;
	    int		fl ;
	    const char	*fp ;

	    while ((i < n) && ((fl = field_get(&fsb,kterms,&fp)) > 0)) {

#if	CF_DEBUGS
	debugprintf("kbdinfo_parseline: i=%u f=>%t<\n",i,fp,fl) ;
#endif

	        eis[i].fp = fp ;
	        eis[i].fl = fl ;

		i += 1 ;
	        if (fsb.term == '#') break ;
	    } /* end for */

#if	CF_DEBUGS
	debugprintf("kbdinfo_parseline: mid i=%u\n",i) ;
#endif

	    if (i >= 3) {
	        rs = kbdinfo_process(op,eis,i) ;
	        c = rs ;
	    }

#if	CF_DEBUGS
	debugprintf("kbdinfo_parseline: _process() rs=%u\n",rs) ;
#endif

	    field_finish(&fsb) ;
	} /* end if (field) */

#if	CF_DEBUGS
	debugprintf("kbdinfo_parseline: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (kbdinfo_parseline) */


static int kbdinfo_process(KBDINFO *op,ENTRYINFO *eis,int n)
{
	int		rs = SR_OK ;
	int		ktl = eis[1].fl ;
	int		ktype ;
	int		c = 0 ;
	const char	*ktp = eis[1].fp ;

#if	CF_DEBUGS
	{
	    int	i ;
	    for (i = 0 ; i < n ; i += 1)
	    debugprintf("kbdinfo_process: eis[%u]=>%t<\n",
		i,eis[i].fp,eis[i].fl) ;
	}
#endif /* CF_DEBUGS */

	if ((ktype = matocasestr(keytypes,2,ktp,ktl)) >= 0) {
	    int		knl = eis[0].fl ;
	    cchar	*knp = eis[0].fp ;
	    c = 1 ;
	    rs = kbdinfo_store(op,ktype,knp,knl,eis,n) ;
	} /* end if (match) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (kbdinfo_process) */


static int kbdinfo_store(KBDINFO *op,int ktype,cchar *knp,int knl,
		ENTRYINFO *eis,int n)
{
	KBDINFO_KE	e ;
	int		rs = SR_OK ;
	char		keybuf[KBDINFO_KEYNAMELEN + 1] ;

	if (knl < 0) knl = strlen(knp) ;

#if	CF_DEBUGS
	debugprintf("kbdinfo_store: k=>%t<\n",knp,knl) ;
#endif /* CF_DEBUGS */

	if (hasuc(knp,knl)) {
	    const int	ml = MIN(knl,KBDINFO_KEYNAMELEN) ;
	    knl = strwcpylc(keybuf,knp,ml) - keybuf ;
	    knp = keybuf ;
	}

#if	CF_DEBUGS
	debugprintf("kbdinfo_store: ktype=%u k=%t\n",
		ktype,knp,knl) ;
#endif /* CF_DEBUGS */

	if ((rs = ke_start(&e,ktype,knp,knl,eis,n)) >= 0) {
	    rs = vecobj_add((op->types + ktype),&e) ;
	    if (rs < 0)
	        ke_finish(&e) ;
	}

	return rs ;
}
/* end subroutine (kbdinfo_store) */


static int kbdinfo_kefins(KBDINFO *op)
{
	KBDINFO_KE	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; i < KBDINFO_TOVERLAST ; i += 1) {
	    int	j ;
	    for (j = 0 ; vecobj_get(&op->types[i],j,&ep) >= 0 ; j += 1) {
	        if (ep != NULL) {
	            rs1 = ke_finish(ep) ;
	            if (rs >= 0) rs = rs1 ;
		}
	    } /* end for */
	} /* end for */

	return rs ;
}
/* end subroutine (kbdinfo_kefins) */


#ifdef	COMMENT
struct kbdinfo_e {
	const char	*a		/* the memory allocation */
	const char	*keyname ;	/* keysym-name */
	const char	*istr ;
	const char	*dstr ;
	short		*p ;		/* parameters */
	int		type ;		/* key type */
	int		name ;		/* key name */
	int		keynum ;	/* key number */
	int		np ;		/* number of paramters */
} ;
# keysym	type	final	inter	param(s)
F6              FKEY	-	-	17
F7              FKEY	-	-	18
#endif /* COMMENT */


static int ke_start(KBDINFO_KE *kep,int ktype,cchar *knp,int knl,
		ENTRYINFO *eis,int n)
{
	const int	oi = 4 ;
	int		rs ;
	int		nparams = 0 ;
	int		size = 0 ;
	char		*bp ;

	if (kep == NULL) return SR_FAULT ;
	if (knp == NULL) return SR_FAULT ;

	if (knp[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("kbdinfo/ke_start: k=%t n=%u\n",knp,knl,n) ;
#endif

	memset(kep,0,sizeof(KBDINFO_KE)) ;
	kep->type = ktype ;
	kep->keynum = -1 ;

	{
	    int	name = (eis[2].fp[0] & 0xff) ;
	    if (ktype == KBDINFO_TFKEY) name = '~' ;
	    kep->name = name ;
	}

	size += (strnlen(knp,knl) + 1) ;
	if (n > 3) {
	    size += (strnlen(eis[3].fp,eis[3].fl) + 1) ;
	} else {
	    size += 1 ;
	}

	if (n > oi) {
	    int	i ;
	    for (i = oi ; i < n ; i += 1) {
	        if ((eis[i].fl > 0) && (eis[i].fp[0] != '-')) {
		    nparams = (i+1-oi) ;
	        }
	    }
	    size += (nparams*sizeof(short)) ;
	}

#if	CF_DEBUGS
		debugprintf("kbdinfo/ke_start: nparams=%u size=%u\n",
		nparams,size) ;
#endif

	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    int		pi ;
	    int		fl ;
	    int		v ;
	    short	*pp ;
	    const char	*fp ;

	    kep->a = bp ;
	    kep->nparams = nparams ;
	    kep->p = (short *) bp ;
	    pp = (short *) bp ;
	    for (pi = 0 ; pi < nparams ; pi += 1) {
		v = 0 ;
		fl = eis[pi+oi].fl ;
		fp = eis[pi+oi].fp ;
		if (fl > 0) {
		    if ((fp != NULL) && (fp[0] != '-')) {
		        rs = cfnumi(fp,fl,&v) ;
		    }
		}
#if	CF_DEBUGS
		debugprintf("kbdinfo/ke_start: pi=%u v=%d\n",pi,v) ;
#endif
		pp[pi] = (v & SHORT_MAX) ;
		if (rs < 0) break ;
	    } /* end for */
	    bp += (nparams*sizeof(short)) ;

	    if (rs >= 0) {
	        kep->keyname = bp ;
		bp = (strwcpy(bp,knp,knl) + 1) ;
	    }

	    if (rs >= 0) {
	        kep->istr = bp ;
		fp = eis[3].fp ;
		fl = eis[3].fl ;
		if ((fl > 0) && (fp[0] != '-')) {
		    bp = (strwcpy(bp,fp,fl) + 1) ;
		} else {
		    *bp++ = '\0' ;
		}
	    }

	    if (rs < 0) {
		uc_free(kep->a) ;
		kep->a = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("kbdinfo/ke_start: ret rs=%d\n",rs) ;
	if ((rs >= 0) && (kep->nparams > 0)) {
	    int	pi ;
	    for (pi = 0 ; pi < kep->nparams ; pi += 1) {
		debugprintf("kbdinfo/ke_start: p[%u]=%hd\n",
			pi,kep->p[pi]) ;
	    }
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (ke_start) */


static int ke_finish(KBDINFO_KE *kep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (kep == NULL) return SR_FAULT ;

	if (kep->a != NULL) {
	    rs1 = uc_free(kep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    kep->a = NULL ;
	}

	return rs ;
}
/* end subroutine (ke_finish) */


static int vcmpfind(const void *v1pp,const void *v2pp)
{
	KBDINFO_KE	**e1pp = (KBDINFO_KE **) v1pp ;
	KBDINFO_KE	**e2pp = (KBDINFO_KE **) v2pp ;
	int		rc = 0 ;

	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            if ((rc = ((*e1pp)->name - (*e2pp)->name)) == 0) {
	                if (((*e1pp)->nparams > 0) && ((*e2pp)->nparams > 0)) {
		            rc = ((*e1pp)->p[0] - (*e2pp)->p[0]) ;
	                } else if ((*e1pp)->nparams > 0) {
		            rc = 1 ;
	                } else if ((*e2pp)->nparams > 0) {
		            rc = -1 ;
	                } else {
		            rc = 0 ;
	                }
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}

#if	CF_DEBUGS
	debugprintf("kbdinfo/vcmpfind: ret rc=%d\n",rc) ;
#endif

	return rc ;
}
/* end subroutine (vcmpfind) */


