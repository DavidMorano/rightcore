/* find form feed */

/*
	David A.D. Morano
	September 1987
*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Search for a line feed and report when we have found at least one.


*******************************************************************************/



#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>

#include	<bfile.h>

#include	"localmisc.h"



/* defines */

#define		DPRINT		0

#define		REFPAGE		1

#define		NTOPLINES	3
#define		MAXLINES	130
#define		LINELEN		100
#define		MAXBS		20

#define		NAME		"ff"
#define		VERSION		"0"

#define		NPARG		2	/* number of positional arguments */


/* externals */




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;

	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	long		argl ;

	int		pan, i, j, si ;
	int		page, len, l, lines, rs ;
	int		ic ;

	char		tbuf[LINELEN + 1] ;
	char		*tbp ;

	char	c, *argp ;
	char	f_refpage = FALSE ;
	char	f_concatenate = FALSE ;
	char	*fontname = "CB" ;

	char	*infname = (char *) 0 ;
	char	*outfname = (char *) 0 ;


	if (bopen(efp,BERR,"wc",0666) < 0) return BAD ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            argp += 1 ;
	            c = tolower(*argp) ;

	            switch (c) {

	            case 'c':
	                f_concatenate = TRUE ;
	                break ;

	            case 'r':
	                f_refpage = TRUE ;
	                break ;

	            case 'v':
	                bprintf(efp,"%s version : %s\n",NAME,VERSION) ;

	                break ;

	            case 'i':
	                if (argc <= 0) goto not_enough ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                if (argl) infname = argp ;

	                break ;

	            case 'o':
	                if (argc <= 0) goto not_enough ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                if (argl) outfname = argp ;

	                break ;

	            case 'f':
	                if (argc <= 0) goto not_enough ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                fontname = argp ;
	                break ;

	            default:
	                bprintf(efp,"%s: unknown option - %c\n",NAME,*argp) ;

	                goto usage ;

	            } /* end switch */

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) infname = argp ;

	                break ;

	            case 1:
	                if (argl > 0) outfname = argp ;

	                break ;

	            default:
	                break ;

	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",NAME) ;

	        }

	    } /* end if */

	} /* end while */


/* check arguments */

	if (infname == (char *) 0) infname = (char *) BIN ;

	if (outfname == (char *) 0) outfname = (char *) BOUT ;


/* open files */

	rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0) return rs ;

	rs = bopen(ofp,outfname,"wc",0666) ;

	if (rs < 0) return rs ;


	i = 0 ;
	while ((ic = bgetc(ifp)) >= 0) {

	    if (ic == 0x0C) bprintf(ofp,"found one\n") ;

		i += 1 ;
	}

	bprintf(ofp," %d bytes\n",i) ;

	bclose(ifp) ;

	bclose(ofp) ;

bad_ret:
	bclose(efp) ;

	return OK ;

usage:
	bprintf(efp,
"usage: mroff [-c] [-r] [infile] [outfile] [-f font] [-o outfile]\n") ;

	goto bad_ret ;

not_enough:
	bprintf(efp,"not enough arguments specified\n") ;

	goto bad_ret ;
}


