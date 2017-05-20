/* format a file into pages for a printer */

/*

	= 1998-09-01, David A.D. Morano

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will read the input file and copy it line by line to the
        output, except that it will prepend some leading spaces on each line.
        Also, only 60 lines will be printed on each page and a page header and
        trailer will also be written.


*******************************************************************************/


#include	<envstandards.h>

#include	<fcntl.h>

#include	<time.h>

#include	"bfile.h"

#include	<stdio.h>

#include	"rel.h"




	extern long	time() ;

	extern int	bopen(), bclose(), bflush() ;
	extern int	bprintf(), bgetc(), bputc() ;


	static char	tabs[] = "\t\t\t\t\t\t\t\t\t\t" ;


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;

	long		clock ;

	register int	c, i, n ;

	int		offset, page, line, rs ;

	char		*ts ;
	char		*name = "(standard input)" ;


	switch (argc) {

case 0:
	return BAD ;

case 1:
	bopen(ifp,BFILE_IN,"r",0666) ;

	bopen(ofp,BFILE_OUT,"w",0666) ;

	break ;

case 2:
	rs = bopen(ifp,argv[1],"r",0666) ;
	if (rs < 0) return (rs) ;

	rs = bopen(ofp,BFILE_OUT,"w",0666) ;
	if (rs < 0) return rs ;

	name = argv[1] ;
	break ;

case 3:
default:
	rs = bopen(ifp,argv[1],"r",0666) ;
	if (rs < 0) return (rs) ;

	rs = bopen(ofp,argv[2],"w",0666) ;
	if (rs < 0) return rs ;

	name = argv[1] ;
	break ;

	} ; /* end switch */


	clock = time(0L) ;

	ts = ctime(&clock) ;
	ts[24] = '\0' ;


	offset = 16 ;


	line = 0 ; 
	page = 1 ;

	while ((c = bgetc(ifp)) != BFILE_EOF) {

		if (c == '\014') {

			if (line != 0) {

			    bputc(ofp,c) ;

			    bprintf(ofp,
"\n\n\t%24s\t%16s\t\t\tpage %4d\n\n",ts,name,page) ;

			    page += 1 ;
			    line = 0 ;

			} ;

			continue ;
		} ;


		if (line == 0) {

			if (page != 1) bputc(ofp,'\014') ;

			bprintf(ofp,
"\n\n\t%24s\t%16s\t\t\tpage %4d\n\n",ts,name,page) ;

		} ;


		if (c != '\n') {

			for (i = 0 ; i < offset ; i += 1) {

				bputc(ofp,' ') ;

			} ;

			bputc(ofp,c) ;

			while ((c = bgetc(ifp)) != BFILE_EOF) {

				bputc(ofp,c) ;

				if (c == '\n') break ;

			} ;

		} else bputc(ofp,c) ;


		line += 1 ;

		if (line == 59) {

			bprintf(ofp,"\n%spage %4d\n",tabs,page) ;

			line = 0 ;
			page += 1 ;

		} ;

	} ;


	bflush(ofp) ;

	rs = bclose(ofp) ;

	return OK ;
}


