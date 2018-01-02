/* progquery */

/* make a query */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG 	0		/* run-time debug print-outs */
#define	CF_READSTDIN	0		/* read STDIN failing arguments */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a single file.

	Synopsis:

	int progquery(pip,aip,terms,dbname,ofname)
	PROGINFO	*pip ;
	ARGINFO		*aip ;
	const uchar	terms[] ;
	const uchar	dbname[] ;
	char		ofname[] ;

	Arguments:

	- pip		program information pointer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>
#include	<textlook.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	OUTBUFLEN	(MAXPATHLEN + 50)
#define	WPBUFLEN	(20 * 100)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfcasesub(const char *,int,const char *,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	mkexpandpath(char *,const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_adds(vecstr *,const char *,int) ;
extern int	field_wordphrase(FIELD *,const uchar *,char *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUG || CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern int	progexit(PROGINFO *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	procdb(PROGINFO *,ARGINFO *,bfile *,cchar *) ;

static int	procspecs(PROGINFO *,bfile *,TEXTLOOK *,cchar *,int) ;
static int	procspec(PROGINFO *,bfile *,TEXTLOOK *,vecstr *) ;

static int	procout(PROGINFO *,bfile *,TEXTLOOK_TAG *) ;


/* local variables */

static const uchar	aterms[] = {
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


int progquery(PROGINFO *pip,ARGINFO *aip,const uchar *terms,
		cchar *dbname,cchar *ofname)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf( "progquery: dbname=%s\n",dbname) ;
	    debugprintf( "progquery: ofname=%s\n",ofname) ;
	}
#endif /* CF_DEBUG */

	if (terms == NULL) return SR_FAULT ;

	if (dbname == NULL) {
	    rs = SR_FAULT ;
	    fmt = "%s: no database specified (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}

	if ((rs >= 0) && (dbname[0] == '\0')) {
	    rs = SR_INVALID ;
	    fmt = "%s: no database specified (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}

/* open the output file */

	if ((ofname == NULL) || (ofname[0] == '\0')) 
		ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

	    rs = procdb(pip,aip,ofp,dbname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf( "progquery: procdb() rs=%d\n",rs) ;
#endif

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file) */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    bprintf(pip->efp,"%s: matches=%u\n",pip->progname,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf( "progquery: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progquery) */


/* local subroutines */


static int procdb(PROGINFO *pip,ARGINFO *aip,bfile *ofp,cchar *dbname)
{
	TEXTLOOK	tl, *tlp = &tl ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		c_hits = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	char		tmpfname[MAXPATHLEN+1] ;

	if ((rs = mkexpandpath(tmpfname,dbname,-1)) >= 0) {
	    cchar	*bdn = pip->basedname ;
	    if (rs > 0) dbname = tmpfname ;
	    if ((rs = textlook_open(tlp,pip->pr,dbname,bdn)) >= 0) {
		vecstr	qstr ;
	        int	opts ;
	        int	ai ;
	        int	f ;
		cchar	*afn = aip->afname ;
		cchar	*cp ;

	        if (pip->f.optaudit) {
	            rs = textlook_audit(tlp) ;
	            if ((rs < 0) || (pip->debuglevel > 0)) {
	                bprintf(pip->efp, "%s: DB audit (%d)\n",
	                    pip->progname,rs) ;
	            }
	        } /* end if (audit) */

/* process the positional arguments */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf( "progquery/procdb: keys as args\n") ;
#endif

	        opts = VECSTR_OCOMPACT ;
	        if ((rs >= 0) && ((rs = vecstr_start(&qstr,5,opts)) >= 0)) {
	            BITS	*bop = &aip->pargs ;
	            int		c = 0 ;
		    cchar	**argv = aip->argv ;

	            for (ai = 1 ; ai < aip->argc ; ai += 1) {

	                f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	                f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	                if (f) {
	                    cp = argv[ai] ;
			    if (cp[0] != '\0') {
	                        pan += 1 ;
	                        rs = vecstr_adduniq(&qstr,cp,-1) ;
				if (rs < INT_MAX) c += 1 ;
			    }
	                }

	                if (rs >= 0) rs = progexit(pip) ;
	                if (rs < 0) break ;
	            } /* end for (looping through positional arguments) */

	            if ((rs >= 0) && (c > 0)) {
	                if ((rs = vecstr_count(&qstr)) > 0) {
	                    rs = procspec(pip,ofp,tlp,&qstr) ;
	                    c_hits += rs ;
	                }
	            } /* end if */

	            rs1 = vecstr_finish(&qstr) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (positional arguments) */

/* process any files in the argument filename list file */

	        if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	            bfile	afile, *afp = &afile ;

	            if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	            if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                int		len ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                    len = rs ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("progquery/procdb: line=>%t<\n",
	                            lbuf,strlinelen(lbuf,len,60)) ;
#endif

	                    rs = procspecs(pip,ofp,tlp,lbuf,len) ;
	                    c_hits += rs ;

	                    if (rs >= 0) rs = progexit(pip) ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                rs1 = bclose(afp) ;
	                if (rs >= 0) rs = rs1 ;
	            } else {
	                fmt = "%s: inaccessible argument-list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            } /* end if */

	        } /* end if (argument file) */

#if	CF_READSTDIN
	        if ((rs >= 0) && (pan == 0)) {
	            bfile	infile, *ifp = &infile ;

	            if ((rs = bopen(ifp,BFILE_STDIN,"dr",0666)) >= 0) {
	                const int	llen = LINEBUFLEN ;
	                int		len ;
	                char		lbuf[LINEBUFLEN + 1] ;

	                while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	                    int	len = rs ;

	                    rs = procspecs(pip,ofp,tlp,lbuf,len) ;
	                    c_hits += rs ;

	                    if (rs >= 0) rs = progexit(pip) ;
	                    if (rs < 0) break ;
	                } /* end while (reading lines) */

	                bclose(ifp) ;
	            } else {
	                fmt = "%s: inaccessible input list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	            } /* end if (error) */

	        } /* end if (file list arguments or not) */
#endif /* CF_READSTDIN */

	        rs1 = textlook_close(tlp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (textlook) */
	} /* end if (expand) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progquery/procdb: ret rs=%d c_hits=%u\n",
	        rs,c_hits) ;
#endif

	return (rs >= 0) ? c_hits : rs ;
}
/* end subroutine (procdb) */


static int procspecs(pip,ofp,tlp,sp,sl)
PROGINFO	*pip ;
bfile		*ofp ;
TEXTLOOK	*tlp ;
const char	*sp ;
int		sl ;
{
	VECSTR		qstr ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (sp == NULL) return SR_FAULT ;

	if ((rs = vecstr_start(&qstr,5,0)) >= 0) {
	    FIELD	fsb ;

	    if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	        const uchar	*at = aterms ;
	        const int	wlen = WPBUFLEN ;
	        int		fl ;
	        cchar		*fp ;
	        char		wbuf[WPBUFLEN + 1] ;

	        fp = wbuf ;
	        while ((fl = field_wordphrase(&fsb,at,wbuf,wlen)) >= 0) {
	            if (fl > 0) {
	                rs = vecstr_adduniq(&qstr,fp,fl) ;
	                c += ((rs < INT_MAX) ? 1 : 0) ;
	            }
	            if (fsb.term == '#') break ;
	            if (rs < 0) break ;
	        } /* end while */

	        field_finish(&fsb) ;
	    } /* end if (field) */

	    if ((rs >= 0) && (c > 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            int		i ;
	            cchar	*fp ;
	            for (i = 0 ; vecstr_get(&qstr,i,&fp) >= 0 ; i += 1)
	                debugprintf("progquery/procspecs: qs=>%s<\n",fp) ;
	        }
#endif /* CF_DEBUG */

	        rs = procspec(pip,ofp,tlp,&qstr) ;

	    } /* end if */

	    rs1 = vecstr_finish(&qstr) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (queries) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


static int procspec(pip,ofp,tlp,qsp)
PROGINFO	*pip ;
bfile		*ofp ;
TEXTLOOK	*tlp ;
vecstr		*qsp ;
{
	TEXTLOOK_CUR	cur ;
	TEXTLOOK_TAG	tag ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ntags ;
	int		qopts = 0 ;
	int		c = 0 ;
	const char	**qkeya ;

	if (qsp == NULL) return SR_FAULT ;

	if (pip->f.prefix) qopts |= TEXTLOOK_OPREFIX ;

	if ((rs = vecstr_getvec(qsp,&qkeya)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        int	i ;
	        for (i = 0 ; qkeya[i] != NULL ; i += 1)
	            debugprintf("progquery/procspec: sk=>%s<\n",qkeya[i]) ;
	    }
#endif /* CF_DEBUG */

	    if ((rs = textlook_curbegin(tlp,&cur)) >= 0) {

	        rs = textlook_lookup(tlp,&cur,qopts,qkeya) ;
	        ntags = rs ;

	        while ((rs >= 0) && (ntags > 0)) {
	            rs1 = textlook_read(tlp,&cur,&tag) ;
	            if (rs1 == SR_NOTFOUND) break ;
	            rs = rs1 ;

	            if (rs >= 0) {
	                c += 1 ;
	                rs = procout(pip,ofp,&tag) ;
	            }

	            if (rs >= 0) rs = progexit(pip) ;
	            if (rs < 0) break ;
	        } /* end while */

	        textlook_curend(tlp,&cur) ;
	    } /* end if (cursor) */

	} /* end if (vecstr_getvec) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf( "progquery/procspec: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspec) */


static int procout(pip,ofp,tagp)
PROGINFO	*pip ;
bfile		*ofp ;
TEXTLOOK_TAG	*tagp ;
{
	uint		recoff ;
	uint		reclen ;
	const int	olen = OUTBUFLEN ;
	int		rs = SR_OK ;
	int		len ;
	int		wlen = 0 ;
	const char	*fmt ;
	const char	*fn ;
	char		obuf[OUTBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	fmt = "%s:%u,%u\n" ;
	fn = tagp->fname ;
	recoff = tagp->recoff ;
	reclen = tagp->reclen ;

	if ((rs = bufprintf(obuf,olen,fmt,fn,recoff,reclen)) >= 0) {
	    len = rs ;
	    rs = bprintf(ofp,"%t",obuf,len) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


