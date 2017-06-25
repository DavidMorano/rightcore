/* notify */

/*
	David A.D. Morano
	February 88
*/



#include	<sys/types.h>
#include	<stdio.h>
#include	<time.h>

#include	<termstr.h>

#include	"term.h"
#include	"localmisc.h"



#define		TIME_ON		30
#define		TIME_OFF	20




int main()
{
	long	clock ;

	int	len ;

	char	*buf, dbuf[100] ;

	printf("%s",TERMSTR_NORM) ;

	fflush(stdout) ;

	sleep(TIME_OFF) ;

	while (1) {

		len = sprintf(dbuf, 
		"%s\033[24;40H          %syou have mail\033[K%s",
		TERMSTR_SAVE,TERMSTR_BLINK,TERMSTR_RESTORE) ;

		write(1L,dbuf,len) ;

		sleep(TIME_ON) ;

		len = sprintf(dbuf, 
		"%s\033[24;40H\033[K%s",
		TERMSTR_SAVE,TERMSTR_RESTORE) ;

		write(1L,dbuf,len) ;

		sleep(TIME_OFF) ;
	}

}
/* end subroutine (main) */


