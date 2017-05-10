/* main (testhdb) */

/* test the HDB object */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_DEBUGENUM	0
#define	CF_CMPVALUE	1
#define	CF_ENUM1	1
#define	CF_ENUM2	1


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This program is used to debug the HDB object.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<mallocstuff.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"hdb.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#if	CF_DEBUGS
#define	DEFENTRIES	250
#else
#define	DEFENTRIES	250000
#endif


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procfile(PROGINFO *,VECSTR *,HDB *,bfile *,cchar *) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	HDB		ht ;
	HDB_DATUM	key, value, data ;
	HDB_CUR		cur ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	vecstr		strdb ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs, rs1, rs2 ;
	int		ai ;
	int		i ;
	int		sl, cl ;
	int		c ;
	int		ex = EX_INFO ;
	const char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else {
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	}
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif

	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badout ;
	}

	rs = vecstr_start(&strdb,DEFENTRIES,0) ;

	bprintf(ofp,"INIT VECSTR rs=%d\n",rs) ;

	if (rs < 0)
	    goto ret1 ;

	rs = hdb_start(&ht,DEFENTRIES,TRUE,NULL,NULL) ;

#if	CF_DEBUGS
	debugprintf("main: hdb_start() rs=%d\n",rs) ;
#endif

	bprintf(ofp,"INIT HDB rs=%d\n",rs) ;

	if (rs < 0)
	    goto ret2 ;

/* read all files */

	bprintf(ofp,"READING\n") ;

	c = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    cp = argv[ai] ;
	    if ((cp != NULL) && (cp[0] != '\0')) {

	        rs = procfile(pip,&strdb,&ht,ofp,cp) ;
	        c += rs ;
	        if (rs < 0)
	            break ;

	    } /* end if (got argument) */

	} /* end for (reading files) */

	bprintf(ofp,"READING rs=%d c=%u\n",rs,c) ;

	if (rs < 0)
	    goto ret3 ;

/* check counts */

	if (rs >= 0) {

	    rs1 = vecstr_count(&strdb) ;

	    rs2 = hdb_count(&ht) ;

#if	CF_DEBUGS
	    debugprintf("main: counts=%u:%u\n",
	        rs1,rs2) ;
#endif

	    if (rs1 != rs2)
	        rs = SR_BADFMT ;

	    bprintf(ofp,"COUNTS rs=%d vecstr=%u hdb=%u\n",
		rs,rs1,rs2) ;

	}

	if (rs < 0)
	    goto ret3 ;

/* verify that all words are present in the HDB object */

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: FETCH phase\n") ;
#endif

	    c = 0 ;
	    for (i = 0 ; vecstr_get(&strdb,i,&sp) >= 0 ; i += 1) {
	        if (sp == NULL) continue ;

	        key.buf = sp ;
	        key.len = strlen(sp) ;

	        rs = hdb_fetch(&ht,key,NULL,&value) ;
	        if (rs < 0)
	            break ;

#if	CF_DEBUGS && CF_CMPVALUE
	        cp = (char *) value.buf ;
	        cl = value.len ;
	        if (cl != key.len) {
	            debugprintf("main: CMP LEN i=%u mismatch\n",i) ;
	            rs = SR_BADFMT ;
	            break ;
	        }

	        if (strcmp(sp,cp) != 0) {
	            debugprintf("main: CMP VAL i=%u mismatch\n",i) ;
	            rs = SR_BADFMT ;
	            break ;
	        }
#endif /* CF_CMPVALUE */

	        c += 1 ;

	    } /* end for */

#if	CF_DEBUGS
	    if (rs < 0)
	        debugprintf("main: FETCH failure rs=%d\n",rs) ;
#endif

	    bprintf(ofp,"FETCH rs=%d c=%u\n",rs,c) ;

	} /* end if (FETCH verification) */

#if	CF_ENUM1

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: ENUM-2\n") ;
#endif

	    bprintf(ofp,"ENUM-1\n") ;

/* enumerate all entries */

	    c = 0 ;
	    hdb_curbegin(&ht,&cur) ;

	    while (hdb_enum(&ht,&cur,&key,&data) >= 0) {

	        sp = (const char *) key.buf ;
	        sl = key.len ;

	        rs = vecstr_findn(&strdb,sp,sl) ;
		if (rs < 0)
	            break ;

	        c += 1 ;

	    } /* end while */

	    hdb_curend(&ht,&cur) ;

	    bprintf(ofp,"ENUM-1 rs=%d c=%u\n",rs,c) ;

	} /* end if (ENUM-1 verification) */

