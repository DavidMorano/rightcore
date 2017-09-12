/* strtoumax */


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

uintmax_t strtoumax(const char *str,char **endptr,int base)
{
	return strtoul(str,endptr,base) ;
}

uintmax_t strtouintmax(const char *str,char **endptr,int base)
{
	return strtoul(str,endptr,base) ;
}

#else /* defined(_LP64) */

uintmax_t strtoumax(const char *str,char **endptr,int base)
{
	return strtoull(str,endptr,base) ;
}

uintmax_t strtouintmax(const char *str,char **endptr,int base)
{
	return strtoull(str,endptr,base) ;
}

#endif /* defined(_LP64) */


