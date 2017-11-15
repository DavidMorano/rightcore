/* svckv INCLUDE */

/* these subroutines perform key-value type functions */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

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
extern int	svckv_isexec(cchar *(*)[2],int,cchar **) ;
extern int	svckv_svcopts(cchar *(*)[2],int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCKV_INCLUDE */


