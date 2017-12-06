/* main (ematest) */


#define	CF_DEBUGS	0		/* run-time debugging */
#define	CF_PARTS	0


/* revision history:

	= 2000-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
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
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* exported subroutines */


int main()
{
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;

	struct ema	tmpema, *emap = &tmpema ;

	struct ema_ent	*ep ;

	int	i, len ;

	char	linebuf[LINEBUFLEN + 1] ;


	bopen(ifp,BIO_STDIN,"dr",0666) ;

	bopen(ofp,BIO_STDOUT,"dwct",0666) ;


#if	CF_DEBUGS
	eprintf("main: entered\n") ;
#endif


	ema_init(emap) ;


	while ((len = bgetline(ifp,linebuf,LINEBUFLEN)) > 0) {
		if (linebuf[len - 1] == '\n') len -= 1 ;
		linebuf[len] = '\0' ;
		ema_parse(emap,linebuf,len) ;
	}



	for (i = 0 ; ema_get(emap,i,&ep) >= 0 ; i += 1) {

		bprintf(ofp,"%s\n",ep->original) ;

#if	CF_PARTS
		if (ep->address != NULL)
	    bprintf(ofp,"a=%s\n",ep->address) ;

		if (ep->route != NULL)
	    bprintf(ofp,"r=%s\n",ep->route) ;

		if (ep->comment != NULL)
	    bprintf(ofp,"c=%s\n",ep->comment) ;
#endif /* CF_PARTS */

	}


	ema_free(emap) ;


	bclose(ifp) ;

	bclose(ofp) ;

	return OK ;
}
/* end subroutine (main) */


