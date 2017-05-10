/* main */


#define	CF_DEBUGS	0


#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	"localmisc.h"



#define	REALNAMELEN	100



int main()
{
	int	rs ;
	int	len ;
	int	c ;
	int	i ;

	char	namebuf[REALNAMELEN + 1] ;



	i = 0 ;
	while ((c = fgetc(stdin)) != EOF) {

#if	CF_DEBUGS
		fprintf(stderr,"main: i=%u c=%c\n",i,c) ;
#endif

	    if ((i == 0) && (c == 'd')) {

	        namebuf[i++] = c ;

	    } else if (i > 0) {

	        switch (c) {

	        case 's':
	            if (namebuf[i - 1] == 'd')
	                namebuf[i++] = c ;

	                else
	                i = 0 ;

	            break ;

	        case 'c':
	            if (namebuf[i - 1] == 's')
	                namebuf[i++] = c ;

	                else
	                i = 0 ;

	            break ;

	        default:
	            if (isdigit(c) && (i >= 3) && (i < (3+5))) {

	                namebuf[i++] = c ;

	            } else
	                i = 0 ;

	            break ;

	        case '.':
	            if (isdigit(namebuf[i - 1])) {

	                namebuf[i++] = c ;

	            } else
	                i = 0 ;

		    break ;

	        case 'j':
	            if (namebuf[i - 1] == '.')
	                namebuf[i++] = c ;
	                else
	                i = 0 ;

	            break ;

	        case 'p':
	            if (namebuf[i - 1] == 'j')
	                namebuf[i++] = c ;
	                else
	                i = 0 ;

	            break ;

	        case 'g':
	            if (namebuf[i - 1] == 'p') {
	                namebuf[i++] = c ;
	                namebuf[i] = '\0' ;
	                fprintf(stdout,"%s\n",namebuf) ;
	            }
	                i = 0 ;

	            break ;

	        } /* end switch */

#if	CF_DEBUGS
		if (i > 0) {
			namebuf[i] = '\0' ;
	                fprintf(stderr,"namebuf=%s\n",namebuf) ;
		}
#endif

	    } /* end if */

	} /* end while */

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



