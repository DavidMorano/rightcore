/* main */

/* test term stuff */


/* revision history:

	= February 88, David A­D­ Morano


*/



#include	<time.h>
#include	<stdio.h>

#include	<termstr.h>


#define		TIME_ON		2
#define		TIME_OFF	2



int main()
{
	long	clock ;

	int	len, i, j ;

	char	*bp, dbuf[1000] ;


	printf("%s",TERMSTR_NORM) ;

	fflush(stdout) ;

	sleep(TIME_OFF) ;

	while (1) {

		bp = dbuf ;

		bp += sprintf(bp, 
		"%s\033[14;40H%s",
		TERMSTR_SAVE,TERMSTR_EL) ;

		bp += sprintf(bp, 
		"\033[15;40H%s",
		TERMSTR_EL) ;

		bp += sprintf(bp, 
		"\033[16;40H          can you see this%s",
		TERMSTR_EL) ;

		bp += sprintf(bp,
		"\033[17;40H          yes I can but%s",TERMSTR_EL) ;

		bp += sprintf(bp,
		"\033[18;40H          do I want to%s",TERMSTR_EL) ;

		bp += sprintf(bp,
		"\033[19;40H          today or tomorrow%s",TERMSTR_EL) ;

		bp += sprintf(bp,
		"\033[20;40H          I love you tomorrow%s",TERMSTR_EL) ;

		bp += sprintf(bp,
		"\033[21;40H          we're always a day%s",TERMSTR_EL) ;

		bp += sprintf(bp,
		"\033[22;40H          away, away from home%s",
		TERMSTR_EL) ;

		bp += sprintf(bp,
		"\033[23;40H          and away a day%s%s",
		TERMSTR_EL,TERMSTR_RESTORE) ;

#ifdef	COMMENT
		j = 0 ;
		while (j < (bp - dbuf)) {

		for (i = 0 ; i < 16 ; i += 1) {

			printf(" %02X",dbuf[j++] & 0xFF) ;
		} ;

		printf("\n") ;

		} ;

		fflush(stdout) ;
#endif

		write(1L,dbuf,bp - dbuf) ;

		sleep(TIME_ON) ;

	}

}


