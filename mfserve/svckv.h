/* svckv INCLUDE */

/* these subroutines perform key-value type functions */


/* revision history:

	= 2017-10-13, David A­D­ Morano
	This was split out of the HOMEPAGE program (where it was originally
	local).

*/

/* Copyright © 2018 David A­D­ Morano.  All rights reserved. */

#ifndef	SVCKV_INCLUDE
#define	SVCKV_INCLUDE	1


#include	<envstandards.h>
#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int	svckv_val(cchar *(*)[2],int,cchar *,cchar **) ;
extern int	svckv_dequote(cchar *(*)[2],int,cchar *,cchar **) ;
extern int	svckv_isfile(cchar *(*)[2],int,cchar **) ;
extern int	svckv_ispass(cchar *(*)[2],int,cchar **) ;
extern int	svckv_isprog(cchar *(*)[2],int,cchar **) ;
extern int	svckv_svcopts(cchar *(*)[2],int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCKV_INCLUDE */


