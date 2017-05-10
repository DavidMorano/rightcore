/* process */

/* program to test out the 'cfnum' subroutine */
/* last modified %G% version %I% */


#define	CF_DEBUG	1


/* revision history:

	= 88/01/01, David A­D­ Morano

	This subroutine (it's the whole program -- same as
	the FIFO test) was originally written.


*/


/************************************************************************

	This program tests the 'cfnum' and other similar subroutines.


***************************************************************************/


#include	<sys/types.h>
#include	<sys/mman.h>
#include	<sys/msg.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<termios.h>
#include	<signal.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<char.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif



/* external subroutines */

extern int	cfnumui(char *,int,uint *) ;
extern int	cfnumull(char *,int,ULONG *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */









int process(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
char		fname[] ;
{
	bfile		ifile, *ifp = &ifile ;

	ULONG		ulw ;
	LONG		lw ;

	uint		val ;

	int		len, rs ;

	char	linebuf[LINELEN + 1], *bp ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("process: entered, fname=%s\n",fname) ;
#endif

	if ((fname == NULL) || (fname[0] == '\0') || (fname[0] == '-'))
		rs = bopen(ifp,BFILE_STDIN,"rd",0666) ;

	else
		rs = bopen(ifp,fname,"r",0666) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("process: bopen rs=%d\n",rs) ;
#endif

	if (rs < 0)
		goto badin ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("process: about to loop reading lines\n") ;
#endif

	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

		if (linebuf[len - 1] == '\n')
			len -= 1 ;

		linebuf[len] = '\0' ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("process: line> %W\n",linebuf,len) ;
#endif

		cp = linebuf ;
		while (*cp && CHAR_ISWHITE(*cp)) {

			cp += 1 ;
			len -= 1 ;
		}

		if ((cp[0] == '\0') || (len <= 0))
			continue ;

		bprintf(ofp,"\norig=%s\n",cp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("process: cfnumui() >%W<\n",cp,len) ;
#endif

		rs = cfnumui(cp,len,&val) ;

		bprintf(ofp,"CFNUMUI conversion rs=%d\n",rs) ;

		bprintf(ofp,"\tvalue=\\x%08x (%u)\n",
			val,val) ;

/* regular 'int' conversion */

		rs = cfnumi(cp,len,&val) ;

		bprintf(ofp,"CFNUMI conversion rs=%d\n",rs) ;

		bprintf(ofp,"\tvalue=\\x%08x (%d)\n",
			val,val) ;

/* do a LONG conversion also */

		rs = cfdecll(cp,len,&lw) ;

		bprintf(ofp,"CFDECLL conversion rs=%d\n",rs) ;

		bprintf(ofp,"\tvalue=\\x%16llx (%lld)\n",
			lw,lw) ;


		rs = cfnumull(cp,len,&ulw) ;

		bprintf(ofp,"CFNUMULL conversion rs=%d\n",rs) ;

		bprintf(ofp,"\tvalue=\\x%16llx (%llu)\n",
			ulw,ulw) ;


	} /* end while */

	bclose(ifp) ;


	rs = len ;


badret:
	return rs ;

/* bad stuff comes here */
badin:
	bprintf(pip->efp,"%s: could not open input, rs=%d\n",
	    pip->progname,rs) ;

	goto badret ;

}
/* end subroutine (process) */




