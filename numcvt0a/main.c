/* main */

/* program to convert a decimal number to HEX */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1999-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	LINELEN		100



/* external subroutines */

extern int	cfdecui(char *,int,uint *) ;

extern char	*strbasename(char *) ;





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	infile, *ifp = &infile ;

	struct proginfo		g, *pip = &g ;

	FIELD	fsb ;

	uint	value ;

	int	len ;

	char	buf[LINELEN] ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


	if (bopen(ofp,BFILE_STDOUT,"wc",0666) < 0)
		return EX_CANTCREAT ;

	if (bopen(ifp,BFILE_STDIN,"r",0666) < 0)
		return EX_NOINPUT ;


	while ((len = breadline(ifp,buf,LINELEN)) > 0) {

		if (buf[len - 1] == '\n')
			len -= 1 ;

	    fsb.rlen = len ;
	    fsb.lp = buf ;
	    while (field_get(&fsb,NULL) >= 0) {

#if	CF_DEBUGS
	debugprintf("main: 1 gp=>%W<\n",fsb.fp,fsb.flen) ;
#endif

	        if (cfdecui(fsb.fp,fsb.flen,&value) >= 0) {

#if	CF_DEBUGS
	debugprintf("main: 2\n") ;
#endif

	            bprintf(ofp,"%08lx\n", value) ;

	        } else {

#if	CF_DEBUGS
	debugprintf("main: 2a\n") ;
#endif

	            bprintf(efp,
	                "%s: \"%W\" is not a valid decimal number\n",
	                g.progname,fsb.fp,fsb.flen) ;

		}

#if	CF_DEBUGS
	debugprintf("main: 3\n") ;
#endif

	        field_get(&fsb,NULL) ;

	    } /* end while (getting fields) */

#if	CF_DEBUGS
	debugprintf("main: 4\n") ;
#endif

	    bflush(ofp) ;

	} /* end while (reading lines) */

	bclose(ifp) ;

	bclose(ofp) ;

	return EX_OK ;
}
/* end subroutine (main) */



