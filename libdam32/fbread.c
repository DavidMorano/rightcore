/* fbread */

/* read a block of (binary) data from the STDIO stream */


#define	CF_FGETS	0		/* faster or not? */


/* revision history:

	= 1998-08-17, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a knock-off of the 'fread(3)' subroutine, only sensible!

	Synopsis:

	int fbread(fp,buf,len)
	FILE	*fp ;
	char	buf[] ;
	int	len ;

	Arguments:

	fp		pointer to a FILE object
	buf		user supplied buffer
	len		size of user supplied buffer


	Returns:

	>0		size of data returned
	==0		no data was returned
	<0		an error occurred


******************************************************************************/


#include	<envstandards.h>

#include	<stdio.h>

#include	<vsystem.h>


/* local defines */


/* exported subroutines */


int fbread(fp,bbuf,blen)
FILE	*fp ;
char	bbuf[] ;
int	blen ;
{
	int	rs ;


	if (fp == NULL)
	    return SR_FAULT ;

	if (bbuf == NULL)
	    return SR_FAULT ;

	if (blen == 0)
	    return 0 ;

	rs = fread(bbuf,1,blen,fp) ;

	if ((rs == 0) && ferror(fp)) {
	   clearerr(fp) ;
	   rs = SR_IO ;
	}

	return rs ;
}
/* end subroutine (fbread) */



