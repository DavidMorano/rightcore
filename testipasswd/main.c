/* main */

/* test the IPASSWD object */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_DEBUG	1		/* run-time debugging */
#define	CF_DEBUGSPWI	1		/* some extra thing */
#define	CF_COLLISIONS	1
#define	CF_LASTFULL	0		/* some option */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This little subroutine tests the IPASWD object.



******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"ipasswd.h"
#include	"realname.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	snrealname(char *,int,const char **,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int prompt(bfile *,bfile *,char *,int) ;


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
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	IPASSWD		pwi ;
	IPASSWD_CUR	cur ;

	REALNAME	rn ;

	FIELD		f ;

	time_t	daytime = time(NULL) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	const int	llen = LINEBUFLEN ;

	int	rs, rs1 ;
	int	i, n ;
	int	len, sl ;
	int	fopts = 0 ;

	const char	*cp ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	ubuf[IPASSWD_USERNAMELEN + 1] ;


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

	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

#if	CF_LASTFULL
	fopts = IPASSWD_FLASTFULL ;
#endif

	bprintf(ofp,"IPASSWD object test program\n") ;

/* initial stuff */

	if ((argc > 1) && (argv[1] != NULL)) {
	    const char	*dbname = argv[1] ;
#if	CF_DEBUGS
	debugprintf("main: dbname=%s\n",dbname) ;
#endif
	    rs = ipasswd_open(&pwi,argv[1]) ;

	} else
	    rs = ipasswd_open(&pwi,IPASSWDDB) ;

#if	CF_DEBUGS
	debugprintf("main: ipasswd_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGSPWI
	{
	    const int	rlen = REALNAMELEN ;
	    int		j ;
	    const char	*ss[6] ;
	    char	rbuf[REALNAMELEN+1] ;
	    char	ubuf[USERNAMELEN+1] ;
	    debugprintf("main: record enumeration\n") ;
	    if ((rs = ipasswd_curbegin(&pwi,&cur)) >= 0) {
	        const int	nlen = REALNAMELEN ;
	        char		nbuf[REALNAMELEN+1] ;
	        while (rs >= 0) {
	            rs = ipasswd_enum(&pwi,&cur,ubuf,ss,rbuf,rlen) ;
	            debugprintf("main: ipasswd_enum() rs=%d\n",rs) ;
	            if (rs < 0) break ;
	            debugprintf("main: username=%s\n",ubuf) ;
	            for (j = 0 ; (j < 5) && ss[j] ; j += 1) {
	                debugprintf("main: n%u=%s\n",j,ss[j]) ;
	            }
	            snrealname(nbuf,nlen,ss,j) ;
	            debugprintf("main: name=%s\n",nbuf) ;
	        }
	        ipasswd_curend(&pwi,&cur) ;
	    } /* end if (cursor) */
	}
#endif /* CF_DEBUGSPWI */


/* loop stuff */

	if ((rs = bopen(ifp,BFILE_STDIN,"r",0666)) >= 0) {

	    while ((rs = prompt(ofp,ifp,lbuf,llen)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;
	        if (len == 0) break ;

	        if (len > 0) {
	            IPASSWD_INFO	info ;
	            int	cols, kcols ;

	            if ((rs = realname_start(&rn,lbuf,len)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("main: realname_start() rs=%d\n",rs) ;
	                debugprintf("main: last=%p\n",rn.last) ;
	                debugprintf("main: first=%p\n",rn.last) ;
	                debugprintf("main: m1=%p\n",rn.last) ;
	                debugprintf("main: storelen=%d\n",rn.len.store) ;
#endif /* CF_DEBUGS */


	                bprintf(ofp,"last=%s\n",rn.last) ;

	                bprintf(ofp,"first=%s\n",rn.first) ;

	                bprintf(ofp,"m1=%s\n",rn.m1) ;


#if	CF_COLLISIONS
	                ipasswd_info(&pwi,&info) ;
	                cols = info.collisions ;
#endif


	                if ((rs = ipasswd_curbegin(&pwi,&cur)) >= 0) {

	                    for (i = 0 ; TRUE ; i += 1) {

	                        rs = ipasswd_fetch(&pwi,&rn,&cur,fopts,ubuf) ;

#if	CF_DEBUGS
	                        debugprintf("main: ipasswd_fetch() rs=%d\n",
					rs) ;
#endif

	                        if (rs < 0) break ;
	                        bprintf(ofp,"%d username=%s\n",i,ubuf) ;
	                    } /* end for */

	                    ipasswd_curend(&pwi,&cur) ;
	                } /* end if (cursor) */


#if	CF_COLLISIONS
	                ipasswd_info(&pwi,&info) ;
	                kcols = (info.collisions - cols) ;
	                bprintf(ofp,"key collisions=%d\n",kcols) ;
#endif

	                realname_finish(&rn) ;
	            } /* end if (had a non-zero length query) */

	        } /* end if */

	    } /* end while */

	    bputc(ofp,'\n') ;

	    bclose(ifp) ;
	} /* end if (file-open) */

	ipasswd_close(&pwi) ;
	} /* end if (ipasswd) */


	bclose(ofp) ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return EX_OK ;
}
/* end subroutine (main) */


/* local subroutines */


static int prompt(bfile *ofp,bfile *ifp,char *lbuf,int llen)
{
	int	rs ;

	rs = bprintf(ofp,"query name> ") ;
	if (rs >= 0) bflush(ofp) ;

	if (rs >= 0) rs = breadline(ifp,lbuf,llen) ;

	return rs ;
}
/* end subroutine (prompt) */


