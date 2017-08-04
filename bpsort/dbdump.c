/* dbdump */

/* dump out the stuff in the DB */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_NULLENTRY	1


/* revision history:

	= 2000-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine sorts and dumps out the stuff in the DB.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<vecitem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<field.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"yags.h"
#include	"bpalpha.h"
#include	"gspag.h"
#include	"gskew.h"
#include	"tourna.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))

#ifndef	LINELEN
#define	LINELEN		100
#endif

#define	NHMS		100


/* external subroutines */

extern int	matostr(cchar **,int,cchar *,int) ;

extern char	*strwcpy(char *,cchar *,int) ;

extern double	fhm(double *,int) ;


/* external variables */


/* global variables */


/* local structures */

struct bpconfig {
	uint	bits ;
	uint	p1 ;
} ;


/* forward references */

static int	cmpbits(struct entry **,struct entry **) ;
static int	cmpconfig(struct bpconfig **,struct bpconfig **) ;
static int	cmpname(char **,char **) ;


/* local variables */

static cchar	*bpnames[] = {
	"yags",
	"bpalpha",
	"gspag",
	"gskew",
	"tourna",
	NULL
} ;

enum bpnames {
	bpname_yags,
	bpname_bpalpha,
	bpname_gspag,
	bpname_gskew,
	bpname_tourna,
	bpname_overlast
} ;


/* exported subroutines */


