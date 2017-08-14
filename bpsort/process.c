/* process */

/* process a dbfile */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 2000-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine processes a file that contains branch-predictor
	results.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */

#include	<vsystem.h>
#include	<baops.h>
#include	<vecitem.h>
#include	<bfile.h>
#include	<field.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	cfdecf(const char *,int,double *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int cmpconfig(struct entry **,struct entry **) ;


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


int process(PROGINFO *pip,vecitem *dbp,cchar *fname)
{
	struct entry	e ;

	bfile	ifile ;

	int	rs, c ;
	int	len ;
	int	line ;
	int	cl ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	namebuf[MAXNAMELEN + 1] ;
	char	*cp ;

	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: entered fname=\"%s\"\n",fname) ;
#endif

	rs = bopen(&ifile,fname,"r",0666) ;

	if (rs < 0)
	    return rs ;

/* read 'em */

	c = 0 ;
	line = 1 ;
	while ((len = breadline(&ifile,lbuf,LINEBUFLEN)) > 0) {
	   FIELD	lf ;

	    if (lbuf[len - 1] == '\n') len -= 1 ;
	    lbuf[len] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: line=>%t<\n",
	            lbuf,len) ;
#endif /* CF_DEBUG */

	    field_start(&lf,lbuf,len) ;

	    (void) memset(&e,0,sizeof(struct entry)) ;

	    e.line = line ;
	    e.fname = mallocstr(fname) ;

/* program name */

	    if ((rs = field_get(&lf,NULL,NULL)) > 0) {
	        e.progname = mallocstrw(lf.fp,lf.fl) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("process: progname=%s\n",e.progname) ;
#endif /* CF_DEBUG */

/* accuracy */

	    if (rs >= 0) {

	        rs = field_get(&lf,NULL,NULL) ;

	        rs = SR_INVALID ;
	        if (lf.fl > 0)
	            rs = cfdecf(lf.fp,lf.fl,&e.accuracy) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: accuracy=%8.3f\n",e.accuracy) ;
#endif /* CF_DEBUG */

	    }

/* predictor name */

	    if (rs >= 0) {

	        rs = field_get(&lf,NULL,NULL) ;

	        if (lf.fl > 0)
	            e.bpname = mallocstrw(lf.fp,lf.fl) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: bpname=%s\n",e.bpname) ;
#endif /* CF_DEBUG */

	    }

/* predictor bits (if present) */

	    if (rs >= 0) {

	        field_get(&lf,NULL,NULL) ;

	        if (lf.fl > 0) {

	            rs = cfdecui(lf.fp,lf.fl,&e.bits) ;

			if ((rs >= 0) && (e.bits == 0))
				e.bits = -1 ;
		}

	    }


/* predictor parameters (up to four of them) */

	    if (rs >= 0) {

	        rs = field_get(&lf,NULL,NULL) ;

	        if (lf.fl > 0)
	            rs = cfdecui(lf.fp,lf.fl,&e.p1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: p1=%u\n",e.p1) ;
#endif /* CF_DEBUG */

	    }

	    if (rs >= 0) {

	        field_get(&lf,NULL,NULL) ;

	        if (lf.fl > 0)
	            rs = cfdecui(lf.fp,lf.fl,&e.p2) ;

	    }

	    if (rs >= 0) {

	        field_get(&lf,NULL,NULL) ;

	        if (lf.fl > 0)
	            rs = cfdecui(lf.fp,lf.fl,&e.p3) ;

	    }

	    if (rs >= 0) {

	        field_get(&lf,NULL,NULL) ;

	        if (lf.fl > 0)
	            rs = cfdecui(lf.fp,lf.fl,&e.p4) ;

	    }

/* we're out of this one */

	    field_finish(&lf) ;

	    if ((rs >= 0) && (e.progname != NULL) && (e.bpname != NULL)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: vecitem_add() progname=%s bpname=%s\n",
	                e.progname,e.bpname) ;
#endif /* CF_DEBUG */

	        rs = SR_NOANODE ;
	        if (vecitem_search(dbp,&e,cmpconfig,NULL) < 0) {
	            rs = vecitem_add(dbp,&e,sizeof(struct entry)) ;

		} else {
			bprintf(pip->efp,
			"%s: duplicate configuration prog=%s pred=%s\n",
			pip->progname,e.progname,e.bpname) ;
		}

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: vecitem_add() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	    }

	    if (rs < 0) {

	        if (e.fname != NULL)
	            uc_free(e.fname) ;

	        if (e.progname != NULL)
	            uc_free(e.progname) ;

	        if (e.bpname != NULL)
	            uc_free(e.bpname) ;

	    } else
	        c += 1 ;

	    line += 1 ;

	} /* end while (reading lines) */

	bclose(&ifile) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: ret=%d\n",c) ;
#endif /* CF_DEBUG */

	return c ;
}
/* end subroutine (process) */


/* local subroutines */


static int cmpconfig(e1pp,e2pp)
struct entry	**e1pp, **e2pp ;
{
	int	diff ;


#if	F_NULLENTRY
	if (((*e1pp) == NULL) && ((*e2pp) == NULL))
	    return 0 ;

	else if ((*e1pp) == NULL)
	    return 1 ;

	else if ((*e2pp) == NULL)
	    return -1 ;
#endif /* F_NULLENTRY */

	diff = strcasecmp((*e1pp)->progname,(*e2pp)->progname) ;

	if (diff != 0)
	    return diff ;

	diff = strcasecmp((*e1pp)->bpname,(*e2pp)->bpname) ;

	if (diff != 0)
	    return diff ;

	diff = (*e1pp)->p1 - (*e2pp)->p1 ;
	if (diff != 0)
	    return diff ;

	diff = (*e1pp)->p2 - (*e2pp)->p2 ;
	if (diff != 0)
	    return diff ;

	diff = (*e1pp)->p3 - (*e2pp)->p3 ;
	if (diff != 0)
	    return diff ;

	diff = (*e1pp)->p4 - (*e2pp)->p4 ;
	return diff ;
}
/* end subroutine (cmpconfig) */


