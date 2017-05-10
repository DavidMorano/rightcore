/* main */

/* test program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine (it's the whole program -- same as
	the FIFO test) was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	This program tests the XDR facility often used in RPC.


***************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<rpc/types.h>
#include	<rpc/xdr.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<termios.h>

#include	<vsystem.h>
#include	<field.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_SRVFLAGS	(O_RDONLY | O_CREAT | O_NONBLOCK)


/* external subroutines */

extern char	*strbasename(char *) ;


/* external variables */


/* forward references */

void		int_alarm() ;
void		int_signal() ;


/* gloabal variables */

int		f_alarm ;
int		f_signal ;





int main(int argc,cchar *argv[],cchar *envv[])
{
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;
	struct XDR	xb ;
	struct termios		ots, nts ;
	struct sigaction	sigs ;
	sigset_t		signalmask ;
	long		lw ;

	int		len, lenr, rs ;
	int		i, j ;
	int		iw ;
	int		err_fd ;

	char	*progname ;
	char	tmpbuf[100] ;
	char	xbuf[BUFLEN + 1] ;
	char	*bp ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"dwca",0664) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	f_signal = FALSE ;
	f_alarm = FALSE ;


/* do something */

	bopen(ofp,BFILE_STDOUT,"dwct",0666) ;


	xdrmem_create(&xb,xbuf,BUFLEN,XDR_ENCODE) ;

	lw = 0x01020304 ;
	xdr_long(&xb,&lw) ;

	iw = 0x0102 ;
	xdr_int(&xb,&iw) ;

#ifdef	COMMENT
	(void) xdr_control(&xb,XDR_GET_BYTES_AVAIL,&lenr) ;

	len = BUFLEN - lenr ;

#else
	len = xdr_getpos(&xb) ;
#endif

#if	CF_DEBUGS
	debugprintf("main: len=%d\n",len) ;
#endif

	bwrite(ofp,xbuf,len) ;

	xdr_destroy(&xb) ;


	bclose(ofp) ;


done:
	bclose(efp) ;

	return ES_OK ;

badret:
	bclose(efp) ;

	return ES_BADARG ;

badin:
	bprintf(efp,"%s: could not open input, rs=%d\n",
	    progname,rs) ;

	goto badret ;

badout:
	bprintf(efp,"%s: could not open output, rs=%d\n",
	    progname,rs) ;

	goto badret ;

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