int dbdump(pip,dbp)
PROGINFO	*pip ;
vecitem		*dbp ;
{
	struct entry	*ep ;

	struct bpconfig	bpc ;

	FIELD	lf ;

	bfile	predfile ;

	vecstr	prognames ;
	vecitem	configs ;

	int	rs, rs1 ;
	int	kwi ;
	int	i, j, k ;
	int	cl ;

	cchar	*cp ;
	char	progname[MAXNAMELEN + 1] ;


	vecstr_start(&prognames,20,VECSTR_PSORTED) ;

	vecitem_start(&configs,20,VECITEM_PSORTED) ;

/* calculate bits for each instance of a known predictor */

	for (i = 0 ; vecitem_get(dbp,i,&ep) >= 0 ; i += 1) {

	    if (ep == NULL)
	        continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("dbdump: i=%d progname=%s\n",i,ep->progname) ;
#endif

/* store the program name, if it is not already known */

	    cl = strwcpy(progname,ep->progname,-1) - progname ;

	    rs = vecstr_search(&prognames,progname,NULL,NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("dbdump: search=%s vecstr_search() rs=%d\n",
	            progname,rs) ;
#endif

	    if (rs < 0) {

	        rs1 = vecstr_add(&prognames,progname,cl) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("dbdump: vecstr_add() rs=%d\n",rs1) ;
#endif

	    }

/* calculate the bits */

	    kwi = matostr(bpnames,3,ep->bpname,-1) ;

	    if (kwi < 0)
	        continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("dbdump: found bpname=%s(%d)\n",
	            bpnames[kwi],kwi) ;
#endif

	    switch (kwi) {

	    case bpname_yags:
	        {
	            YAGS		pred ;

	            YAGS_STATS	s ;


	            rs = yags_init(&pred,ep->p1,ep->p2) ;

	            if (rs >= 0) {

	                yags_stats(&pred,&s) ;

	                ep->bits = s.bits ;
	                yags_free(&pred) ;

	            } else
	                bprintf(pip->efp,
	                    "%s: bad predictor (%d) file=%s:%d\n",
	                    pip->progname,rs,
	                    ep->fname,
	                    ep->line) ;

	        }

	        break ;

	    case bpname_bpalpha:
	        {
	            BPALPHA		pred ;

	            BPALPHA_STATS	s ;


	            rs = bpalpha_init(&pred,ep->p1,ep->p2,ep->p3) ;

	            if (rs >= 0) {

	                bpalpha_stats(&pred,&s) ;

	                ep->bits = s.bits ;
	                bpalpha_free(&pred) ;

	            } else
	                bprintf(pip->efp,
	                    "%s: bad predictor (%d) file=%s:%d\n",
	                    pip->progname,rs,
	                    ep->fname,
	                    ep->line) ;

	        }

	        break ;

	    case bpname_gspag:
	        {
	            GSPAG		pred ;

	            GSPAG_STATS	s ;


	            rs = gspag_init(&pred,ep->p1,ep->p2) ;

	            if (rs >= 0) {

	                gspag_stats(&pred,&s) ;

	                ep->bits = s.bits ;
	                gspag_free(&pred) ;

	            } else
	                bprintf(pip->efp,
	                    "%s: bad predictor (%d) file=%s:%d\n",
	                    pip->progname,rs,
	                    ep->fname,
	                    ep->line) ;

	        }

	        break ;

	    case bpname_gskew:
	        {
	            GSKEW		pred ;

	            GSKEW_STATS		s ;


	            rs = gskew_init(&pred,ep->p1,ep->p2,ep->p3,ep->p4) ;

	            if (rs >= 0) {

	                gskew_stats(&pred,&s) ;

	                ep->bits = s.bits ;
	                gskew_free(&pred) ;

	            } else
	                bprintf(pip->efp,
	                    "%s: bad predictor (%d) file=%s:%d\n",
	                    pip->progname,rs,
	                    ep->fname,
	                    ep->line) ;

	        }

	        break ;

	    case bpname_tourna:
	        {
	            TOURNA		pred ;

	            TOURNA_STATS	s ;


	            rs = tourna_init(&pred,ep->p1,ep->p2,ep->p3) ;

	            if (rs >= 0) {

	                tourna_stats(&pred,&s) ;

	                ep->bits = s.bits ;
	                tourna_free(&pred) ;

	            } else
	                bprintf(pip->efp,
	                    "%s: bad predictor (%d) file=%s:%d\n",
	                    pip->progname,rs,
	                    ep->fname,
	                    ep->line) ;

	        }

	        break ;

	    } /* end switch */

/* store this particular configuration, if we do not already have it */

	    bpc.bits = ep->bits ;
	    bpc.p1 = ep->p1 ;
	    if (vecitem_search(&configs,&bpc,cmpconfig,NULL) < 0) {

	        rs1 = vecitem_add(&configs,&bpc,sizeof(struct bpconfig)) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("dbdump: vecitem_add() rs=%d bits=%u p1=%u\n",
	                rs1,bpc.bits,bpc.p1) ;
#endif

	    }

	} /* end for */

/* sort the program names */

	vecstr_sort(&prognames,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("dbdump: prognames\n") ;
	    for (i = 0 ; vecstr_get(&prognames,i,&cp) >= 0 ; i += 1) {
	        debugprintf("dbdump: progname=%s\n",cp) ;
	    }
	}
#endif /* CF_DEBUG */

/* sort everything by bits */

	vecitem_sort(dbp,cmpbits) ;


/* print stuff out */

	for (j = 0 ; bpnames[j] != NULL ; j += 1) {
	    struct bpconfig	*cep ;
	    bfile	cfile ;
	    double	hms[NHMS + 1] ;
	    int		ci ;
	    cchar	*np ;
	    char	tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("dbdump: predictor=%s\n",bpnames[j]) ;
#endif

	    bufprintf(tmpfname,MAXPATHLEN,"%s_hm.txt",
	        bpnames[j]) ;

	    bopen(&cfile,tmpfname,"wct",0666) ;

	    for (ci = 0 ; vecitem_get(&configs,ci,&cep) >= 0 ; ci += 1) {

	        double	hm ;

	        uint	hmbits ;

	        int	hmi ;


#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("dbdump: config bits=%u p1=%u\n",
	                cep->bits,cep->p1) ;
#endif

	        hmi = 0 ;
	        hmbits = 0 ;
	        for (k = 0 ; vecstr_get(&prognames,k,&np) >= 0 ; k += 1) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("dbdump: progname=%s\n",np) ;
#endif

	            bufprintf(tmpfname,MAXPATHLEN,"%s_%s.txt",
	                bpnames[j],np) ;

	            rs = bopen(&predfile,tmpfname,"wca",0666) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("dbdump: bopen() rs=%d fname=%s\n",rs,tmpfname) ;
#endif

	            for (i = 0 ; vecitem_get(dbp,i,&ep) >= 0 ; i += 1) {

	                if (ep == NULL)
	                    continue ;

	                if (strcasecmp(ep->bpname,bpnames[j]) != 0)
	                    continue ;

	                if ((cep->bits != ep->bits) || (cep->p1 != ep->p1))
	                    continue ;

	                if (strcasecmp(ep->progname,np) != 0)
	                    continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("dbdump: hmbits=%u hmi=%d\n",hmbits,hmi) ;
#endif

	                if (hmbits == 0)
	                    hmbits = ep->bits ;

	                if (hmi < NHMS)
	                    hms[hmi++] = ep->accuracy ;

	                bprintf(&predfile,"%12u %7.3f\n",
	                    ep->bits,ep->accuracy) ;

	            } /* end for (data) */

	            bclose(&predfile) ;

	        } /* end for (program names) */

		if (hmi > 0) {

	        hm = fhm(hms,hmi) ;

	        bprintf(&cfile,"%12u %7.3f\n",
	            hmbits,hm) ;

		}

	    } /* end for (configs) */

	    bclose(&cfile) ;

	} /* end for (b-predictors) */


	vecitem_finish(&configs) ;

	vecstr_finish(&prognames) ;

	return rs ;
}
/* end subroutine (dbdump) */