#endif /* CF_ENUM1 */

#if	CF_ENUM2

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("main: ENUM-2\n") ;
#endif

	    bprintf(ofp,"ENUM-2\n") ;

/* enumerate all entries */

	    c = 0 ;
	    for (i = 0 ; vecstr_get(&strdb,i,&sp) >= 0 ; i += 1) {

		key.buf = sp ;
		key.len = strlen(sp) ;

	        rs = hdb_fetch(&ht,key,NULL,NULL) ;
		if (rs < 0)
			break ;

	        c += 1 ;

	    } /* end while */

	    bprintf(ofp,"ENUM-2 rs=%d c=%u\n",rs,c) ;

	} /* end if (ENUM-2 verification) */

#endif /* CF_ENUM */

/* loop freeing all hash DB user data */

#if	CF_DEBUGS
	debugprintf("main: cleanup phase\n") ;
#endif

	bprintf(ofp,"cleanup\n") ;

	c = 0 ;
	hdb_curbegin(&ht,&cur) ;

	while (hdb_enum(&ht,&cur,&key,&data) >= 0) {

	    sp = (const char *) key.buf ;
	    sl = key.len ;

#if	CF_DEBUGS && CF_DEBUGENUM
	    debugprintf("main: kl=%d key=%t\n",
	        key.len,key.buf,key.len) ;
#endif

#ifdef	COMMENT
	    bprintf(ofp,"key=%s data=%s\n",
	        key.buf,data.buf) ;
#endif

	    if (sp != NULL)
	        uc_free(sp) ;

	    c += 1 ;

	} /* end while */

	hdb_curend(&ht,&cur) ;

	bprintf(ofp,"FREE rs=%d c=%u\n",rs,c) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

ret3:
	hdb_finish(&ht) ;

ret2:
	vecstr_finish(&strdb) ;

ret1:
	bclose(ofp) ;

badout:
ret0:
	bclose(pip->efp) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_sysfs: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int procfile(pip,vtp,stp,ofp,fname)
PROGINFO	*pip ;
VECSTR		*vtp ;
HDB		*stp ;
bfile		*ofp ;
const char	fname[] ;
{
	FIELD		fsb ;
	HDB_DATUM	key, data ;
	bfile		infile, *ifp = &infile ;
	int		rs, rs1, rs2 ;
	int		len, fl ;
	int		c ;
	const char	*fp ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	c = 0 ;
	rs = bopen(ifp,fname,"r",0666) ;
	if (rs < 0)
	    goto ret0 ;

	while ((rs = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    if (lbuf[len - 1] == '\n') len -= 1 ;
	    lbuf[len] = '\0' ;

	    if ((rs = field_start(&fsb,lbuf,len)) >= 0) {

	        while ((fl = field_get(&fsb,NULL,&fp)) >= 0) {
	            if (fl == 0) break ;

	            rs1 = vecstr_findn(vtp,fp,fl) ;

#if	CF_DEBUGS && 0
	if (rs1 >= 0) 
	debugprintf("procfile: vecstr_findn() s=%t\n",
		fp,fl) ;
#endif

	            if (rs1 == SR_NOTFOUND)
	                rs = vecstr_add(vtp,fp,fl) ;

#if	CF_DEBUGS && 0
	debugprintf("procfile: vecstr_add() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {

	                key.buf = fp ;
	                key.len = fl ;

	                rs2 = hdb_fetch(stp,key,NULL,&data) ;

	                if (rs2 == SR_NOTFOUND) {

	                    key.len = fl ;
	                    key.buf = mallocstrw(fp,fl) ;

	                    data.len = key.len ;
	                    data.buf = key.buf ;

	                    rs = hdb_store(stp,key,data) ;

	                }

#if	CF_DEBUGS && 0
	debugprintf("procfile: hdb_store() rs=%d\n",rs) ;
#endif

	            }

#if	CF_DEBUGS 
	if (((rs1 < 0) && (rs2 >= 0)) ||
		((rs1 >= 0) && (rs2 < 0))) {
		debugprintf("procfile: rs1=%d rs2=%d s=%t\n",
		rs1,rs2,fp,fl) ;
	}
#endif

	            if (rs < 0)
	                break ;

	            c += 1 ;

	        } /* end while (field) */

	        field_finish(&fsb) ;
	    } /* end if */

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	bclose(ifp) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfile) */



