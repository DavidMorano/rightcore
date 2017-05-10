/* mkplogid */

/* make a prefix log ID */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine makes a prefix log ID for the PCSPOLL program
	(or other programs that have sub-jobs associated with them
	and where they require a prefix log ID).

	Synopsis:

	int mkplogid(dbuf,dlen,nodename,v)
	char		dbuf[] ;
	int		dlen ;
	const char	nodename[] ;
	int		v ;

	Arguments:

	dbuf		destination buffer
	dlen		destination length
	nodename	nodename
	v		value

	Returns:

	<0		error
	>=0		length of result


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>

#include	<localmisc.h>



/* local defines */

#define	MAXNC		3		/* maximum nodename characters */



/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkplogid(dbuf,dlen,nodename,v)
char		dbuf[] ;
int		dlen ;
const char	nodename[] ;
int		v ;
{
	int	rs ;
	int	nl ;
	int	ni ;

	char	scratch[MAXNC + 1] ;


	if (dbuf == NULL)
	    return SR_FAULT ;

	if (nodename == NULL)
	    return SR_FAULT ;

	nl = strlen(nodename) ;

	if (nl <= MAXNC)
	    ni = 0 ;

	else if (nl <= (MAXNC + 2))
	    ni = 2 ;

	else
	    ni = (nl - MAXNC) ;

	strwcpy(scratch,(nodename + ni),MAXNC) ;

	v = (v % (PID_MAX + 1)) ;
	rs = snsd(dbuf,dlen,scratch,v) ;

	return rs ;
}
/* end subroutine (mkplogid) */



