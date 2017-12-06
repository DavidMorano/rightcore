/* main */

/* test the 'poll(2)' system call */
/* last modified %G% version %I% */


#define	CF_DEBUG	0
#define	F_WHO		0
#define	F_SIGACTION	1
#define	F_ALARM		1
#define	F_TESTRAW	0
#define	F_SLEEP		0


/* revision history:

	= David A.D. Morano, January 88

	This is a hack to test the SYSV 'poll(2)' system call.


*/


/************************************************************************

	This program displays the characters typed on the
	terminal in HEX to the screen.  A carriage return will
	exit the program.


***************************************************************************/


#include	<sys/types.h>
#include	<sys/types.h>
#include	<sys/unistd.h>
#include	<termios.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<errno.h>

#include	<field.h>
#include	<bfile.h>

#include	"localmisc.h"



/* local defines */

#define	LINELEN		100
#define	TTY_READCHARS	10

#define	TTY_MINCHARS	0		/* minimum characters */
#define	TTY_MINTIME	10		/* x100 milliseconds */



/* external subroutines */


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;




/* start of program */
int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct termios		ots, nts ;
	struct sigaction	sigs ;
	sigset_t	signalmask ;
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;
	int		len ;
	int		i, j ;
	int		tfd = -1 ;
	int		f_reveal = FALSE ;
	uchar		c ;
	uchar		*bp, buf[LINELEN] ;


	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_NOBUF,0) ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0)
	    return BAD ;

#ifdef	COMMENT
	bcontrol(ofp,BC_NOBUF) ;
#endif

	if (argc > 1)
	    f_reveal = TRUE ;


/* set up terminal for raw access */

	ioctl(tfd,TCGETS,&ots) ;

	nts = ots ;

	nts.c_iflag &= 
	    (~ (INLCR | ICRNL | IUCLC | IXANY | ISTRIP | INPCK | PARMRK)) ;
	nts.c_iflag |= IXON ;

	nts.c_cflag &= (~ (CSIZE)) ;
	nts.c_cflag |= CS8 ;

#if	F_TESTRAW
	nts.c_lflag = 0 ;
#else
	nts.c_lflag &= (~ (ICANON | ECHO | ECHOE | ECHOK | ECHONL)) ;
#endif

#if	F_WHO
	nts.c_lflag &= (~ (ISIG)) ;
#endif

	nts.c_oflag &= (~ (OCRNL | ONOCR | ONLRET)) ;

	nts.c_cc[VMIN] = TTY_MINCHARS ;
	nts.c_cc[VTIME] = TTY_MINTIME ;

/* set it ! */

	ioctl(tfd,TCSETSW,&nts) ;


/* set up the handler for alarms */

#if	F_ALARM
#if	F_SIGACTION
	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_alarm ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	sigaction(SIGALRM,&sigs,NULL) ;
#else
	signal(SIGALRM,int_alarm) ;
#endif

	f_alarm = FALSE ;
	alarm(4) ;
#endif /* F_ALARM */

/* set up the handler for interrupts */

	f_signal = FALSE ;
	(void) sigemptyset(&signalmask) ;

	sigs.sa_handler = int_signal ;
	sigs.sa_sigaction = int_signal ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = SA_RESTART ;

	sigaction(SIGINT,&sigs,NULL) ;


/* go through the loops */

	j = 0 ;
	while (TRUE) {

	    bflush(ofp) ;

#if	F_SLEEP
#ifdef	COMMENT
	    system("sleep 3") ;
#else
	    sleep(3) ;
#endif
#endif /* F_SLEEP */

	    for (i = 0 ; i < TTY_READCHARS ; i += 1) 
		buf[i] = '\0' ;

	    if ((len = read(tfd,buf,TTY_READCHARS)) < 0)
		len = (- errno) ;

#if	F_ALARM
	    if (f_alarm) {

#if	(! F_SIGACTION)
	        signal(SIGALRM,int_alarm) ;
#endif

	        f_alarm = FALSE ;
	        if (f_reveal) 
			bprintf(ofp," Alarm len=(%d)\n",len) ;

	        alarm(4) ;

	    } /* end if (alarm) */
#endif /* F_ALARM */

	    if (f_signal) {

	        f_signal = FALSE ;
	        bprintf(ofp," Signal len=(%d)\n",len) ;

	    }

	    if (len < 0) {

	        if (f_reveal) {

	            j = 0 ;
	            bprintf(ofp," len=(%d)\n",len) ;

	        }

	        continue ;

	    } else if (len == 0) {

	        if (f_reveal)
			bprintf(ofp," TO") ;

	        if (j > 0) {

	            j = 0 ;
	            bprintf(ofp,"\n") ;

	        }

	        continue ;

	    } /* end if */

	    for (i = 0 ; i < len ; i += 1) {

	        bprintf(ofp," %02X",buf[i]) ;

	        if ((buf[i] & 0x7F) == '\r') goto done ;

	        if ((buf[i] & 0x7F) == '\n') goto done ;

	        j += 1 ;
	        if (j > 20) {

	            j = 0 ;
	            bprintf(ofp,"\n") ;

	        }

	    } /* end for */

	} /* end while */


done:
	bprintf(ofp,"\n") ;

	ioctl(tfd,TCSETSW,&ots) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */


void int_alarm(sig)
int	sig ;
{

	f_alarm = TRUE ;
}


void int_signal(sig)
int	sig ;
{

	f_signal = TRUE ;
}




