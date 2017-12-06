/* main */

/* program to read a shared memory mapped file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1


/* revision history:

	= 1988-01-10, David A­D­ Morano

	This subroutine was taken from the TESTMAP program.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This subroutine is the major part of a program that tests
	file mapping concepts.


***************************************************************************/


#include	<envstandards.h>


#include	<sys/types.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<termios.h>
#include	<time.h>
#include	<stdlib.h>

#include	<field.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>


/* local defines */

#define	LINELEN		100
#define	BUFLEN		1024
#define	RUNTIME		120		/* seconds */
#define	MAXLINES	10



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


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
	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	sigset_t		signalmask ;

	struct termios		ots, nts ;

	struct sigaction	sigs ;

	time_t	daytime, endtime ;

	int	len, rs ;
	int	ex = EX_USAGE ;
	int	i, j ;
	int	fd = -1 ;
	int	f_msync ;
	int	fd_debug ;

	char	*progname ;
	char	seqbuf[LINELEN + 1] ;
	char	tmpbuf[BUFLEN + 1] ;
	char	*buf, *bp ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_NOBUF,0) ;

	if (argc < 2)
	    goto earlyret ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0)
	    goto earlyret ;



#if	CF_DEBUGS
	debugprintf("main: 'open()' file=%s\n",argv[1]) ;
#endif

	if ((fd = u_open(argv[1],O_RDWR,0666)) < 0) {

	    bprintf(efp,"%s: could not open mapfile rs=%d\n",
	        progname,fd) ;

		ex = EX_NOINPUT ;
	    goto done ;
	}

#if	CF_DEBUGS
	debugprintf("main: 'mapfile()'\n") ;
#endif

	rs = u_mmap(NULL,BUFLEN, (PROT_WRITE | PROT_READ) ,MAP_SHARED,
	    fd,0L,&buf) ;

	if ((rs < 0) || (buf == NULL)) {

	    bprintf(efp,"%s: could not map the file rs=%d\n",
	        progname,rs) ;

		ex = EX_DATAERR ;
		goto done ;

	} else
	    bprintf(efp,"%s: buf=%08X\n",
	        progname,buf) ;


	u_time(&daytime) ;

	endtime = daytime + RUNTIME ;

	if (argc > 2) {

/* we are the writer */

	    for (i = 0 ; daytime < endtime ; i += 1) {

	        int	lines ;


	        bp = buf ;
	        bp += ctdeci(buf,BUFLEN,i) ;

	        *bp++ = '\n' ;
	        *bp = '\0' ;

#if	CF_DEBUGS
	debugprintf("main: wrote %d bytes into buffer\n",
		(bp - buf)) ;
#endif

	        sleep(2) ;

	        u_time(&daytime) ;

	    } /* end for */

	} else {

	    int	sequence ;


/* we are the reader */

#if	CF_DEBUGS
	    debugprintf("main: test read of buffer\n") ;
#endif

	    buf[0] = '\0' ;
	    tmpbuf[0] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("main: about to 'for 1'\n") ;
#endif

	    sequence = 0 ;
	    for (i = 0 ; daytime < endtime ; i += 1) {

	        int	j, v, lines ;


	        f_msync = FALSE ;
	        if ((i % 5) == 0) {

	            rs = uc_msync(buf,BUFLEN,MS_INVALIDATE) ;

	            if (rs < 0) {

	                bprintf(efp,"%s: bad 'msync()' rs=%d\n",
	                    progname,rs) ;

	                goto badret ;
	            }

	            f_msync = TRUE ;

	        } /* end if (synchronization) */


	        if (strncmp(buf,tmpbuf,BUFLEN) != 0) {

/* read off the first coded line in the buffer */

	            bp = buf ;
	            j = 0 ;
	            while (*bp && (bp < (buf + LINELEN)) && (*bp != '\n'))
	                seqbuf[j++] = *bp++ ;

#if	CF_DEBUGS
		debugprintf("main: reader line=>%W<\n",
			buf,(bp - buf)) ;
#endif

			if (*bp == '\n')
				bp += 1 ;

	            rs = cfdeci(seqbuf,j,&v) ;

	            bprintf(ofp,"%H") ;

	            if ((rs >=0) && (v <= (sequence + 1)))
	                bprintf(ofp,"%s %K\n",
	                    ((f_msync) ? "SYNC" : "    ")) ;

			else
	                bprintf(ofp,"%s bad sequence number detected%K\n",
	                    ((f_msync) ? "SYNC" : "    ")) ;

			sequence = v ;

			bp = buf ;
	            lines = 1 ;
	            for (j = 0 ; (lines < MAXLINES) && (j < BUFLEN) && 
	                (*bp != '\0') ; j += 1) {

#if	CF_DEBUGS
	                debugprintf("main: 'bputc()' \n") ;
	                debugprintf("main: 'bputc()' c=%c\n",*bp) ;
#endif

	                if (*bp == '\n') {

	                    lines += 1 ;
	                    bprintf(ofp,"%K\n") ;

	                } else
	                    bputc(ofp,*bp) ;

	                bp += 1 ;

	            } /* end for */

#if	CF_DEBUGS
	            debugprintf("main: 'bflush()'\n") ;
#endif

	            bflush(ofp) ;

	            strwcpy(tmpbuf,buf,BUFLEN) ;

	        } /* end if (buffer changed) */

#if	CF_DEBUGS
	        debugprintf("main: 'sleep()'\n") ;
#endif

	        sleep(1) ;

	        u_time(&daytime) ;

	    } /* end for */

	} /* end if (reader or writer) */

	u_close(fd) ;


	ex = EX_OK ;


done:
	bclose(ofp) ;

earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

badret:
	ex = EX_DATAERR ;
	goto earlyret ;
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



