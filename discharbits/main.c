/* main */

/* program to display characters */
/* last modified %G% version %I% */


#define	CF_DEBUG	0


/* revision history:

	= 1988-01-10, David A­D­ Morano


*/


/************************************************************************

	This program displays the characters typed on the
	terminal in HEX output in a form suitable for inclusion
	into a C language program.


***************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<localmisc.h>


/* program defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;
	long		lw ;
	int		len, llen ;
	int		i, j ;
	int		tfd = -1 ;
	int		f_exit, f_timeout ;
	uchar		c ;
	uchar		*bp, buf[LINEBUFLEN + 1] ;
	uchar		bits[32], buf2[32] ;


	if (bopen(efp,BFILE_STDERR,"dwca",0664) < 0) return BAD ;

	bcontrol(efp,BC_NOBUF) ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) < 0) return BAD ;


#if	CF_DEBUG
	debugprintf("main: entered\n") ;
#endif

/* set up terminal for raw access */

	if (uterm_start(tfd) < 0) {

	    bprintf(efp,"bad terminal initialize \n") ;

	    goto badret ;
	}

#if	CF_DEBUG
	debugprintf("main: initialized the terminal\n") ;
#endif


/* set initial terminal mode to default */

	if (uterm_control(tfd,fm_setmode,0) < 0)
	    bprintf(efp,"bad return from 'control'\n") ;


/* initialize the bit array */

#if	CF_DEBUG
	debugprintf("main: about to call 'fieldterms'\n") ;
#endif

	fieldterms(bits,0,"") ;

#if	CF_DEBUG
	debugprintf("main: called 'fieldterms'\n") ;
#endif


	f_exit = FALSE ;
	while (! f_exit) {

	    f_timeout = FALSE ;
	    for (i = 0 ; (i < 20) && (! ((i > 0) && f_timeout)) ; i += 1) {

	        bflush(ofp) ;

	        len = uterm_reade(tfd,fm_rawin | fm_noecho,
	            buf,1, 2,0,0,0) ;

#if	CF_DEBUG
	        bprintf(efp,"len read (%d)\n",len) ; 
	        ; 
	        bflush(efp) ;
#endif

	        if (len < 0) {

	            bprintf(efp,"problems w/ input\n") ;

	            goto exit ;

	        } else if (len == 0) {

	            f_timeout = TRUE ;
	            continue ;
	        }

	        bprintf(ofp," %02X",buf[0]) ;

	        buf2[0] = buf[0] ;
	        buf2[1] = '\0' ;

	        if ((buf[0] & 0x7F) == '\r') {

	            f_exit = TRUE ;
	            break ;

	        } else if ((buf[0] & 0x7F) == '\n')
	            bprintf(ofp,"\r\n") ;

	        else {

#if	CF_DEBUG
	            debugprintf("main: about to call 'fieldterms' for a character\n") ;
#endif

	            fieldterms(bits,1,buf2) ;

#if	CF_DEBUG
	            debugprintf("main: called 'fieldterms' for a character\n") ;
#endif

	        }


	    } /* end for */

	    bprintf(ofp,"\n") ;

	} /* end while */

	for (i = 0 ; i < 16 ; i += 1) {

	    bprintf(ofp,"	0x%02X, 0x%02X,\n",
	        bits[(i * 2)], bits[(i * 2) + 1]) ;

	} /* end for */

/* we are out of here ! */
exit:
	uterm_restore(tfd) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



