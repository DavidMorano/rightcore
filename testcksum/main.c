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

#if	CF_DEBUGS
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

	        if ((rs = cksum_start(&sum)) >= 0) {
		    int		tlen = 0 ;

	            if ((rs = cksum_begin(&sum)) >= 0) {
	                const int	rlen = RBUFLEN ;
			int		len ;
	                char		rbuf[RBUFLEN+ 1] ;

	                while ((len = u_read(fd,rbuf,rlen)) > 0) {
			    tlen += len ;
	                    cksum_accum(&sum,rbuf,len) ;
			}

#if	CF_DEBUGS
	                debugprintf("main: CKSUM intermediate crc=%u\n",
			    sum.sum) ;
#endif

	                cksum_end(&sum) ;
	            } /* end if (cksum-accum) */

	            {
	                uint	val ;
	                tlen = cksum_getsum(&sum,&val) ;
	                bprintf(ofp,"cksum=%u len=%d\n",val,tlen) ;
	            }

	            cksum_finish(&sum) ;
	        } /* end if (cksum) */

	        u_close(fd) ;
	    } /* end if (file-input) */

	    bclose(ofp) ;
	} /* end if (file-output) */

	if (rs < 0) ex = EX_DATAERR ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


