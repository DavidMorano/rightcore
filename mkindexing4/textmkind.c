/* textmkind *NOT_FINISHED* */

/* make an index for some text files */


#define	CF_DEBUG 	0		/* run-time debugging */
#define	CF_EXTRAWORDS	1		/* extra words? */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module will create a single index out of all of the files given to
	us.


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
#include	"textmkind.h"
#include	"xwords.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	MAXEXTRAWORDS	3

#define	DISP		struct disp
#define	WARGS		struct wargs


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	sfword(const char *,int,const char **) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getnprocessors(const char **,int) ;
extern int	field_word(FIELD *,const uchar *,const char **) ;
extern int	haslc(const char *,int) ;
extern int	hasuc(const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isalnumlatin(int) ;

extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct subinfo {
	PROGINFO	*pip ;
	ARGINFO	*aip ;
	const uchar	*terms ;
	const char	*delimiter ;
	const char	*ignorechars ;
	IDS		id ;
	int		pan ;
} ;

struct disp {
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
	const char	*ignorechars ;
} ;


/* forward references */

static int	subinfo_start(struct subinfo *,PROGINFO *,
			ARGINFO *,const uchar *,
			const char *,const char *) ;
static int	subinfo_finish(struct subinfo *) ;
static int	subinfo_sendparams(struct subinfo *,bfile *) ;
static int	subinfo_sendparamseigens(struct subinfo *,bfile *) ;
static int	subinfo_sendparamsval(struct subinfo *,bfile *,
			const char *,int) ;
static int	subinfo_sendparamsstr(struct subinfo *,bfile *,
			const char *,const char *) ;

static int	subinfo_args(struct subinfo *,DISP *) ;
static int	subinfo_argfile(struct subinfo *,DISP *) ;
static int	subinfo_stdin(struct subinfo *,DISP *) ;
static int	subinfo_procfile(struct subinfo *,DISP *,const char *) ;

static int	worker(void *) ;

static int	disp_start(DISP *,WARGS *) ;
static int	disp_addwork(DISP *,const char *,int) ;
static int	disp_finish(DISP *,int) ;

static int	progkey(PROGINFO *,bfile *,PTM *,
			const uchar *,
			const char *,const char *,char *) ;

static int	ignoreline(const char *,int,const char *) ;
static int	procword(PROGINFO *,HDB *,
			int, const char *,int) ;

extern int	keysstart(PROGINFO *,HDB *,int) ;
extern int	keysadd(PROGINFO *,HDB *,const char *,int) ;
extern int	keysfinish(PROGINFO *,HDB *,bfile *,PTM *,const char *,
			offset_t,int) ;


/* local variables */


/* exported subroutines */


int textmkind_open(op,pap,basedname,dbname)
TEXTMKIND	*op ;
TEXTMKIND_PARAMS	*pap ;
const char	basedname[] ;
const char	dbname[] ;
{
	int	rs = SR_OK ;



	return rs ;
}
/* end subroutine (textmkind_open) */


extern int textmkind_open(TEXTMKIND *,const char *,const char *,const char *) ;
extern int textmkind_count(TEXTMKIND *) ;
extern int textmkind_info(TEXTMKIND *,TEXTMKIND_INFO *) ;
extern int textmkind_add(TEXTMKIND *,const char *,int) ;
extern int textmkind_close(TEXTMKIND *) ;


int textmkind_close(op)
TEXTMKIND	*op ;
{
	int	rs = SR_OK ;


	return rs ;
}
/* end subroutine (textmkind_open) */






int mkkey(pip,aip,terms,delimiter,ignorechars,outfname)
PROGINFO	*pip ;
ARGINFO	*aip ;
const uchar	terms[] ;
const char	delimiter[] ;
const char	ignorechars[] ;
const char	outfname[] ;
{
	struct subinfo	si, *sip = &si ;

	WARGS	wa ;

	DISP	disp ;

	bfile	ofile, *ofp = &ofile ;

	int	rs = SR_OK ;
	int	pan = 0 ;
	int	f ;

	char	openstr[10 + 1] ;


	rs = subinfo_start(sip,pip,aip,terms,delimiter,ignorechars) ;
	if (rs < 0)
	    goto ret0 ;

/* open the output key file */

	strcpy(openstr,"wc") ;
	if (pip->f.append) {
	    strcat(openstr,"a") ;
	} else {
	    strcat(openstr,"t") ;
	}

	if ((outfname == NULL) || (outfname[0] == '\0')) {
	    strcat(openstr,"d") ;
	    rs = bopen(ofp,BFILE_STDOUT,openstr,0666) ;
	} else
	    rs = bopen(ofp,outfname,openstr,0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf( "mkkey: outfname=%s\n",outfname) ;
	    debugprintf( "mkkey: bopen() rs=%d\n",rs) ;
	}
#endif

	if (rs < 0)
	    goto ret1 ;

/* output parameters */

	rs = subinfo_sendparams(sip,ofp) ;
	if (rs < 0)
	    goto ret1 ;

/* process the arguments */

	memset(&wa,0,sizeof(WARGS)) ;
	wa.pip = pip ;
	wa.terms = terms ;
	wa.delimiter = delimiter ;
	wa.ignorechars = ignorechars ;
	wa.ofp = ofp ;

	if ((rs = disp_start(&disp,&wa)) >= 0) {

	    rs = subinfo_args(sip,&disp) ;

	    if (rs >= 0)
	        rs = subinfo_argfile(sip,&disp) ;

	    if (rs >= 0)
	        rs = subinfo_stdin(sip,&disp) ;

	    f = (rs < 0) ;
	    disp_finish(&disp,f) ;
	} /* end if */

	bclose(ofp) ;

ret1:
	pan = subinfo_finish(sip) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkkey: ret rs=%d pan=%u\n",rs,pan) ;
#endif

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (mkkey) */


/* local subroutines */


static int subinfo_start(sip,pip,aip,terms,delimiter,ignorechars)
struct subinfo	*sip ;
PROGINFO	*pip ;
ARGINFO	*aip ;
const uchar	terms[] ;
const char	delimiter[] ;
const char	ignorechars[] ;
{
	int		rs ;

	memset(sip,0,sizeof(struct subinfo)) ;
	sip->pip = pip ;
	sip->aip = aip ;
	sip->terms = terms ;
	sip->delimiter = delimiter ;
	sip->ignorechars = ignorechars ;

	rs = ids_load(&sip->id) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;

	rs1 = sip->pan ;
	pan = rs1 ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_sendparams(sip,ofp)
struct subinfo	*sip ;
bfile		*ofp ;
{
	PROGINFO	*pip = sip->pip ;

	int		rs = SR_OK ;
	int		i ;
	int		v ;
	int		wlen = 0 ;

	if (pip->f.optsendparams) {
	    for (i = 0 ; i < mkcmd_overlast ; i += 1) {
		const char	*cp = mkcmds[i] ;

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

	    } /* end switch */

	        if (rs < 0) break ;
	    } /* end for */
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparams) */


static int subinfo_sendparamseigens(sip,ofp)
struct subinfo	*sip ;
bfile		*ofp ;
{
	PROGINFO	*pip = sip->pip ;

	EIGENDB		*edbp = &pip->eigendb ;

	EIGENDB_CUR	ecur ;

	const int	ci = mkcmd_eigenwords ;
	const int	linelen = COLUMNS ;

	int	rs = SR_OK ;
	int	n ;
	int	wl ;
	int	llen ;
	int	wlen = 0 ;

	char	*wp ;


	if (! pip->open.eigendb)
	    goto ret0 ;

	n = eigendb_count(edbp) ;
	if (n <= 0)
	    goto ret0 ;

	llen = 0 ;
	rs = bprintf(ofp,"-%s",mkcmds[ci]) ;
	wlen += rs ;
	llen += rs ;
	if (rs >= 0) {

	    eigendb_curbegin(edbp,&ecur) ;

	    while (rs >= 0) {

	        wl = eigendb_enum(edbp,&ecur,&wp) ;
	        if (wl < 0)
	            break ;

	        if (wl > 0) {

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

	        } /* end if (have eigen word) */

	        if (rs < 0)
	            break ;

	    } /* end while */

	    if ((rs >= 0) && (llen > 0)) {
	        llen = 0 ;
	        rs = bprintf(ofp,"\n") ;
	        wlen += rs ;
	    }

	    eigendb_curend(edbp,&ecur) ;

	} /* end if */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparamseigens) */


static int subinfo_sendparamsval(sip,ofp,cmd,v)
struct subinfo	*sip ;
bfile		*ofp ;
const char	cmd[] ;
int		v ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (v >= 0) {
	    rs = bprintf(ofp,"-%s %u\n",cmd,v) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparamsval) */


static int subinfo_sendparamsstr(sip,ofp,cmd,s)
struct subinfo	*sip ;
bfile		*ofp ;
const char	cmd[] ;
const char	s[] ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if ((s != NULL) && (s[0] != '\0')) {
	    rs = bprintf(ofp,"-%s %s\n",cmd,s) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subinfo_sendparamsstr) */


static int subinfo_args(sip,dop)
struct subinfo	*sip ;
DISP		*dop ;
{
	PROGINFO	*pip ;

	ARGINFO	*aip ;

	int	rs = SR_OK ;
	int	ai ;
	int	f ;

	const char	*cp ;


	pip = sip->pip ;
	aip = sip->aip ;
	for (ai = 1 ; ai < aip->argc ; ai += 1) {

	    f = (ai <= aip->ai_max) && (bits_test(&aip->pargs,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = aip->argv[ai] ;
	    sip->pan += 1 ;
	    rs = subinfo_procfile(sip,dop,cp) ;
	    if (rs < 0)
	        break ;

	} /* end for */

	return rs ;
}
/* end subroutine (subinfo_args) */


static int subinfo_argfile(sip,dop)
struct subinfo	*sip ;
DISP		*dop ;
{
	PROGINFO	*pip ;
	ARGINFO		*aip ;
	bfile		argfile ;
	int		rs = SR_OK ;

	pip = sip->pip ;
	aip = sip->aip ;
	if ((aip->afname == NULL) || (aip->afname[0] == '\0'))
	    goto ret0 ;

	if (strcmp(aip->afname,"-") != 0) {
	    rs = bopen(&argfile,aip->afname,"r",0666) ;
	} else
	    rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	if (rs >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    cchar	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(&argfile,lbuf,llen)) > 0) {

	        len = rs ;
	        if (lbuf[len - 1] == '\n')
	            len -= 1 ;

	        lbuf[len] = '\0' ;
	        cp = lbuf ;

	        if ((cp[0] == '\0') || (cp[0] == '#'))
	            continue ;

	        sip->pan += 1 ;
	        rs = subinfo_procfile(sip,dop,cp) ;
	        if (rs < 0)
	            break ;

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
struct subinfo	*sip ;
DISP		*dop ;
{
	int	rs = SR_OK ;

	char	*cp ;


	if (sip->pan > 0)
	    goto ret0 ;

	cp = "-" ;
	sip->pan += 1 ;
	rs = subinfo_procfile(sip,dop,cp) ;

ret0:
	return rs ;
}
/* end subroutine (subinfo_stdin) */


static int subinfo_procfile(sip,dop,fname)
struct subinfo	*sip ;
DISP		*dop ;
const char	fname[] ;
{
	PROGINFO	*pip = sip->pip ;

	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 = SR_OK ;


/* ignore all files that start w/ a '-' character */

	if ((fname[0] == '-') && (fname[1] != '\0'))
	    goto ret0 ;

/* continue */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: processing file=%s\n",
	        pip->progname,fname) ;

	if (fname[0] != '-') {

	    rs1 = u_stat(fname,&sb) ;
	    if (rs1 >= 0)
	        rs1 = sperm(&sip->id,&sb,R_OK) ;

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

	int	rs = SR_OK ;
	int	rs1 ;
	int	size ;
	int	opts ;
	int	i ;


	if (dop == NULL)
	    return SR_FAULT ;

	if (wap == NULL)
	    return SR_FAULT ;

	wap->dop = dop ;
	pip = wap->pip ;

	memset(dop,0,sizeof(DISP)) ;
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

	rs = ptm_create(&dop->om,NULL) ;
	if (rs < 0)
	    goto bad4 ;

	wap->omp = &dop->om ;

	dop->n = pip->npar ;
	if (dop->n == 0) {
	    rs1 = getnprocessors(pip->envv,0) ;
	    dop->n = (rs1 >= 0) ? (rs1 + 1) : 1 ;
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
bad5:
	ptm_destroy(&dop->om) ;

bad4:
	vecobj_finish(&dop->tids) ;

bad3:
	psem_destroy(&dop->wq_sem) ;

bad2:
bad1:
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

	int	rs = SR_OK ;
	int	i ;


	if (dop == NULL)
	    return SR_FAULT ;

	dop->f_done = TRUE ;
	if (f_abort)
	    dop->f_exit = TRUE ;

	for (i = 0 ; i < dop->n ; i += 1)
	    psem_post(&dop->wq_sem) ;

	for (i = 0 ; vecobj_get(&dop->tids,i,&tidp) >= 0 ; i += 1) {
	    if (tidp == NULL) continue ;
	    uptjoin(*tidp,NULL) ;
	}

	ptm_destroy(&dop->om) ;

	vecobj_finish(&dop->tids) ;

	psem_destroy(&dop->wq_sem) ;

	fsi_finish(&dop->wq) ;

ret0:
	return rs ;
}
/* end subroutine (disp_finish) */


static int disp_addwork(dop,tagbuf,taglen)
DISP		*dop ;
const char	tagbuf[] ;
int		taglen ;
{
	int	rs = SR_OK ;


	rs = fsi_add(&dop->wq,tagbuf,taglen) ;

	if (rs >= 0)
	    rs = psem_post(&dop->wq_sem) ;

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

	int	rs = SR_OK ;
	int	rs1 ;
	int	c = 0 ;

	char	fname[MAXPATHLEN + 1] ;


	pip = wap->pip ;
	dop = wap->dop ;
	uptself(&tid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkkey/worker: starting tid=%u\n",tid) ;
#endif

	while (rs >= 0) {

	    while ((rs = psem_wait(&dop->wq_sem)) < 0) {
	        if ((rs != SR_AGAIN) && (rs != SR_INTR))
	            break ;
	    } /* end while */

	    if (rs < 0)
	        break ;

	    if (dop->f_exit)
	        break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("mkkey/worker: tid=%u wakeup\n",tid) ;
#endif

	    rs1 = fsi_remove(&dop->wq,fname,MAXPATHLEN) ;

	    if ((rs1 == SR_NOTFOUND) && dop->f_done)
	        break ;

	    if (rs1 > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("mkkey/worker: tid=%u fname=%s\n",
	                tid,fname) ;
#endif

	        rs = progkey(pip,wap->ofp,wap->omp,
	            wap->terms,wap->delimiter,wap->ignorechars,
	            fname) ;

	        if (rs > 0)
	            c += 1 ;

	    } /* end if (work to do) */

	    if (rs >= 0)
	        rs = rs1 ;

	} /* end while */

	if (rs >= 0) rs = c ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mkkey/worker: tid=%u ret rs=%d c=%u\n",tid,rs,c) ;
#endif

	return rs ;
}
/* end subroutine (worker) */


int progkey(pip,ofp,omp,terms,delimiter,ignorechars,fname)
PROGINFO	*pip ;
bfile		*ofp ;
PTM		*omp ;
uchar		terms[] ;
char		delimiter[], ignorechars[] ;
char		fname[] ;
{
	struct ustat	sb ;

	FIELD	fsb ;

	HDB	keydb ;

	bfile	infile, *ifp = &infile ;

	offset_t	offset, recoff ;

	int	rs ;
	int	rs1 ;
	int	len, dlen ;
	int	reclen ;
	int	sl, cl, ll, lo ;
	int	hashsize ;
	int	c, nk ;
	int	entries = 0 ;
	int	n = 0 ;
	int	f_start, f_ent, f_finish ;
	int	f_open = FALSE ;
	int	f_bol, f_eol ;

	uchar	bterms[32] ;

	const char	*sp, *cp ;

	char	lbuf[LINEBUFLEN + 1], *lp ;


#if	CF_DEBUG && 0
	if (DEBUGLEVEL(2)) {
	    int	i ;
	    debugprintf("progkey: ent file=%s\n",fname) ;
	    debugprintf("progkey: delimiter=>%s<\n",delimiter) ;
	    debugprintf("progkey: terms(%p)\n",terms) ;
	    for (i = 0 ; i < 256 ; i += 1) {
	        if (BATST(terms,i))
	            debugprintf("progkey: t=%02x\n",i) ;
	    }
	}
#endif /* CF_DEBUG */

	if (pip->f.optbible) {

	    memcpy(bterms,terms,32) ;

	    BACLR(bterms,':') ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progkey: cleared colon as term\n") ;
#endif

	} /* end if (special bible processing) */

	dlen = strlen(delimiter) ;

/* open the file that we are supposed to process */

	if ((fname[0] != '-') && (fname[0] != '\0')) {
	    rs = bopen(ifp,fname,"r",0666) ;
	} else
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progkey: continuing\n") ;
#endif

/* figure a default hash DB size based on the input file length */

	if (pip->f.wholefile) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progkey: wholefile\n") ;
#endif

	    if ((bcontrol(ifp,BC_STAT,&sb) >= 0) && (sb.st_size > 0))
	        hashsize = sb.st_size / 6 ;

	    else
	        hashsize = 1000 ;

	} else
	    hashsize = 20 ;

/* go to it, read the file line by line */

	f_start = pip->f.wholefile ;
	f_ent = FALSE ;
	f_finish = FALSE ;

	offset = 0 ;
	recoff = 0 ;
	reclen = 0 ;
	lo = 0 ;
	f_bol = TRUE ;
	while (rs >= 0) {

	    rs = breadline(ifp,(lbuf + lo),(LINEBUFLEN - lo)) ;
	    if (rs < 0)
		break ;

	    len = (lo + rs) ;
	    if (len == 0)
		break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progkey: line> %t", lbuf,len) ;
#endif

/* we ignore lo...ng lines entirely, but we try to resynchronize up */

	    lp = lbuf ;
	    ll = len ;
	    f_eol = (lp[ll - 1] == '\n') ;
	    if (! f_eol) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progkey: ignoring long line\n") ;
#endif

	        offset += len ;
	        while (((c = bgetc(ifp)) != SR_EOF) && (c != '\n')) {
	            offset += 1 ;
	            recoff += 1 ;
	        } /* end while */

	        lo = 0 ;
	        continue ;

	    } /* end if (discarding extra line input) */

	    lp[--ll] = '\0' ;

/* figure out where we start and/or end an entry */

	    if (! pip->f.wholefile) {

	        if (delimiter[0] == '\0') {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkey: delimit every line\n") ;
#endif

	            f_start = TRUE ;
	            f_finish = TRUE ;

	        } else if (CHAR_ISWHITE(delimiter[0])) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkey: delimit using blank lines\n") ;
#endif

	            sl = sfshrink(lp,ll,&sp) ;

	            if (sl == 0) {

	                if (f_ent)
	                    f_finish = TRUE ;

	            } else {

	                if (! f_ent)
	                    f_start = TRUE ;

	            }

	        } else {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkey: specified delimiter\n") ;
#endif

	            if (strncmp(lp,delimiter,dlen) == 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkey: got a delimiter\n") ;
#endif

	                if (f_ent)
	                    f_finish = TRUE ;

	            } else {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkey: regular line\n") ;
#endif

	                if (! f_ent)
	                    f_start = TRUE ;

	            }

	        } /* end if (delimiter cascade) */

	        if (f_finish) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkey: finishing off entry\n") ;
#endif

	            f_ent = FALSE ;
	            f_finish = FALSE ;
	            if (n > 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkey: closing a DB\n") ;
#endif

	                f_open = FALSE ;
	                reclen = (int) (offset - recoff) ;
	                rs = keysfinish(pip,&keydb,ofp,omp,
				fname,recoff,reclen) ;
	                nk = rs ;
	                if (nk > 0)
	                    entries += 1 ;

	            }

	        } /* end if (finishing entry) */

	    } /* end if (not whole file -- determining entry boundary) */

	    if ((rs >= 0) && f_start) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("progkey: starting off entry, offset=%ld\n",
	                offset) ;
#endif

	        f_start = FALSE ;
	        f_ent = TRUE ;

	        n = 0 ;
	        reclen = 0 ;
	        recoff = offset ;
	        if (! f_open) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkey: opening a DB\n") ;
#endif

	            f_open = TRUE ;
	            rs = keysstart(pip,&keydb,hashsize) ;

	        } /* end if (opening keys DB) */

	    } /* end if (starting entry) */

/* process this current input line if we are supposed to */

	    if ((rs >= 0) && (ll >= pip->minwordlen) && f_ent && 
	        (! ignoreline(lp,ll,ignorechars))) {

	        if ((rs = field_start(&fsb,lp,ll)) >= 0) {
		    int		ch ;
	            int		fl ;
	            int		f_first = FALSE ;
	            cchar	*fp ;

	            if (pip->f.optbible) {

	                fl = field_get(&fsb,bterms,&fp) ;

	                while ((fl > 0) && (fp[fl - 1] == ':'))
	                    fl -= 1 ;

	                f_first = (fl > 0) ;

	            } /* end if (special bible processing) */

/* loop on parsing words (which become keys) from the input line */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progkey: f_first=%u\n",f_first) ;
#endif

	            while ((rs >= 0) &&
	                (f_first || ((fl = field_word(&fsb,terms,&fp)) >= 0))) {

	                char	lowbuf[LOWBUFLEN + 1] ;


	                f_first = FALSE ;

/* remove apostrophes (single quotes) from the leading edge */

	                if (fl && (fp[0] == CH_SQUOTE)) {
	                    fp += 1 ;
	                    fl -= 1 ;
	                }

	                if (fl == 0) continue ;

/* abandon stuff that starts with weirdo characters (if any) */

			ch = MKCHAR(fp[0]) ;
	                if (! isalnumlatin(ch)) continue ;
	                if (fl > NATURALWORDLEN) continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkey: word=%t\n",fp,fl) ;
#endif

	                cp = fp ;
	                cl = fl ;
	                if (hasuc(fp,fl)) {
	                    int	ml = MIN(LOWBUFLEN,fl) ;
	                    cl = strwcpylc(lowbuf,fp,ml) - lowbuf ;
	                    cp = lowbuf ;
	                }

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkey: lowercase word=%t\n",
	                        cp,cl) ;
#endif

/* remove possible trailing single quote */

	                sl = sfword(cp,cl,&sp) ;

	                if (sl <= 0) continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("progkey: sfword() wl=%u wp=>%t<\n",
	                        sl,sp,sl) ;
#endif

#if	CF_EXTRAWORDS
	                {
	                    XWORDS	xw ;

	                    if ((rs = xwords_start(&xw,sp,sl)) >= 0) {
	                        int	i ;

	                        i = 0 ;
	                        while ((rs >= 0) &&
	                            ((cl = xwords_get(&xw,i++,&cp)) > 0)) {

	                            rs = procword(pip,&keydb,n,cp,cl) ;
	                            if (rs > 0)
	                                n += 1 ;

	                        } /* end while */

	                        xwords_finish(&xw) ;
	                    } /* end if */

	                } /* end block */
#else /* CF_EXTRAWORDS */

	                rs = procword(pip,&keydb,n,sp,sl) ;
	                if (rs > 0)
	                    n += 1 ;

#endif /* CF_EXTRAWORDS */

	                if (rs < 0) break ;
	            } /* end while (getting word fields from the line) */

	            field_finish(&fsb) ;
	        } /* end if */

	    } /* end if (we were in an entry) */

	    offset += len ;
	    reclen += len ;
	    if (rs < 0)
	        break ;

	    f_bol = f_eol ;

	} /* end while (looping reading lines) */

/* write out (finish off) the last (or only) entry */

	if (f_open) {

	    f_open = FALSE ;
	    reclen = (int) (offset - recoff) ;
	    rs1 = keysfinish(pip,&keydb,ofp,omp,fname,recoff,reclen) ;
	    nk = rs1 ;
	    if (rs >= 0) rs = rs1 ;
	    if (nk > 0)
	        entries += 1 ;

	} /* end if */

	bclose(ifp) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkey: ret rs=%d entries=%d\n",rs,entries) ;
#endif

	return (rs >= 0) ? entries : rs ;
}
/* end subroutine (progkey) */


/* which input lines are supposed to be ignored ? */
static int ignoreline(lbuf,ll,ignorechars)
const char	lbuf[] ;
const char	ignorechars[] ;
const int	ll ;
{

	if ((ignorechars != NULL) && (lbuf[0] == '%')) {

	    if (ll < 2)
	        return TRUE ;

	    if (strchr(ignorechars,lbuf[1]) != NULL)
	        return TRUE ;

	} /* end if */

	return FALSE ;
}
/* end subroutine (ignoreline) */


/* process a word */
static int procword(pip,keydbp,n,buf,buflen)
PROGINFO	*pip ;
HDB		*keydbp ;
int		n ;
const char	buf[] ;
int		buflen ;
{
	EIGENDB		*edbp = &pip->eigendb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

/* continue with regular checks */

	if ((pip->maxkeys > 0) && (n >= pip->maxkeys))
	    goto ret0 ;

	if ((buflen == 0) || (buflen > NATURALWORDLEN))
	    goto ret0 ;

	if (buflen < pip->minwordlen)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (buflen > 0)
	        debugprintf("progkey/procword: key=%t\n",buf,buflen) ;
	    else
	        debugprintf("progkey/procword: zero length field\n") ;
	}
#endif /* CF_DEBUG */

/* check if this word is in the eigenword database */

	rs1 = SR_NOTFOUND ;
	if (pip->eigenwords > 0) 
	    rs1 = eigendb_exists(edbp,buf,buflen) ;

	if (rs1 >= 0)
	    goto ret0 ;

	if ((pip->maxwordlen > 0) && (buflen > pip->maxwordlen))
	    buflen = pip->maxwordlen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkey/procword: buflen=%d\n",buflen) ;
#endif

	rs = keysadd(pip,keydbp,buf,buflen) ;
	f = (rs > 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkey/procword: keysadd() rs=%d\n",rs) ;
#endif

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progkey/procword: ret=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procword) */


