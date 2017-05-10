/* subfile */

/* subroutine to perform variable substitutions on files */


#define	CF_DEBUG	0


/* revision history:

	= David A.D. Morano, October 1993

	This subroutine was originally written.

	= David A.D. Morano, Febuary 1996

	This program was pretty extensively modified to take
	much more flexible combinations of user supplied paramters.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<rpc/rpc.h>
#include	<unistd.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<paramopt.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"varsub.h"
#include	"configfile.h"


/* defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define		BUFLEN		(MAXPATHLEN + (LINEBUFLEN * 2))


/* external subroutines */

extern int	matstr() ;
extern int	vsainit(), vsaadd(), vsaget(), vsafree() ;
extern int	configread(), configfree() ;
extern int	paramloads(), paramloadu(), paramload() ;
extern int	cfdouble() ;
extern int	machineinit(), machineadd() ;
extern int	runinit(), runadd(), rundel() ;

extern char	*strbasename() ;

extern void	vsaprint() ;
extern void	fixdisplay() ;


/* forward references */

void	int_signal() ;


/* global data */

struct global		g ;


/* exported subroutines */


int subfile(vshp,infname,outfname)
struct varsub	*vshp ;
char		*infname, *outfname ;
{
	bfile		outfile, *ofp = &outfile ;
	int		rs ;
	int		rs1 ;

	if ((rs = bopen(ofp,outfname,"wct",0666)) >= 0) {
	    bfile	infile, *ifp = &infile ;
	    if ((rs = bopen(ifp,infname,"r",0666)) >= 0) {
		{
		    rs = varsub_expandfile(vshp,ifp,ofp) ;
		}
	        rs1 = bclose(ifp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (file-input) */
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-output) */

	return rs ;
}
/* end subroutine (subfile) */



