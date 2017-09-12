/* strtoimax */


/* revision history:

	= 2017-09-07, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<inttypes.h>
#include	<stdlib.h>


#if defined(_LP64)

intmax_t strtoimax(const char *str,char **endptr,int base)
{
	return strtol(str,endptr,base) ;
}

intmax_t strtointmax(const char *str,char **endptr,int base)
{
	return strtol(str,endptr,base) ;
}

#else /* defined(_LP64) */

intmax_t strtoimax(const char *str,char **endptr,int base)
{
	return strtoll(str,endptr,base) ;
}

intmax_t strtointmax(const char *str,char **endptr,int base)
{
	return strtoll(str,endptr,base) ;
}

#endif /* defined(_LP64) */


