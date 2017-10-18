/* main (testcksum) */

/* CKSUM object testing */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-17, David A­D­ Morano
        I refactoed this somewhat.  I looked at this because I needed to test
	the CKSUM object again (due to a relatively small change w/ that).

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test the CKSUM object.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"cksum.h"
#include	"config.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	RBUFLEN
#define	RBUFLEN		MAXPATHLEN
#endif


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrchr(cchar *,int,int) ;


/* forward references */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	bfile		outfile, *ofp = &outfile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_OK ;
	cchar		*ifname = NULL ;
	cchar		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("main: ent\n") ;
#endif

	if ((argc >= 2) && (argv[1] != NULL)) {
	    ifname = argv[1] ;
	} else {
	    ifname = "q" ;
	}

	if ((rs = bopen(ofp,BFILE_STDOUT,"wct",0666)) >= 0) {
	    bcontrol(ofp,BC_LINEBUF,0) ;

	    if ((ifname == NULL) || (ifname[0] == '\0'))
	        ifname = "/dev/fd/0" ;

	    if ((rs = uc_open(ifname,O_RDONLY,0666)) >= 0) {
	        CKSUM		sum ;
	        const int	fd = rs ;

#if	CF_DEBUGS
		debugprintf("main: cksum_start()\n") ;
#endif

	        if ((rs = cksum_start(&sum)) >= 0) {
		    int		tlen = 0 ;

	            if ((rs = cksum_begin(&sum)) >= 0) {
	                const int	rlen = RBUFLEN ;
			int		len ;
	                char		rbuf[RBUFLEN+ 1] ;

	                while ((rs = u_read(fd,rbuf,rlen)) > 0) {
			    len = rs ;
			    tlen += len ;
	                    rs = cksum_accum(&sum,rbuf,len) ;
			    if (rs < 0) break ;
			} /* end while */

#if	CF_DEBUGS
	                debugprintf("main: out rs=%d\n",rs) ;
#endif

	                rs1 = cksum_end(&sum) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (cksum-accum) */

#if	CF_DEBUGS
		    debugprintf("main: accum-out rs=%d tlen=%u\n",rs,tlen) ;
#endif

	            if (rs >= 0) {
	                uint	val ;
	                tlen = cksum_getsum(&sum,&val) ;
	                bprintf(ofp,"cksum=%10u len=%10d\n",val,tlen) ;
	            }

	            rs1 = cksum_finish(&sum) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (cksum) */

#if	CF_DEBUGS
		debugprintf("main: cksum_start() out rs=%d\n",rs) ;
#endif

	        u_close(fd) ;
	    } /* end if (file-input) */

	    bclose(ofp) ;
	} /* end if (file-output) */

#if	CF_DEBUGS
	debugprintf("main: done rs=%d\n",rs) ;
#endif

	if (rs < 0) ex = EX_DATAERR ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


