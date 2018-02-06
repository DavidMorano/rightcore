/* progkey */

/* process the input files */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG 	0		/* run-time debugging */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

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

#include	<vsystem.h>
#include	<ids.h>
#include	<bfile.h>
#include	<field.h>
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

#define	DISP		struct disp_head
#define	DISP_THR	struct disp_thr
#define	DISP_ARGS	struct disp_args


/* typedefs */


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnprocessors(cchar **,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

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

struct disp_args {
	PROGINFO	*pip ;
	DISP		*dop ;
	bfile		*ofp ;
	const uchar	*terms ;
	const char	*delimiter ;
	const char	*ignchrs ;
	int		npar ;
} ;

struct disp_thr {
	pthread_t	tid ;
	int		f_active ;
} ;

struct disp_head {
	PROGINFO	*pip ;
	DISP_THR	*threads ;
	DISP_ARGS	a ;		/* arguments */
	FSI		wq ;
	PSEM		wq_sem ;
	PTM		om ;		/* output mutex */
	volatile int	f_exit ;
	volatile int	f_done ;
	int		nthr ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,PROGINFO *, ARGINFO *,
			cuchar *,cchar *,cchar *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_sendparams(SUBINFO *,bfile *) ;
static int	subinfo_sendparamseigens(SUBINFO *,bfile *) ;
static int	subinfo_sendparamsval(SUBINFO *,bfile *,cchar *,int) ;
static int	subinfo_sendparamsstr(SUBINFO *,bfile *,cchar *,cchar *) ;

static int	subinfo_args(SUBINFO *,DISP *) ;
static int	subinfo_argfile(SUBINFO *,DISP *) ;
static int	subinfo_stdin(SUBINFO *,DISP *) ;
static int	subinfo_procfile(SUBINFO *,DISP *,const char *) ;

static int	ereport(PROGINFO *,const char *,int) ;

static int	disp_start(DISP *,DISP_ARGS *) ;
static int	disp_starter(DISP *) ;
static int	disp_addwork(DISP *,cchar *,int) ;
static int	disp_finish(DISP *,int) ;
static int	disp_worker(DISP *) ;


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
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;

	if ((rs = subinfo_start(sip,pip,aip,terms,delimiter,ignchrs)) >= 0) {
	    bfile	ofile, *ofp = &ofile ;
	    char	openstr[10 + 1] ;

/* open the output key file */

	    if ((ofname == NULL) || (ofname[0] == '\0'))
	        ofname = BFILE_STDOUT ;

	    strcpy(openstr,"wc") ;
	    if (pip->f.append) {
	        strcat(openstr,"a") ;
	    } else {
	        strcat(openstr,"t") ;
	    }

	    if ((rs = bopen(ofp,ofname,openstr,0666)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf( "progkey: ofname=%s\n",ofname) ;
	            debugprintf( "progkey: bopen() rs=%d\n",rs) ;
	        }
#endif

/* output parameters */

	        if ((rs = subinfo_sendparams(sip,ofp)) >= 0) {
	            DISP_ARGS	wa ;
	            DISP	disp ;

/* process the arguments */

	            memset(&wa,0,sizeof(DISP_ARGS)) ;
	            wa.pip = pip ;
	            wa.terms = terms ;
	            wa.delimiter = delimiter ;
	            wa.ignchrs = ignchrs ;
	            wa.ofp = ofp ;
		    wa.npar = pip->npar ;

	            if ((rs = disp_start(&disp,&wa)) >= 0) {
	                if ((rs = subinfo_args(sip,&disp)) >= 0) {
	                    if ((rs = subinfo_argfile(sip,&disp)) >= 0) {
	                        rs = subinfo_stdin(sip,&disp) ;
	                    }
	                }
	                rs1 = disp_finish(&disp,(rs < 0)) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (disp) */

	        } /* end if (subinfo-sendparams) */

	        rs1 = bclose(ofp) ;
	        if (rs >= 0) rs = rs1 ;
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
			cchar	*elang = pip->eigenlang ;
	                rs = subinfo_sendparamsstr(sip,ofp,cp,elang) ;
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


static int subinfo_sendparamsval(SUBINFO *sip,bfile *ofp,cchar *cmd,int v)
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


static int subinfo_sendparamsstr(SUBINFO *sip,bfile *ofp,cchar *cmd,cchar *s)
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


static int subinfo_args(SUBINFO *sip,DISP *dop)
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


static int subinfo_argfile(SUBINFO *sip,DISP *dop)
{
	PROGINFO	*pip = sip->pip ;
	ARGINFO		*aip = sip->aip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if ((aip->afname != NULL) && (aip->afname[0] != '\0')) {
	    bfile	afile ;
	    cchar	*afname = aip->afname ;

	    if (afname[0] == '-') afname = BFILE_STDIN ;

	    if ((rs = bopen(&afile,afname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		cl ;
	        cchar		*cp ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(&afile,lbuf,llen)) > 0) {
	            int	len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    lbuf[(cp+cl)-lbuf] = '\0' ;
	                    sip->pan += 1 ;
	                    rs = subinfo_procfile(sip,dop,cp) ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(&afile) ;
	        if (rs >= 0) rs = rs1 ;
	    } else if (! pip->f.quiet) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: inaccessible (%d) afile=%s\n" ;
	        bprintf(pip->efp,fmt,pn,rs,aip->afname) ;
	    } /* end if */

	} /* end if (have) */

	return rs ;
}
/* end subroutine (subinfo_argfile) */


static int subinfo_stdin(SUBINFO *sip,DISP *dop)
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


static int subinfo_procfile(SUBINFO *sip,DISP *dop,cchar *fname)
{
	PROGINFO	*pip = sip->pip ;
	int		rs = SR_OK ;

/* ignore all files that start w/ a '-' character */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: processing file=%s\n",
	        pip->progname,fname) ;
	}

	if (fname[0] != '-') {
	    USTAT	sb ;
	    if ((rs = u_stat(fname,&sb)) >= 0) {
	        if ((rs = sperm(&sip->id,&sb,R_OK)) >= 0) {
	            rs = disp_addwork(dop,fname,-1) ;
	        } else if (isNotAccess(rs)) {
		    if (pip->f.iacc) {
			if (pip->debuglevel > 0) {
	                    ereport(pip,fname,rs) ;
			}
			rs = SR_OK ;
		    } else {
	                ereport(pip,fname,rs) ;
		    }
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (flie check) */

	return rs ;
}
/* end subroutine (subinfo_procfile) */


static int disp_start(DISP *dop,DISP_ARGS *wap)
{
	PROGINFO	*pip ;
	int		rs = SR_OK ;

	if (dop == NULL) return SR_FAULT ;
	if (wap == NULL) return SR_FAULT ;

	pip = wap->pip ;

	memset(dop,0,sizeof(DISP)) ;
	dop->pip = pip ;
	dop->a = *wap ;
	dop->nthr = wap->npar ;

	if ((rs = fsi_start(&dop->wq)) >= 0) {
	    if ((rs = psem_create(&dop->wq_sem,FALSE,0)) >= 0) {
		if ((rs = ptm_create(&dop->om,NULL)) >= 0) {
		    const int	size = (dop->nthr * sizeof(DISP_THR)) ;
		    void	*p ;
		    if ((rs = uc_malloc(size,&p)) >= 0) {
		        dop->threads = p ;
		        memset(p,0,size) ;
			rs = disp_starter(dop) ;
		        if (rs < 0) {
			    uc_free(dop->threads) ;
			    dop->threads = NULL ;
			}
		    } /* end if (m-a) */
		    if (rs < 0)
			ptm_destroy(&dop->om) ;
		} /* end if (ptm_create) */
		if (rs < 0)
		    psem_destroy(&dop->wq_sem) ;
	    } /* end if (psem_create) */
	    if (rs < 0)
		fsi_finish(&dop->wq) ;
	} /* end if (fsi_start) */

	return rs ;
}
/* end subroutine (disp_start) */


static int disp_starter(DISP *dop)
{
	uptsub_t	fn = (uptsub_t) disp_worker ;
	pthread_t	tid ;
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; (rs >= 0) && (i < dop->nthr) ; i += 1) {
	    if ((rs = uptcreate(&tid,NULL,fn,dop)) >= 0) {
	        dop->threads[i].tid = tid ;
	        dop->threads[i].f_active = TRUE ;
	    }
	} /* end for */

	if (rs < 0) {
	    const int	n = i ;
	    dop->f_exit = TRUE ;
	    for (i = 0 ; i < n ; i += 1) {
	        psem_post(&dop->wq_sem) ;
	    }
	    for (i = 0 ; i < n ; i += 1) {
	        tid = dop->threads[i].tid ;
	        uptjoin(tid,NULL) ;
		dop->threads[i].f_active = FALSE ;
	    }
	} /* end if (failure) */

	return rs ;
}
/* end subroutine (disp_starter) */


static int disp_finish(DISP *dop,int f_abort)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (dop == NULL) return SR_FAULT ;

	dop->f_done = TRUE ;
	if (f_abort)
	    dop->f_exit = TRUE ;

	for (i = 0 ; i < dop->nthr ; i += 1) {
	    psem_post(&dop->wq_sem) ;
	}

	if (dop->threads != NULL) {
	    DISP_THR	*dtp ;
	    pthread_t	tid ;
	    int		trs ;
	    for (i = 0 ; i < dop->nthr ; i += 1) {
		dtp = (dop->threads+i) ;
	        if (dtp->f_active) {
	            dtp->f_active = FALSE ;
	            tid = dtp->tid ;
	            rs1 = uptjoin(tid,&trs) ;
		    if (rs >= 0) rs = rs1 ;
		    if (rs >= 0) rs = trs ;
	        } /* end if (active) */
	    } /* end for */
	    rs1 = uc_free(dop->threads) ;
	    if (rs >= 0) rs = rs1 ;
	    dop->threads = NULL ;
	} /* end if (threads) */

	rs1 = ptm_destroy(&dop->om) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = psem_destroy(&dop->wq_sem) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = fsi_finish(&dop->wq) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (disp_finish) */


static int disp_addwork(DISP *dop,cchar *tagbuf,int taglen)
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


static int disp_worker(DISP *dop)
{
	PROGINFO	*pip = dop->pip ;
	DISP_ARGS	*wap = &dop->a ;
	PTM		*omp = &dop->om ;
	const int	rlen = MAXPATHLEN ;
	int		rs ;
	int		c = 0 ;
	char		rbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    pthread_t	tid = pthread_self() ;
	    debugprintf("progkey/worker: ent tid=%u\n",tid) ;
	}
#endif

	while ((rs = psem_wait(&dop->wq_sem)) >= 0) {
	    if (dop->f_exit) break ;

	    if ((rs = fsi_remove(&dop->wq,rbuf,rlen)) >= 0) {

	        rs = progkeyer(pip,wap->ofp,omp,
	            wap->terms,wap->delimiter,wap->ignchrs,rbuf) ;
	        if (rs > 0) c += 1 ;

	    } else if (rs == SR_NOTFOUND) {
		rs = SR_OK ;
	        if (dop->f_done) break ;
	    } /* end if (work to do) */

	    if (rs < 0) break ;
	} /* end while (server loop) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    pthread_t	tid = pthread_self() ;
	    debugprintf("progkey/worker: ret tid=%u rs=%d c=%u\n",tid,rs,c) ;
	}
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (disp_worker) */


static int ereport(PROGINFO *pip,cchar *fname,int frs)
{
	int		rs = SR_OK ;
	if (! pip->f.quiet) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: file-processing error (%d)\n",pn,frs) ;
	    bprintf(pip->efp,"%s: file=%s\n",pn,fname) ;
	}
	return rs ;
}
/* end subroutine (ereport) */


