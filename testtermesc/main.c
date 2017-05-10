/* main */

/* program to clear the terminal screen */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 1985-08-10, David A­D­ Morano

	This subroutine was originally written.

	= 1998-11-01, David A­D­ Morano

	After all of these years I seem to need to add a
	"shift in" to the cleanup procedure.  This shifts back
	into the current font the normal characters that should be
	there (usually ISO Latin-1).


*/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<termios.h>

#include	<vsystem.h>
#include	<termstr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		2048
#endif

#define	TFD	1

#define	SPECIAL		"\033[10X"


/* external subroutines */


/* local variables */

static const char	*s[] = {
	"\033[H\033[J\033[m",		/* clear screen, attributes off */
	"\017",				/* shift-in */
	"\033[4l",			/* insert/replace off */
	TERMSTR_S_CUR,			/* set cursor on */
	NULL
} ;

/* NOTES: grammar

	DCS	= ESC P
	ST	= ESC \
	CSI	= ESC [

*/

static const char	*s_extra[] = {
	"\033P1!uA\033\\",	/* designate ISO Latin-1 as supplimental */
	"\033(B",		/* map ASCII to G0 */
	"\033)0",		/* DEC special graphic as G1 */
	"\033.AF",		/* map ISO Latin-1 as G2 */
	"\033+>",		/* map DEC Technical to G3 */
	"\017",			/* ASCII SHIFT-IN (G0 into GL) */
	"\033\175",		/* lock shift G2 into GR */
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct winsize	ws ;

	int	rs = 0 ;
	int	len, lines ;
	int	f_extra = FALSE ;
	int	f_special = FALSE ;

	const char	**sp = s ;

	char	buf[256] ;


	if (! isatty(TFD)) 
		return 0 ;

	if (argc == 2) {
		f_extra = TRUE ;
	} else if (argc == 3)
		f_special = TRUE ;

/* load all of the output strings into a common buffer */

	buf[0] = '\0' ;
	while (*sp != NULL) {

	    strcat(buf,*sp) ;

	    sp += 1 ;

	} /* end while */

	if (f_extra) {

	sp = s_extra ;
	while (*sp != NULL) {

	    strcat(buf,*sp) ;

	    sp += 1 ;

		} /* end while */

	} /* end if */

	if (! f_special) {

	u_write(TFD,buf,strlen(buf)) ;

	if (ioctl(TFD, TIOCGWINSZ, &ws) >= 0) {

	    if (ws.ws_row > 0) {
		lines = ws.ws_row ;
	    } else 
		lines = 24 ;

	    bufprintf(buf,BUFLEN,"\033[1;%dr",lines) ;

	    rs = u_write(TFD,buf,strlen(buf)) ;

	} /* end if */

	if (argc > 1) {

	    len = bufprintf(buf,BUFLEN,"%s\r%s%s",
	        TERMSTR_S_SD,TERMSTR_ED,TERMSTR_R_SD) ;

#ifdef	COMMENT
	    len += bufprintf((buf + len),(BUFLEN - len),
		"\033[26;1H\r%s\033[1;1H%s",
	        TERMSTR_ED,TERMSTR_ED) ;
#endif

	    rs = u_write(TFD,buf,len) ;

	}

	} else
		rs = u_write(TFD,SPECIAL,strlen(SPECIAL)) ;

#ifdef	COMMENT
	bufprintf(buf,BUFLEN,"LINES=%d\n",lines) ;

	rs = u_write(TFD,buf,strlen(buf)) ;
#endif

	return 0 ;
}
/* end subroutine (main) */


/* local subroutines */



