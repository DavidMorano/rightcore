/* main (testhdbstr) */

/* test the HDBSTRSTR object */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_SIMULATE	1		/* special test */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"hdbstr.h"

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* exported subroutines */


int main(int argc, const char **argv,const char **envv)
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		tmpfile, *tfp = &tmpfile ;

	HDBSTR		ht ;
	HDBSTR_CUR	c ;

	VECSTR		list ;

	uint	mo_start = 0 ;

	int	rs = SR_OK ;
	int	i, j, len ;
	int	vlen ;
	int	ll ;
	int	cl ;
	int	kl, vl ;
	int	f_next ;

	const char	*lp ;
	const char	*tp ;
	const char	*cp ;
	const char	*kp, *vp ;
	const char	*keyp, *valp ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	tmpfname_buf[MAXPATHLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	bopen(efp,BFILE_STDERR,"dwca",0666) ;


	vecstr_start(&list,0,0) ;

	rs = hdbstr_start(&ht,0) ;

#if	CF_DEBUGS
	debugprintf("main: hdbinit rs=%d\n",rs) ;
#endif


	bopen(ifp,BFILE_STDIN,"r",0666) ;

	tmpfname_buf[0] = '\0' ;
	if ((rs = bseek(ifp,0L,SEEK_CUR)) < 0) {
	    mktmpfile( tmpfname_buf, 0644, "/tmp/dbtestXXXXXXXX") ;
	    bopen(tfp,tmpfname_buf,"rwct",0644) ;
	    while ((rs = bcopyblock(ifp,tfp,LINEBUFLEN)) > 0) ;
	    bclose(ifp) ;
	    ifp = tfp ;
	} /* end if */


	bopen(ofp,BFILE_STDOUT,"wct",0666) ;

#if	CF_DEBUGS
	debugprintf("main: entered\n\n") ;
#endif

	while ((len = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	    if (lbuf[len - 1] == '\n') len -= 1 ;
	    lbuf[len] = '\0' ;

	    if ((tp = strnchr(lbuf,len,'#')) != NULL)
	        len = (tp-lbuf) ;

	    lp = lbuf ;
	    ll = len ;
	    if ((kl = nextfield(lp,ll,&kp)) > 0) {
		ll -= ((lp+ll)-(kp+kl)) ;
		lp = (kp+kl) ;
	        if ((vl = nextfield(lp,ll,&vp)) > 0) {

#if	CF_DEBUGS
	            debugprintf("main: k=%t v=%t\n",kp,kl,vp,vl) ;
#endif

	            vecstr_add(&list,kp,kl) ;

	            rs = hdbstr_add(&ht,kp,kl,vp,vl) ;

#if	CF_DEBUGS
	            debugprintf("main: hdbstore rs=%d\n",rs) ;
#endif

		} /* end if */
	     } /* end if */

	} /* end while (reading lines) */


#if	CF_DEBUGS
	debugprintf("main: fetch phase\n") ;
#endif

	bprintf(ofp,"fetching phase\n\n") ;

	for (i = 0 ; vecstr_get(&list,i,&lp) >= 0 ; i += 1) {
	    if (lp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("main: key=%s\n",lp) ;
#endif

	    if ((rs = hdbstr_curbegin(&ht,&c)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: hdbstr_fetch\n") ;
#endif

	    j = 0 ;
	    while (rs >= 0) {

	        vlen = hdbstr_fetch(&ht,lp,-1,&c,&valp) ;
		if (vlen == SR_NOTFOUND) break ;
		rs = vlen ;
		if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("main: hdbstr_fetch got one, rs=%d\n",rs) ;
	    debugprintf("main: hdbstr_fetch val=%W\n",valp,vlen) ;
#endif

	        bprintf(ofp,"j=%d key=%s data=%t\n", j,lp,valp,vlen) ;

	        j += 1 ;

#if	CF_DEBUGS
	    debugprintf("main: bot loop, j=%d\n",j) ;
#endif

	    } /* end while */

	    hdbstr_curend(&ht,&c) ;
	    } /* end if (cursor) */

	    if (j <= 0)
	        bprintf(ofp,"could not find key=%s\n",lp) ;

	} /* end for (list enumeration) */

	bprintf(ofp,"\n") ;

#if	CF_DEBUGS
	debugprintf("main: enumeration phase\n") ;
#endif

	bprintf(ofp,"enumeration\n") ;

	bprintf(ofp,"\n") ;

/* enumerate all entries */

	if ((rs = hdbstr_curbegin(&ht,&c)) >= 0) {

	    while (hdbstr_enum(&ht,&c,&keyp,&valp,&vlen) >= 0) {

	        bprintf(ofp,"key=%s data=%s vlen=%d\n",
	            keyp,valp,vlen) ;

	    } /* end while */

	    hdbstr_curend(&ht,&c) ;
	} /* end if (cursor) */

	bprintf(ofp,"\n") ;

/* delete entries with certain values */

#if	CF_DEBUGS
	debugprintf("main: deletion test\n") ;
#endif

	bprintf(ofp,"deletion (of all '1's)\n") ;

	bprintf(ofp,"\n") ;

	if ((rs = hdbstr_curbegin(&ht,&c)) >= 0) {

	f_next = TRUE ;
	while (rs >= 0) {

#if	CF_SIMULATE
	    if (f_next) {
	        rs = hdbstr_enum(&ht,&c,&keyp,&valp,&vlen) ;
	    } else
	        rs = hdbstr_getrec(&ht,&c,&keyp,&valp,&vlen) ;
#else
	        rs = hdbstr_getrec(&ht,&c,&keyp,&valp,&vlen) ;
#endif /* CF_SIMULATE */

	    if (rs < 0) 
		break ;

	    bprintf(ofp,"key=%s data=%s\n",
	        keyp,valp) ;

	    f_next = TRUE ;
	    if (strncmp(valp,"1",1) == 0) {

	        f_next = FALSE ;
	        hdbstr_delcur(&ht,&c,1) ;

	    } /* end if (deleted) */

#if	(! CF_SIMULATE)
	    if (f_next)
		hdbstr_next(&ht,&c) ;
#endif

	} /* end while */

	hdbstr_curend(&ht,&c) ;
	} /* end if (cursor) */

	bprintf(ofp,"\n") ;

/* loop freeing all hash DB user data */

#if	CF_DEBUGS
	debugprintf("main: cleanup phase\n") ;
#endif

	bprintf(ofp,"cleanup\n") ;

	bprintf(ofp,"\n") ;

	if ((rs = hdbstr_curbegin(&ht,&c)) >= 0) {

	while (hdbstr_enum(&ht,&c,&keyp,&valp,&vlen) >= 0) {

	    bprintf(ofp,"key=%s data=%s\n",
	        keyp,valp) ;

	} /* end while */

	hdbstr_curend(&ht,&c) ;
	} /* end if (cursor) */

	bprintf(ofp,"\n") ;

	hdbstr_finish(&ht) ;

	vecstr_finish(&list) ;

	bclose(ifp) ;

	bclose(ofp) ;


	bclose(efp) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return EX_OK ;
}
/* end subroutine (main) */



