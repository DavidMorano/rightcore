/* main (termtest) */


#define	CF_SINGLEOFF	1
#define	CF_EOL		1


/* revision history:

	= 2017-03-17, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#undef	BUFLEN
#define	BUFLEN		((2 * LINEBUFLEN) + 10)

#define	TO_READ		3


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	bfile		ifile, ofile ;
	int		rs ;
	int		bl ;
	int		llen ;
	int		f_off ;
	char		lbuf[LINEBUFLEN + 1] ;
	char		outbuf[BUFLEN + 1] ;


	f_off = (argc > 1) ;


	rs = bopen(&ofile,BFILE_STDOUT,"wct",0666) ;

	if (rs < 0)
		return EX_CANTCREAT ;

	rs = bopen(&ifile,BFILE_STDIN,"r",0666) ;

	if (rs < 0)
		return EX_NOINPUT ;
	
	while ((rs = breadline(&ifile,lbuf,LINEBUFLEN)) > 0) {
		llen = rs ;

		if (lbuf[llen - 1] == '\n') llen -= 1 ;

#if	(! CF_EOL)
		bwrite(&ofile,"\033#3",3) ;
#endif

		bwrite(&ofile,lbuf,llen) ;

#if	CF_EOL
		bwrite(&ofile,"\033#3",3) ;
#endif

#ifdef	COMMENT
		bwrite(&ofile,"\033#5",3) ;

		bprintf(&ofile," computer system\n") ;
#else
		bputc(&ofile,'\n') ;
#endif

#if	(! CF_EOL)
		bwrite(&ofile,"\033#4",3) ;
#endif

		bwrite(&ofile,lbuf,llen) ;

#if	CF_EOL
		bwrite(&ofile,"\033#4",3) ;
#endif

#ifdef	COMMENT
		bwrite(&ofile,"\033#5",3) ;

		bprintf(&ofile," ready\n") ;
#else
		bputc(&ofile,'\n') ;
#endif

#ifdef	COMMENT
		bwrite(&ofile,"\033[0m",4) ;
#endif

	} /* end while */

	bwrite(&ofile,"\033[4m",4) ;

	bprintf(&ofile,"computer system ready\n") ;

#if	CF_SINGLEOFF
	bwrite(&ofile,"\033[24m",5) ;
#else
	bwrite(&ofile,"\033[0m",4) ;
#endif

/* all attributes off */

	if (f_off)
	bwrite(&ofile,"\033[0m",4) ;

/* done */

	bclose(&ifile) ;

	bclose(&ofile) ;

	return 0 ;
}
/* end subroutine (main) */


