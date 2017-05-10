/* main */


#define	F_SFBASENAME	1		/* use 'sfdbasename()' ? */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>
#include	<stdio.h>

#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	fgetline(FILE *,char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdirname(char *) ;
extern char	*strbasename(char *) ;


/* exported subroutines */


int main()
{
	int	rs ;
	int	sl, cl, bl, dl ;
	int	len ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	dirbuf[LINEBUFLEN + 1] ;
	char	basebuf[LINEBUFLEN + 1] ;
	char	*sp ;
	char	*cp ;
	char	*bp, *dp ;


	while ((len = fgetline(stdin,linebuf,LINEBUFLEN)) > 0) {

	    sl = sfshrink(linebuf,len,&sp) ;

	    if (sl > 0) {

	        sp[sl] = '\0' ;

#if	F_SFBASENAME
	    cl = sfbasename(sp,sl,&cp) ;

		bp = basebuf ;
		bl = cl ;
	        strwcpy(basebuf,cp,cl) ;
#else
		strwcpy(basebuf,sp,sl) ;

		bp = strbasename(basebuf) ;

		bl = strlen(bp) ;
#endif


	        fprintf(stdout,"%-40s => %s (%d)\n",sp,bp,bl) ;

	    }

	} /* end while */


	fclose(stdin) ;

	fclose(stdout) ;

	return 0 ;
}
/* end subroutine (main) */



