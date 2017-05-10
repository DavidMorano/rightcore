
#define	F_EXTERN	0


#include	<sys/utsname.h>
#include	<stdio.h>


#if	F_EXTERN
extern struct utsname	utsname ;
#endif



int main()
{
	struct utsname	utsname ;


#if	F_EXTERN
	fprintf(stdout,"name=%s\n",
		utsname.nodename) ;
#endif

	uname(&utsname) ;

	fprintf(stdout,"name=%s\n",
		utsname.nodename) ;


	fclose(stdout) ;

	return 0 ;
}


