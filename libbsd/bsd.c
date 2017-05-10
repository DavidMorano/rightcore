
#include	<string.h>



void bcopy(s1,s2,len)
char	*s1, *s2 ;
int	len ;
{

	(void) memcpy(s2,s1,(size_t) len) ;

}


int bcmp(s1,s2,len)
char	*s1, *s2 ;
int	len ;
{


	return memcmp(s1,s2,(size_t) len) ;
}


void bzero(sp,len)
char	*sp ;
int	len ;
{

	(void) memset(sp,0,(size_t) len) ;
}




