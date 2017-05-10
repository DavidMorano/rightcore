/* procfile */

/* process an input file to the output */


#define	CF_DEBUG	1


/* revistion history:

	= 97/09/10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This subroutine processes a file by writing the
	parsed addresses out to the output file.


*********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;


/* local structures */


/* forward references */

static int	procitem(struct proginfo *,bfile *,const char *) ;


/* local variables */


/* exported subroutines */


int procfile(pip,ofp,infname,fn)
struct proginfo	*pip ;
bfile		*ofp ;
char		infname[] ;
int		fn ;
{
	bfile		infile, *ifp = &infile ;

	struct ustat	sb ;

	int	rs = SR_OK ;
	int	lines, len ;
	int	f_stdinput = FALSE ;

	char	linebuf[LINEBUFLEN + 1], *lbp ;
	char	*cp ;


/* check the arguments */

	if ((infname == NULL) || (infname[0] == '-')) {

	    f_stdinput = TRUE ;
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	} else
	    rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0)
	    goto ret0 ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procfile: processing file number %d\n",fn) ;
#endif


/* output the pages */

	lines = 0 ;
	while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    int	tlen ;


	    if (linebuf[len - 1] == '\n')
		linebuf[--len] = '\0' ;

	    len = rs ;
	    tlen = sfshrink(linebuf,len,&lbp) ;

	    if (tlen <= 0) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procfile: LINE>%t",lbp,len) ;
#endif

	    lbp[tlen] = '\0' ;
	    while ((cp = strchr(lbp,',')) != NULL) {

	        *cp++ = '\0' ;
	        rs = procitem(pip,ofp,lbp) ;

		if (rs < 0)
			break ;

	        lbp = cp ;

	    } /* end while */

	    if ((rs >= 0) && (lbp[0] != '\0'))
	    	rs = procitem(pip,ofp,lbp) ;

	    if (rs < 0)
		break ;

	    lines += 1 ;

	} /* end while (reading file lines) */

done:
	bclose(ifp) ;

ret0:
	return (rs > 0) ? lines : rs ;
}
/* end subroutine (procfile) */



/* LOCAL SUBROUTINES */



static int procitem(pip,ofp,s)
struct proginfo	*pip ;
bfile		*ofp ;
const char	s[] ;
{
	int	rs ;
	int	wlen ;
	int	cl ;

	char	*cp ;


	wlen = 0 ;
	cl = sfshrink(s,-1,&cp) ;

	if (cp[0] != '\0') {

	    rs = bprintf(ofp,"%t\n",cp,cl) ;

	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procitem) */



