/* progkey */

/* process the input files */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes all of the files given us through the program
	invoation arguments.

	Synopsis:

	int progkey(pip,aip,terms,delimiter,ignchrs,ofname)
	PROGINFO	*pip ;
	ARGINFO		*aip ;
	cuchar		terms[] ;
	cchar		delimiter[], ignchrs[] ;
	cchar		ofname[] ;

	Arguments:

	- pip		program information pointer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecobj.h>
#include	<hdb.h>
#include	<ptm.h>
#include	<psem.h>
#include	<eigendb.h>
#include	<localmisc.h>

#include	"fsi.h"
#include	"upt.h"
#include	"config.h"
#include	"defs.h"
#include	"progeigen.h"
#include	"mkcmds.h"


/* local defines */

#define	SUBINFO		struct subinfo
#define	DISP		struct disp
#define	WARGS		struct wargs


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnprocessors(const char **,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(const char *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern int	progkeyer(PROGINFO *,bfile *,PTM *,
cuchar *,cchar *,cchar *,char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct subinfo {
	PROGINFO	*pip ;
	ARGINFO		*aip ;
	const uchar	*terms ;
	const char	*delimiter ;
	const char	*ignchrs ;
	IDS		id ;
	int		pan ;
} ;

struct disp {
	PROGINFO	*pip ;
	WARGS		*wap ;
	FSI		wq ;
	PSEM		wq_sem ;
	PTM		om ;		/* output mutex */
	vecobj		tids ;
	volatile int	f_exit ;
	volatile int	f_done ;
	int		n ;
} ;

struct wargs {
	PROGINFO	*pip ;
	DISP		*dop ;
	PTM		*omp ;
	bfile		*ofp ;
	const uchar	*terms ;
	const char	*delimiter ;
	const char	*ignchrs ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,PROGINFO *,
ARGINFO *,cuchar *,cchar *,cchar *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_sendparams(SUBINFO *,bfile *) ;
static int	subinfo_sendparamseigens(SUBINFO *,bfile *) ;
static int	subinfo_sendparamsval(SUBINFO *,bfile *,cchar *,int) ;
static int	subinfo_sendparamsstr(SUBINFO *,bfile *,cchar *,cchar *) ;

static int	subinfo_args(SUBINFO *,DISP *) ;
static int	subinfo_argfile(SUBINFO *,DISP *) ;
static int	subinfo_stdin(SUBINFO *,DISP *) ;
static int	subinfo_procfile(SUBINFO *,DISP *,const char *) ;

static int	worker(void *) ;

static int	ereport(PROGINFO *,const char *,int) ;

static int	disp_start(DISP *,WARGS *) ;
static int	disp_addwork(DISP *,const char *,int) ;
static int	disp_finish(DISP *,int) ;


/* local variables */


/* exported subroutines */


int progkey(pip,aip,terms,delimiter,ignchrs,ofname)
PROGINFO	*pip ;
ARGINFO		*aip ;
const uchar	terms[] ;
const char	delimiter[] ;
const char	ignchrs[] ;
const char	ofname[] ;
{
	SUBINFO		si, *sip = &si ;
	WARGS		wa ;
	DISP		disp ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	int		f ;

	if ((rs = subinfo_start(sip,pip,aip,terms,delimiter,ignchrs)) >= 0) {
	    bfile	ofile, *ofp = &ofile ;
	    char	openstr[10 + 1] ;

/* open the output key file */

	    if ((ofname == NULL) || (ofname[0] == '\0'))
	        ofname = BFILE_STDOUT ;

	    strcpy(openstr,"wc") ;
	    if (pip->f.append) {
	        strcat(openstr,"a") ;
	    } else
	        strcat(openstr,"t") ;

	    if ((rs = bopen(ofp,ofname,openstr,0666)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf( "progkey: ofname=%s\n",ofname) ;
	            debugprintf( "progkey: bopen() rs=%d\n",rs) ;
	        }
#endif

/* output parameters */

	        if ((rs = subinfo_sendparams(sip,ofp)) >= 0) {

/* process the arguments */

	            memset(&wa,0,sizeof(WARGS)) ;
	            wa.pip = pip ;
	            wa.terms = terms ;
	            wa.delimiter = delimiter ;
	            wa.ignchrs = ignchrs ;
	            wa.ofp = ofp ;

	            if ((rs = disp_start(&disp,&wa)) >= 0) {

	                rs = subinfo_args(sip,&disp) ;

	                if (rs >= 0)
	                    rs = subinfo_argfile(sip,&disp) ;

	                if (rs >= 0)
	                    rs = subinfo_stdin(sip,&disp) ;

	                f = (rs < 0) ;
	                rs1 = disp_finish(&disp,f) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (disp) */

	        } /* end if (subinfo-sendparams) */

	        bclose(ofp) ;
	    } /* end if (output-opened) */

	    pan = subinfo_finish(sip) ;
	    if (rs >= 0) rs = pan ;
	} /* end if (subinfo) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkey: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (progkey) */


/* local subroutines */


static int subinfo_start(sip,pip,aip,terms,delimiter,ignchrs)
SUBINFO		*sip ;
PROGINFO	*pip ;
ARGINFO		*aip ;
const uchar	terms[] ;
const char	delimiter[] ;
const char	ignchrs[] ;
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pip = pip ;
	sip->aip = aip ;
	sip->terms = terms ;
	sip->delimiter = delimiter ;
	sip->ignchrs = ignchrs ;

	rs = ids_load(&sip->id) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = sip->pan ;

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_sendparams(SUBINFO *sip,bfile *ofp)
{
	PROGINFO	*pip = sip->pip ;
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip->f.optsendparams) {
	    int		i ;
	    int		v ;
	    cchar	*cp ;
	    for (i = 0 ; i < mkcmd_overlast ; i += 1) {
	        cp = mkcmds[i] ;
	        switch (i) {
	        case mkcmd_minwordlen:
	            v = pip->minwordlen ;
	            rs = subinfo_sendparamsval(sip,ofp,cp,v) ;
	            wlen += rs ;
	            break ;
	        case mkcmd_maxwordlen:
	            v = pip->maxwordlen ;
	            rs = subinfo_sendparamsval(sip,ofp,cp,v) ;
	            wlen += rs ;
	            break ;
	        case mkcmd_eigenwords:
	            if (pip->open.eigendb) {
	                rs = subinfo_sendparamseigens(sip,ofp) ;
	                wlen += rs ;
	            }
	            break ;
	        case mkcmd_nkeys:
	            v = pip->maxkeys ;
	            rs = subinfo_sendparamsval(sip,ofp,cp,v) ;
	            wlen += rs ;
	            break ;
	        case mkcmd_tablen:
	            rs = subinfo_sendparamsval(sip,ofp,cp,v) ;
	            wlen += rs ;
	            break ;
	        case mkcmd_sdn:
	            rs = subinfo_sendparamsstr(sip,ofp,cp,pip->sdn) ;
	            wlen += rs ;
	            break ;
	        case mkcmd_sfn:
	            rs = subinfo_sendparamsstr(sip,ofp,cp,pip->sfn) ;
	            wlen += rs ;
	            break ;
	        case mkcmd_lang:
		    if (pip->eigenlang != NULL) {
	                rs = subinfo_sendparamsstr(sip,ofp,cp,pip->eigenlang) ;
	                wlen += rs ;
		    }
	            break ;
	        } /* end switch */
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparams) */


static int subinfo_sendparamseigens(SUBINFO *sip,bfile *ofp)
{
	PROGINFO	*pip = sip->pip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;

	if (pip->open.eigendb) {
	    if ((rs = progeigen_count(pip)) > 0) {
	        const int	ci = mkcmd_eigenwords ;
	        const int	linelen = COLUMNS ;
		int		llen = 0 ;
	        if ((rs = bprintf(ofp,"-%s",mkcmds[ci])) >= 0) {
		    PROGEIGEN_CUR	ecur ;
	            int			wl ;
	            cchar		*wp ;
	            wlen += rs ;
	            llen += rs ;
	            if ((rs = progeigen_curbegin(pip,&ecur)) >= 0) {

	        while (rs >= 0) {
	            wl = progeigen_enum(pip,&ecur,&wp) ;
	            if (wl == SR_NOTFOUND) break ;
	            rs = wl ;

	            if ((rs >= 0) && (wl > 0)) {

	                if ((wl + 1) > (linelen - llen)) {
	                    llen = 0 ;
	                    rs = bprintf(ofp,"\n") ;
	                    wlen += rs ;
	                    if (rs >= 0) {
	                        rs = bprintf(ofp,"-%s",mkcmds[ci]) ;
	                        wlen += rs ;
	                        llen += rs ;
	                    }
	                }

	                if (rs >= 0) {
	                    rs = bprintf(ofp," %t",wp,wl) ;
	                    wlen += rs ;
	                    llen += rs ;
	                }

	            } /* end if (have eigen-word) */

	        } /* end while */

	        if ((rs >= 0) && (llen > 0)) {
	            llen = 0 ;
	            rs = bprintf(ofp,"\n") ;
	            wlen += rs ;
	        }

	                rs1 = progeigen_curend(pip,&ecur) ;
		        if (rs >= 0) rs = rs1 ;
	            } /* end if (progeigen-cur) */
	        } /* end if (ok) */
	    } /* end if (progeigen_count) */
	} /* end if (eigen-open) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparamseigens) */


static int subinfo_sendparamsval(sip,ofp,cmd,v)
SUBINFO		*sip ;
bfile		*ofp ;
const char	cmd[] ;
int		v ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (sip == NULL) return SR_FAULT ;

	if (v >= 0) {
	    rs = bprintf(ofp,"-%s %u\n",cmd,v) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparamsval) */


static int subinfo_sendparamsstr(sip,ofp,cmd,s)
SUBINFO		*sip ;
bfile		*ofp ;
const char	cmd[] ;
const char	s[] ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (sip == NULL) return SR_FAULT ;

	if ((s != NULL) && (s[0] != '\0')) {
	    rs = bprintf(ofp,"-%s %s\n",cmd,s) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparamsstr) */


static int subinfo_args(sip,dop)
SUBINFO		*sip ;
DISP		*dop ;
{
	PROGINFO	*pip = sip->pip ;
	ARGINFO		*aip = sip->aip ;
	int		rs = SR_OK ;
	int		ai ;
	int		f ;
	const char	*cp ;

	if (pip == NULL) return SR_FAULT ;

	for (ai = 1 ; ai < aip->argc ; ai += 1) {
	    f = (ai <= aip->ai_max) && (bits_test(&aip->pargs,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (f) {
	        cp = aip->argv[ai] ;
	        sip->pan += 1 ;
	        rs = subinfo_procfile(sip,dop,cp) ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (subinfo_args) */


static int subinfo_argfile(sip,dop)
SUBINFO		*sip ;
DISP		*dop ;
{
	PROGINFO	*pip = sip->pip ;
	ARGINFO		*aip = sip->aip ;
	bfile		argfile ;
	int		rs = SR_OK ;
	const char	*afname ;

	afname = aip->afname ;
	if ((aip->afname == NULL) || (aip->afname[0] == '\0'))
	    goto ret0 ;

	if (afname[0] == '-') afname = BFILE_STDIN ;

	if ((rs = bopen(&argfile,afname,"r",0666)) >= 0) {
	    int	len ;
	    char	linebuf[LINEBUFLEN + 1] ;
	    char	*cp ;

	    while ((rs = breadline(&argfile,linebuf,LINEBUFLEN)) > 0) {
	        len = rs ;

	        if (linebuf[len - 1] == '\n') len -= 1 ;
	        linebuf[len] = '\0' ;

	        cp = linebuf ;
	        if ((cp[0] == '\0') || (cp[0] == '#'))
	            continue ;

	        sip->pan += 1 ;
	        rs = subinfo_procfile(sip,dop,cp) ;

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    bclose(&argfile) ;
	} else if (! pip->f.quiet) {
	    bprintf(pip->efp,"%s: unaccessible (%d) argfile=%s\n",
	        pip->progname,rs,aip->afname) ;
	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (subinfo_argfile) */


static int subinfo_stdin(sip,dop)
SUBINFO		*sip ;
DISP		*dop ;
{
	int		rs = SR_OK ;

	if (sip->pan == 0) {
	    const char	*cp = "-" ;
	    sip->pan += 1 ;
	    rs = subinfo_procfile(sip,dop,cp) ;
	}

	return rs ;
}
/* end subroutine (subinfo_stdin) */


static int subinfo_procfile(sip,dop,fname)
SUBINFO		*sip ;
DISP		*dop ;
const char	fname[] ;
{
	PROGINFO	*pip = sip->pip ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;

/* ignore all files that start w/ a '-' character */

	if ((fname[0] == '-') && (fname[1] != '\0'))
	    goto ret0 ;

/* continue */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: processing file=%s\n",
	        pip->progname,fname) ;
	}

	if (fname[0] != '-') {

	    rs1 = u_stat(fname,&sb) ;
	    if (rs1 >= 0)
	        rs1 = sperm(&sip->id,&sb,R_OK) ;

	    if (rs1 < 0)
	        ereport(pip,fname,rs1) ;

	} /* end if (flie check) */

	if (rs1 >= 0)
	    rs = disp_addwork(dop,fname,-1) ;

ret0:
	return rs ;
}
/* end subroutine (subinfo_procfile) */


static int disp_start(dop,wap)
DISP		*dop ;
WARGS		*wap ;
{
	PROGINFO	*pip ;
	pthread_t	tid, *tidp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		size ;
	int		opts ;
	int		n, i ;

	if (dop == NULL) return SR_FAULT ;
	if (wap == NULL) return SR_FAULT ;

	pip = wap->pip ;
	wap->dop = dop ;

	memset(dop,0,sizeof(DISP)) ;
	dop->pip = pip ;
	dop->wap = wap ;

	rs = fsi_start(&dop->wq) ;
	if (rs < 0)
	    goto bad0 ;

	rs = psem_init(&dop->wq_sem,FALSE,0) ;
	if (rs < 0)
	    goto bad2 ;

	size = sizeof(pthread_t) ;
	opts = (VECOBJ_OREUSE) ;
	rs = vecobj_start(&dop->tids,size,10,opts) ;
	if (rs < 0)
	    goto bad3 ;

	rs = ptm_init(&dop->om,NULL) ;
	if (rs < 0)
	    goto bad4 ;

	wap->omp = &dop->om ;

	dop->n = pip->npar ;
	if (dop->n == 0) {
	    rs1 = getnprocessors(pip->envv,0) ;
	    dop->n = (rs1 >= 0) ? (rs1 + 1) : 1 ;
	}

	n = uptgetconcurrency() ;
	if (dop->n > n) {
	    uptsetconcurrency(dop->n) ;
	}

	for (i = 0 ; i < dop->n ; i += 1) {

	    rs = uptcreate(&tid,NULL,worker,wap) ;
	    if (rs < 0)
	        break ;

	    rs = vecobj_add(&dop->tids,&tid) ;
	    if (rs < 0)
	        break ;

	} /* end for */

	if (rs < 0) {
	    dop->f_exit = TRUE ;
	    for (i = 0 ; i < dop->n ; i += 1)
	        psem_post(&dop->wq_sem) ;
	    for (i = 0 ; vecobj_get(&dop->tids,i,&tidp) >= 0 ; i += 1) {
	        if (tidp == NULL) continue ;
	        uptjoin(*tidp,NULL) ;
	    }
	} /* end if (failure) */

	if (rs < 0)
	    goto bad6 ;

ret0:
	return rs ;

/* bad stuff */
bad6:
	ptm_destroy(&dop->om) ;

bad4:
	vecobj_finish(&dop->tids) ;

bad3:
	psem_destroy(&dop->wq_sem) ;

bad2:
	fsi_finish(&dop->wq) ;

bad0:
	goto ret0 ;
}
/* end subroutine (disp_start) */


static int disp_finish(dop,f_abort)
DISP		*dop ;
int		f_abort ;
{
	pthread_t	*tidp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (dop == NULL)
	    return SR_FAULT ;

	dop->f_done = TRUE ;
	if (f_abort)
	    dop->f_exit = TRUE ;

	for (i = 0 ; i < dop->n ; i += 1) {
	    psem_post(&dop->wq_sem) ;
	}

	for (i = 0 ; vecobj_get(&dop->tids,i,&tidp) >= 0 ; i += 1) {
	    if (tidp == NULL) continue ;
	    uptjoin(*tidp,NULL) ;
	}

	rs1 = ptm_destroy(&dop->om) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&dop->tids) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->wq_sem) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = fsi_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (disp_finish) */


static int disp_addwork(dop,tagbuf,taglen)
DISP		*dop ;
const char	tagbuf[] ;
int		taglen ;
{
	PROGINFO	*pip = dop->pip ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("disp_addwork: tag=>%t<\n",tagbuf,taglen) ;
#endif

	if ((rs = fsi_add(&dop->wq,tagbuf,taglen)) >= 0) {
	    rs = psem_post(&dop->wq_sem) ;
	}

	return rs ;
}
/* end subroutine (disp_addwork) */


static int worker(ptvp)
void		*ptvp ;
{
	PROGINFO	*pip ;
	WARGS		*wap = (WARGS *) ptvp ;
	DISP		*dop ;
	pthread_t	tid ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	char		fname[MAXPATHLEN + 1] ;

	pip = wap->pip ;
	dop = wap->dop ;
	uptself(&tid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkey/worker: ent tid=%u\n",tid) ;
#endif

	while (rs >= 0) {

	    while ((rs = psem_wait(&dop->wq_sem)) < 0) {
	        if ((rs != SR_AGAIN) && (rs != SR_INTR)) break ;
	    } /* end while */

	    if (rs < 0) break ;

	    if (dop->f_exit) break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progkey/worker: tid=%u wakeup\n",tid) ;
#endif

	    rs1 = fsi_remove(&dop->wq,fname,MAXPATHLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("progkey/worker: t=%u fsi_remove() rs=%d\n",
	            tid,rs1) ;
#endif

	    if ((rs1 == SR_NOTFOUND) && dop->f_done)
	        break ;

	    if (rs1 > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progkey/worker: tid=%u fname=%s\n",
	                tid,fname) ;
#endif

	        rs = progkeyer(pip,wap->ofp,wap->omp,
	            wap->terms,wap->delimiter,wap->ignchrs,
	            fname) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progkey/worker: tid=%u progkeyer() rs=%d\n",
	                tid,rs) ;
#endif

	        if (rs > 0)
	            c += 1 ;

	    } /* end if (work to do) */

	    if (rs >= 0)
	        rs = rs1 ;

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkey/worker: ret tid=%u rs=%d c=%u\n",tid,rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (worker) */


static int ereport(pip,fname,frs)
PROGINFO	*pip ;
const char	fname[] ;
int		frs ;
{
	int		rs = SR_OK ;

	if (! pip->f.quiet) {
	    bprintf(pip->efp,"%s: file-processing error (%d)\n",
	        pip->progname,frs) ;
	    bprintf(pip->efp,"%s: file=%s\n",
	        pip->progname,fname) ;
	}

	return rs ;
}
/* end subroutine (ereport) */


