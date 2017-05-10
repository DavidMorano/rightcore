/* main (ematest) */


#define	CF_?gDEBUGS	0
#define	CF_?gPARTS		0



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
#include	<errno.h>
#include	<dirent.h>
#include	<string.h>

#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<veclist.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<char.h>
#include	<ema.h>

#include	"misc.h"



/* local defines */

#define	LINELEN	200




int main()
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	struct ema	tmpema, *emap = &tmpema ;

	struct ema_ent	*ep ;

	int	i, len ;

	char	linebuf[LINELEN + 1] ;


	bopen(ifp,BIO_STDIN,"dr",0666) ;

	bopen(ofp,BIO_STDOUT,"dwct",0666) ;


#if	CF_?gDEBUGS
	eprintf("main: entered\n") ;
#endif


	ema_init(emap) ;


	while ((len = bgetline(ifp,linebuf,LINELEN)) > 0) {

		if (linebuf[len - 1] == '\n') len -= 1 ;

		linebuf[len] = '\0' ;

	ema_parse(emap,linebuf,len) ;

	}



	for (i = 0 ; ema_get(emap,i,&ep) >= 0 ; i += 1) {

		bprintf(ofp,"%s\n",ep->original) ;

#if	CF_?gPARTS
		if (ep->address != NULL)
	    bprintf(ofp,"a=%s\n",ep->address) ;

		if (ep->route != NULL)
	    bprintf(ofp,"r=%s\n",ep->route) ;

		if (ep->comment != NULL)
	    bprintf(ofp,"c=%s\n",ep->comment) ;
#endif /* CF_?gPARTS */

	}


	ema_free(emap) ;


	bclose(ifp) ;

	bclose(ofp) ;

	return OK ;
}
/* end subroutine (main) */



