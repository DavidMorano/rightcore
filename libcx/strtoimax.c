/* strtoimax */


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


