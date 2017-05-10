/* main_test */


#define	CF_DEBUGS	1



#include	<sys/types.h>
#include	<sys/param.h>

#include	<bfile.h>

#include	"localmisc.h"
#include	"netorder.h"




int main()
{
	bfile	outfile ;

	int	iw ;
	int	rs, i ;

	char	buf[MAXPATHLEN + 1] ;


	bopen(&outfile,BFILE_STDOUT,"wct",0666) ;


	iw = 1 ;

#if	CF_DEBUGS
	debugprintf("main: netorder_wint()\n") ;
#endif

	netorder_wint(buf + 1,iw) ;

#if	CF_DEBUGS
	debugprintf("main: loop\n") ;
#endif

	for (i = 0 ; i < 4 ; i += 1) {

		bprintf(&outfile," %02x",(buf + 1)[i]) ;

	}

	bprintf(&outfile,"\n") ;


/* OK, let's try reading ! */

	netorder_rint(buf + 1,&iw) ;

	bprintf(&outfile,"read %08x\n",iw) ;



	bclose(&outfile) ;

	return 0 ;
}




