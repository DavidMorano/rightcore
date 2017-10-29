
#include	<stdio.h>

int main()
{

#if	defined(OSNAME_Dummy) && (OSNAME_Dummy > 0)
	printf("OSNAME=Dummy\n") ;
#elif	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	printf("OSNAME=SunOS\n") ;
#elif	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
	printf("OSNAME=Darwin\n") ;
#endif

	return 0 ;
}


