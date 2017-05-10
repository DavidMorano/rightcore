/* main (testcksum) */

/* CKSUM object testing */


#define	CF_DEBUGS	1		/* compile-time debugging */
#define	CF_STDIO	1
#define	CF_CRC		0



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#if	CF_STDIO
#include	<stdio.h>
#endif /* CF_STDIO */

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"cksum.h"
#include	"config.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

#if	CF_CRC
extern int	crc() ;
#endif


/* forward references */


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile		outfile, *ofp = &outfile ;

	CKSUM		sum ;

	int	rs ;
	int	i, len, fd ;
	int	err_fd ;
	uint	v ;
	int	ex = 0 ;

	char	*progname ;
	char	*filename ;
	char	buf[MAXPATHLEN + 1] ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	progname = argv[0] ;
	if (bopen(ofp,BFILE_STDOUT,"wct",0666) >= 0)
	    bcontrol(ofp,BC_LINEBUF,0) ;


#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	if ((argc >= 2) && (argv[1] != NULL)) {
	    filename = argv[1] ;

	} else
	    filename = "q" ;


	if ((fd = u_open(filename,O_RDONLY,0666)) < 0)
	    goto badopen ;

	if ((rs = cksum_start(&sum)) >= 0) {

	    if ((rs = cksum_begin(&sum)) >= 0) {

	        while ((len = u_read(fd,buf,MAXPATHLEN)) > 0)
	            cksum_accum(&sum,buf,len) ;

#if	CF_DEBUGS
	        debugprintf("main: CKSUM intermediate crc=%u\n",sum.sum) ;
#endif

	        cksum_end(&sum) ;

	    } /* end if */

	    len = cksum_getsum(&sum,&v) ;

	    bprintf(ofp,"cksum=%u len=%d\n",
	        v,len) ;

#if	CF_STDIO
	    printf("cksum=%u\n",v) ;

	    fclose(stdout) ;
#endif /* CF_STDIO */

	    cksum_finish(&sum) ;

	} /* end if */

#if	CF_CRC
	u_seek(fd,0L,SEEK_SET) ;

	crc(fd,&v,&len) ;

	bprintf(ofp,"cksum=%u len=%d\n",
	    v,len) ;

#endif /* CF_CRC */

	u_close(fd) ;


	ex = 0 ;

done:
	bclose(ofp) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badopen:
	bprintf(ofp,"could not open input file (rs %d)\n",
	    fd) ;

	ex = 1 ;
	goto done ;

}
/* end subroutine (main) */


