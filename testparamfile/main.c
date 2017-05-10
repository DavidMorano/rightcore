/* main */

/* test the PARAMFILE object */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_ENUM		1		/* enumerate */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little subroutine tests the PARAMFILE object.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecobj.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"paramfile.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		100
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		120
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local variables */

static unsigned char	tterms[] = {
	        0x00, 0x1B, 0x00, 0x00,
	        0x01, 0x10, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00
} ;

static unsigned char	dterms[] = {
	        0x00, 0x1B, 0x00, 0x00,
	        0x01, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00,
	        0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	PARAMFILE	params ;

	FIELD		f ;

	time_t	daytime = time(NULL) ;

	uint	mo_start = 0 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i, n ;
	int	len, sl, cl ;

	const char	*pfname = NULL ;
	const char	*sp, *cp ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUGS
	if ((cp = getourenv(envv,VARDEBUGFNAME)) == NULL) {
	    if ((cp = getourenv(envv,VARDEBUGFD1)) == NULL)
	        cp = getourenv(envv,VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

#if	CF_DEBUGS
	debugprintf("main: program entered\n") ;
#endif

	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	bopen(ifp,BFILE_STDIN,"dr",0666) ;

	bprintf(ofp,"PARAMFILE object test program\n") ;


/* initial stuff */

	if ((argc > 1) && (argv[1] != NULL)) pfname = argv[1] ;

	bprintf(ofp,"paramfile=%s\n",pfname) ;

	if ((pfname == NULL) || (pfname[0] == '\0')) pfname = BFILE_STDIN ;

	rs = paramfile_open(&params,envv,pfname) ;

#if	CF_DEBUGS
	debugprintf("main: paramfile_open() rs=%d\n",rs) ;
#endif

#if	CF_ENUM
	{
	        PARAMFILE_ENT		pe ;

	        PARAMFILE_CUR	cur ;

		int	el = 0 ;
		char	ebuf[EBUFLEN + 1] ;


	        if ((paramfile_curbegin(&params,&cur)) >= 0) {

	        while (rs >= 0) {

	            el = paramfile_enum(&params,&cur,&pe,ebuf,EBUFLEN) ;
		    if (el == SR_NOTFOUND) break ;
		    rs = el ;
		    if (rs < 0) break ;

	            bprintf(ofp,"k=%s value=>%s<\n",pe.key,pe.value) ;

		} /* end while */

	        paramfile_curend(&params,&cur) ;
		} /* end if (cursor) */
	}
#endif /* CF_ENUM */


/* loop stuff */

	while (rs >= 0) {

	    daytime = time(NULL) ;

	    bprintf(ofp,"query name> ") ;

	    bflush(ofp) ;

	    len = breadline(ifp,linebuf,LINEBUFLEN) ;

#if	CF_DEBUGS
	    debugprintf("main: breadline() rs=%d\n",len) ;
#endif

	    if (len <= 0)
	        break ;

	    if (linebuf[len - 1] == '\n') len -= 1 ;
	    linebuf[len] = '\0' ;

	    sl = sfshrink(linebuf,len,&sp) ;

/* check on the parameter file */

	    paramfile_check(&params,daytime) ;

/* process the current query */

	    if (len > 0) {
	        PARAMFILE_CUR	cur ;
		int	vl = 0 ;
		char	vbuf[VBUFLEN + 1] ;

	        paramfile_curbegin(&params,&cur) ;

	        for (i = 0 ; rs >= 0 ; i += 1) {

	            vl = paramfile_fetch(&params,sp,&cur,vbuf,VBUFLEN) ;

#if	CF_DEBUGS
	            debugprintf("main: paramfile_fetch() rs=%d\n",vl) ;
#endif

	            if (vl == SR_NOTFOUND) break ;
		    rs = vl ;
		    if (rs < 0) break ;

	            bprintf(ofp,"%i value=>%s<\n",i,vbuf) ;

	        } /* end for */

	        paramfile_curend(&params,&cur) ;
	    } /* end if (had a non-zero length query) */

	} /* end while */

	bprintf(ofp,"\n") ;

	rs1 = paramfile_close(&params) ;

#if	CF_DEBUGS
	debugprintf("main: paramfile_close() rs=%d\n",rs1) ;
#endif
#if	CF_DEBUGS
	            debugprintf("main: vecobj_free()\n") ;
#endif

	bclose(ifp) ;

	bclose(ofp) ;

#if	CF_DEBUGS && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return EX_OK ;
}
/* end subroutine (main) */