/* LOCAL SUBROUTINES */



static int cmpbits(e1pp,e2pp)
struct entry	**e1pp, **e2pp ;
{
	int	diff ;


#if	CF_DEBUGS
	if ((*e1pp) == NULL)
	    debugprintf("cmpbits: got_one *e1pp=%p\n",(*e1pp)) ;
	if ((*e2pp) == NULL)
	    debugprintf("cmpbits: got_one *e2pp=%p\n",(*e2pp)) ;
#endif

#if	CF_NULLENTRY
	if (((*e1pp) == NULL) && ((*e2pp) == NULL))
	    return 0 ;

	else if ((*e1pp) == NULL)
	    return 1 ;

	else if ((*e2pp) == NULL)
	    return -1 ;
#endif /* CF_NULLENTRY */

	diff = (*e1pp)->bits - (*e2pp)->bits ;

	if (diff < 0)
	    return -1 ;

	if (diff > 0)
	    return 1 ;

	return (*e1pp)->p1 - (*e2pp)->p1 ;
}
/* end subroutine (cmpbits) */


static int cmpconfig(e1pp,e2pp)
struct bpconfig	**e1pp, **e2pp ;
{
	int	diff ;


#if	CF_NULLENTRY
	if (((*e1pp) == NULL) && ((*e2pp) == NULL))
	    return 0 ;

	else if ((*e1pp) == NULL)
	    return 1 ;

	else if ((*e2pp) == NULL)
	    return -1 ;
#endif /* CF_NULLENTRY */

	diff = (*e1pp)->bits - (*e2pp)->bits ;

	if (diff < 0)
	    return -1 ;

	if (diff > 0)
	    return 1 ;

	return (*e1pp)->p1 - (*e2pp)->p1 ;
}
/* end subroutine (cmpconfig) */


/* compare program names */
static int cmpname(e1pp,e2pp)
char	**e1pp, **e2pp ;
{


#if	CF_DEBUGS
	if ((*e1pp) == NULL)
	    debugprintf("cmpname: got_one *e1pp=%p\n",(*e1pp)) ;
	if ((*e2pp) == NULL)
	    debugprintf("cmpname: got_one *e2pp=%p\n",(*e2pp)) ;
#endif

#if	CF_NULLENTRY
	if (((*e1pp) == NULL) && ((*e2pp) == NULL))
	    return 0 ;

	else if ((*e1pp) == NULL)
	    return 1 ;

	else if ((*e2pp) == NULL)
	    return -1 ;
#endif /* CF_NULLENTRY */

	return strcasecmp(*e1pp,*e2pp) ;
}
/* end subroutine (cmpname) */


