/* main */

/* test term stuff */



/* reivision history :

	= February 88, David A­D­ Morano

*/



#include	<time.h>

#include	<termstr.h>
#include	<bfile.h>

#include	"localmisc.h"



#define		TIME_ON		2
#define		TIME_OFF	2
#define		NLOOPS		10




int main()
{
	bfile	berr, *efp = &berr ;
	bfile	bout, *ofp = &bout ;

	long	clock ;

	int	len, i, j ;

	char	*bp, dbuf[1000] ;


	bopen(efp,BERR,"rc",0666) ;

	bopen(ofp,BOUT,"rc",0666) ;

	bprintf(ofp,"%s",TERMSTR_NORM) ;


	sleep(TIME_OFF) ;

	for (i = 0 ; i < NLOOPS ; i += 1) {

		bp = dbuf ;

		bp += sprintf(bp, 
		"%s%s%s\033[14;40H%s",
		TERMSTR_SAVE,TERMSTR_BOLD,TERMSTR_BLINK,TERMSTR_EL) ;

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


		bwrite(ofp,dbuf,bp - dbuf) ;

		bflush(ofp) ;

		sleep(TIME_ON) ;

	} ;

	bflush(ofp) ;

	bflush(efp) ;

	return OK ;
}

