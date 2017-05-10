/* mkufname */


/* revision history:

	= 1995-04-01, David A­D­ Morano

	This is part of our cleanup-compatibility effort.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<localmisc.h>


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;


/* exported subroutines */


char *mkufname(h,s,buf)
char	h[], *s, buf[] ;
{


	if ((s != NULL) && (s[0] != '/') && 
		(! ((s[0] == '.') && (s[1] == '/'))) &&
		(! ((s[0] == '.') && (s[1] == '.')))) {

	    if (h != NULL) {

	        mkpath2(buf,h,s) ;

	        s = buf ;

	    } else
	        s = NULL ;

	}

	return s ;
}
/* end subroutine (mkufname) */



